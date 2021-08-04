#!/usr/bin/env python

from __future__ import print_function
from __future__ import absolute_import
import os, sys, numpy, pprint
from optparse import OptionParser
import sigraph
from soma import aims
import datamind.io.old_csvIO as datamind_io
from sulci.common import io, add_translation_option_to_parser
from sulci.models import distribution
from six.moves import range


################################################################################
def parseOpts(argv):
	description = 'Compute priors'
	parser = OptionParser(description)
	add_translation_option_to_parser(parser)
	parser.add_option('-b', '--database', dest='database',
		metavar = 'FILE', action='store',
		default = 'bayesian_gaussian_databases.dat',
		help='databases summary file (default : %default).')
	parser.add_option('-m', '--graphmodel', dest='graphmodelname',
		metavar = 'FILE', action='store',
		default = 'bayesian_graphmodel.dat', help='bayesian model : '\
			'graphical model structure (default : %default)')
	parser.add_option('-i', '--idistrib', dest='input_distribname',
		metavar = 'FILE', action='store', default = None,
		help='input distribution models')
	parser.add_option('-o', '--odistrib', dest='output_distribname',
		metavar = 'FILE', action='store', default = None,
		help='output distribution models')
	parser.add_option('-c', '--csv', dest='csvname',
		metavar = 'FILE', action='store',
		default = 'EM_priors_evolution.csv',
		help='csv storing priors while EM is running ' \
					'(default : %default)')
	return parser, parser.parse_args(argv)

def main():
	parser, (options, args) = parseOpts(sys.argv)
	if None in [options.input_distribname, options.output_distribname]:
		parser.print_help()
		sys.exit(1)

	print("read...")
	bayesian_model = io.read_bayesian_model(
			options.graphmodelname, options.input_distribname)
	distribs = bayesian_model['node_distrib']
	databases = io.read_databaselist(options.database)

	print("compute likelihoods...")
	gv = bayesian_model['vertices']
	states = list(gv.keys())
	pn = bayesian_model['priors_nodes_hash']
	prior_ind = pn['nodes_number']
	data_n = distribs['priors_nodes_total'][0]
	labels_n = len(gv)
	labels_id = {}
	for i, label in enumerate(gv.keys()): labels_id[label] = i

	# fill 
	P = numpy.asmatrix(numpy.zeros((labels_n, 1)))
	B = numpy.asmatrix(numpy.zeros((labels_n, data_n)))
	Bl = numpy.asmatrix(numpy.zeros((labels_n, data_n)))
	offset = 0
	for labels, minfname in databases['files'].items():
		if isinstance(labels, tuple) or isinstance(labels, list):
			continue
		db, header = datamind_io.ReaderMinfCsv().read(minfname)
		data = gv[labels]
		distrib = data['density']
		prior = data['priors'][prior_ind]
		P[labels_id[labels], 0] = prior
		X = db.getX()
		for i, (label, data) in enumerate(gv.items()):
			distrib = data['density']
			for j, x in enumerate(X):
				logli, li = distrib.likelihood(x)
				B[i, offset + j] = li
				Bl[i, offset + j] = logli
		offset += len(X)


	# EM : learn priors
	print("estimate priors...")
	mse = numpy.inf
	n = 0
	Ps = [P]
	while mse > 10e-10:
		A = numpy.multiply(P, B)
		D = 1./numpy.dot(B.T, P)
		P2 = numpy.dot(A, D) / B.shape[1]
		mse = (numpy.asarray(P2 - P) ** 2).mean()
		P = P2
		Ps.append(P)
		n += 1

		Q = numpy.multiply(B, numpy.log(P) + Bl).sum()
		print("Q = ", Q)

	Ps = numpy.hstack(Ps)
	print("priors estimated in %d steps" % n)

	# write priors
	print("write priors in distrib file...")
	distribs['priors_nodes_names'].append('EM')
	distribs['priors_nodes_total'].append(1.0)
	for labels, local_model_infos in distribs['files'].items():
		if isinstance(labels, tuple) or isinstance(labels, list):
			continue
		model_type, distribfile, weights = local_model_infos
		prior = P[labels_id[label], 0]
		weights.append(prior)

	summary_file = options.output_distribname
	fd = open(summary_file, 'w')
	fd.write('distributions = \\\n')
	p = pprint.PrettyPrinter(indent=4, width=1000, stream=fd)
	p.pprint(distribs)
	fd.close()

	# write csv of priors
	print("write priors evolution (while EM running) in csv...")
	fd = open(options.csvname, 'w')
	s = "sulci\t"
	s += '\t'.join([("prior_%d" % i) for i in range(n + 1)]) + '\n'
	fd.write(s)
	for label in gv.keys():
		p = Ps[labels_id[label]]
		s = str(label)
		for prior in (numpy.asarray(p)).ravel(): s += "\t%f" % prior
		fd.write(s + '\n')
	fd.close()


if __name__ == '__main__' : main()
