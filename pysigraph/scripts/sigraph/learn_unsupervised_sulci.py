#!/usr/bin/env python2

import os, sys, pprint, numpy
from optparse import OptionParser
import datamind.io.old_csvIO as datamind_io
from datamind.tools import msg
from sulci.models import distribution
from sulci.common import io
from sulci.features import fd3
		

def parseOpts(argv):
	description = 'Learn gaussian models.'
	parser = OptionParser(description)
	parser.add_option('-b', '--database', dest='database',
		metavar = 'FILE', action='store',
		default = 'bayesian_sulci_databases.dat',
		help='databases summary file (default : %default).')
	parser.add_option('-d', '--distribdir', dest='distribdir',
		metavar = 'FILE', action='store',
		default = 'bayesian_gaussianblocs_distribs',
		help='output distribution directory (default : %default).' \
			'A file named FILE.dat is created to store ' \
			'labels/databases links.')
	parser.add_option('-s', '--sulcus', dest='sulcus',
		metavar = 'NAME', action='store', default = None,
		help='learn only specified sulcus.')
	return parser, parser.parse_args(argv)

def group_dimensions_sulci_fd3(sulci, db):
	X = db.getX()

	# filter unvalid vectors
	X = X[X[:, 0] == 1]

	h = fd3.sulci_fd3()
	groups = h['groups']

	# blocs definition
	valid, extremitiesValid, normalValid = 0, 1, 11
	others = groups['others']
	null = X.std(axis=0) == 0
	n1 = numpy.argwhere(null == 0).ravel()
	n2 = numpy.argwhere(null).ravel()
	diagonal_bloc = [d for d in n1 if d in others]
	fixed_spherical_bloc = [d for d in n2 if d in others]

	# unvalid blocs ?
	ev, nv = X[:, extremitiesValid].sum(), X[:, normalValid].sum()
	rev, rnv = ev / X.shape[0], nv / X.shape[0]
	valid_extremities = (ev > 3)
	valid_normal = (nv > 3)
	
	# retained blocs
	blocs = {}
	types = ['full_gaussian', 'diagonal_gaussian', 'spherical_gaussian',
		'spherical_gaussian_fixed', 'spherical_gaussian_few_dots',
		'fake_gaussian']
	if valid_extremities:
		if ev < 5: te = types[4]
		elif ev < 7: te = types[2]
		elif ev < 10: te = types[1]
		else:	te = types[0]
		blocs['extremity1'] = te, groups['extremity1']
		blocs['extremity2'] = te, groups['extremity2']
		blocs['direction'] = te, groups['direction']
	else:	te = 'none'
	blocs['gravity'] = types[0], groups['gravity']
	if valid_normal:
		if nv < 5: tn = types[4]
		elif nv < 7: tn = types[2]
		elif nv < 10: tn = types[1]
		else:	tn = types[0]
		blocs['normal'] = tn, groups['normal']
	else:	tn = 'none'
	blocs['others_diagonal']= types[1], diagonal_bloc
	blocs['others_fixed_spherical'] = types[3], fixed_spherical_bloc, 0.5
	print '* %s : ' % sulci
	if not valid_extremities or not valid_normal:
		s = ' - invalid : '
		if not valid_extremities: s += 'extremities, '
		if not valid_normal: s += 'normal'
		print s
	s = ' - extremities : %s (%d : %d%%)\n' % (te, ev, rev * 100)
	s += ' - normal : %s (%d : %d%%)' % (tn, nv, rnv * 100)
	print s
	
	# nan extremities when unvalid
	d = X.shape[1]
	col = numpy.array([x in (groups['extremities'] + groups['direction']) \
							for x in range(d)])
	unvalid = numpy.repeat(X[:, extremitiesValid] == 0, d).reshape(-1, d)
	X[unvalid * col] = numpy.nan

	# nan normal when unvalid
	d = X.shape[1]
	col = numpy.array([x in groups['normal'] for x in range(d)])
	unvalid = numpy.repeat(X[:, normalValid] == 0, d).reshape(-1, d)
	X[unvalid * col] = numpy.nan

	db.setX(X)
	return blocs


def main():
	parser, (options, args) = parseOpts(sys.argv)
	database = io.read_databaselist(options.database)
	dbdir = options.distribdir
	prefix = os.path.dirname(options.database)

	if database['data'] != 'sulci_features':
		print 'database data type must be : voxels_coordinates'
		sys.exit(1)

	# create output directory
	try:	os.mkdir(dbdir)
	except OSError, e:
		print "warning: directory '%s' allready exists" % dbdir

	nodes_total = 0

	# learn databases
	h = {'model' : 'bloc_gaussian', 'files' : {}}
	for labels, minfname in database['files'].items():
		minfname = os.path.join(prefix, minfname)
		db, header = datamind_io.ReaderMinfCsv().read(minfname)
		size = len(db)
		if isinstance(labels, list) or isinstance(labels, tuple):
			continue
		else:
			if options.sulcus is not None and \
				options.sulcus != labels: continue
			nodes_total += size
		blocs = group_dimensions_sulci_fd3(labels, db)
		g = distribution.BlocGaussian(blocs)
		g.fit(db)
		filename = io.node2densityname(dbdir, 'bloc_gaussian', labels)
		g.write(filename)
		h['files'][labels] = ('bloc_gaussian', filename, [size])
	h['priors_relations_names'] = []
	h['priors_relations_total'] = []
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
