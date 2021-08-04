#!/usr/bin/env python

from __future__ import print_function
from __future__ import absolute_import
import os, sys, pprint, re
from optparse import OptionParser
import numpy
import datamind.io.old_csvIO as datamind_io
import datamind.io.csvIO as datamind_newio
import datamind.ml.database
from sulci.common import io
from sulci.models import distribution
from sulci.registration import ProcrustMetricField
from sulci.registration.common import save_transformation
from six.moves import range

def parseOpts(argv):
	description = 'Learn gaussian models.'
	parser = OptionParser(description)
	parser.add_option('-b', '--database', dest='database',
		metavar = 'FILE', action='store',
		default = 'bayesian_gaussian_databases.dat',
		help='databases summary file (default : %default).')
	parser.add_option('-d', '--distribdir', dest='distribdir',
		metavar = 'FILE', action='store',
		default = 'bayesian_gaussian_distribs',
		help='output distribution directory (default : %default).' \
			'A file named FILE.dat is created to store ' \
			'labels/databases links.')
	parser.add_option('-r', '--robust', dest='robust',
		action='store_true', default = False,
		help='use riemanian robust mean of bootstrap covariance')
	parser.add_option('-v', '--vtk', dest='vtk',
		action='store_true', default = False,
		help='vtk display (default: no display)')
	return parser, parser.parse_args(argv)

def main():
	parser, (options, args) = parseOpts(sys.argv)
	databases = io.read_databaselist(options.database)
	db_prefix = os.path.dirname(options.database)
	prefix = options.distribdir

	datatypes = databases['data']
	if datatypes != 'gravity_centers':
		print('error : wrong datatype')
		sys.exit(1)

	# create output directory
	try:	os.mkdir(prefix)
	except OSError as e:
		print("warning: directory '%s' allready exists" % prefix)

	nodes_total = 0

	data_0 = {}
	data_n = {}
	subjects_map = {}
	priors = []
	labels = []
	for label, minfname in databases['files'].items():
		minfname = os.path.join(db_prefix, minfname)
		db, header = datamind_io.ReaderMinfCsv().read(minfname)
		size = len(db)
		if size < 5: continue
		dataname = os.path.join(db_prefix, os.path.dirname(minfname),
							header['data'])
		X = datamind_newio.ReaderCsv().read(dataname)
		subjects = X[:, 'subject'].tolist()
		data_0[label] = db.getX()
		data_n[label] = data_0[label].copy()
		subjects_map[label] = subjects
		nodes_total += size
		priors.append(size)
		labels.append(label)
	priors = [(p / float(nodes_total)) for p in priors]
	subjects = set()
	for label, label_subjects in subjects_map.items():
		subjects = subjects.union(label_subjects)
	subjects = list(subjects)
	subject_to_id = {}
	for id, s in enumerate(subjects): subject_to_id[s] = id
	for label, label_subjects in subjects_map.items():
		ids = [subject_to_id[s] for s in label_subjects]
		subjects_map[label] = numpy.array(ids)

	old_params = {}
	old_energies = {}
	for i in range(len(subjects)):
		old_params[i] = numpy.asmatrix(numpy.identity(3)), \
				numpy.asmatrix(numpy.zeros((3, 1)))
		old_energies[i] = numpy.inf
	eps = 10e-1
	energy_eps = 10.

	# calcul des associations
	weights = {}
	Xsubjects = {}
	labels_n = numpy.zeros(len(labels))
	for i in range(len(subjects)):
		X_list = []
		subweights = [[] for k in range(len(labels))]
		for j, label in enumerate(labels):
			X = data_0[label]
			indices = (subjects_map[label] == i)
			Xs = X[indices]
			size = len(Xs)
			if size == 0: continue
			labels_n[j] += size
			ones = [1.] * size
			zeros = [0.] * size
			for k, w in enumerate(subweights):
				if k == j:
					w += ones
				else:	w += zeros
			X_list.append(Xs)
		Xsubjects[i] = numpy.asmatrix(numpy.vstack(X_list)).T
		weights[i] = [numpy.asmatrix(w).T for w in subweights]


	if options.vtk:
		import sulci_registration.vtk_helpers as V
		pcXs = []
		for label in labels:
			X = data_0[label]
			pcX = V.PointsCloud(X.T)
			pcX.set_color([1, 0, 0])
			pcX.set_size(3.)
			pcXs.append(pcX)
		plotter = V.VtkPlot(600, 600)
		plotter.set_bgcolor([0.7, 0.8, 0.9])
		plotter.plot(pcXs)
		plotter.render()

	# boucle
	n = 0
	old_energy = total_energy = numpy.inf
	while 1:
		print("*********")
		print("**  %d   " % n)
		print("*********")
		n += 1
		means, metrics = [], []
		total_det, total_lognormalization = 0., 0.
		models = {}
		for j, label in enumerate(labels):
			X = data_n[label]
			if options.robust:
				g = distribution.RobustGaussian()
			else:	g = distribution.Gaussian()
			models[label] = g
			db = datamind.ml.database.DbNumpy(X)
			g.fit(db)
			g._compute_norm()
			means.append(numpy.asmatrix(g.mean()).T)
			metrics.append(g.metric())
			total_det += g.det()
			total_lognormalization += labels_n[j] * \
						g.lognormalization()
		print("total determinant : ", total_det)

		total_energy = 0.
		for i in range(len(subjects)):
			X = Xsubjects[i]
			subweights = weights[i]
			pmf = ProcrustMetricField(X, subweights, means, metrics)
			old_R, old_t = old_params[i]
			pmf.set_initialization(old_R, old_t)
			R, t = pmf.optimize_riemannian(eps)
			energy = pmf.getCurrentEnergy()
			total_energy += energy
			old_params[i] = R, t
			old_energies[i] = energy
			print("%d) en = %f" % (i, energy))
		total_lognormalization *= len(subjects)
		total_energy += total_lognormalization
		print("Total en = ", total_energy)

		for j, label in enumerate(labels):
			X = data_0[label]
			ids = subjects_map[label]
			X2 = []
			for i in range(len(subjects)):
				R, t = old_params[i]
				Xs = X[ids == i]
				if len(Xs) == 0: continue
				XRt = (R * numpy.asmatrix(Xs).T + t).T
				X2.append(numpy.asarray(XRt))
			X2 = numpy.vstack(X2)
			data_n[label] = X2
			if options.vtk: pcXs[j].set_X(X2.T)
		if options.vtk: plotter.render()
		if old_energy - total_energy < energy_eps: break
		else:	old_energy = total_energy
	
		
	h = {'model' : databases['data'], 'files' : {}}
	for j, (label, g) in enumerate(models.items()):
		filename = io.node2densityname(prefix, 'gaussian', label)
		g.write(filename)
		size = labels_n[j]
		h['files'][label] = (g.name(), filename, [size])
	for i, subject in enumerate(subjects):
		R, t = old_params[i]
		filename = os.path.join(prefix, subject + '_motion.trm')
		save_transformation(filename, R, t)

	h['priors_nodes_names'] = ['nodes_number']
	h['priors_nodes_total'] = [nodes_total]

	# write distribution summary file
	summary_file = options.distribdir + '.dat'
	fd = open(summary_file, 'w')
	fd.write('distributions = \\\n')
	p = pprint.PrettyPrinter(indent=4, width=1000, stream=fd)
	p.pprint(h)
	fd.close()

	

if __name__ == '__main__' : main()
