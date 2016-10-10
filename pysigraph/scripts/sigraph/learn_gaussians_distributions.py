#!/usr/bin/env python2

from __future__ import print_function
import os, sys, pprint
from optparse import OptionParser
import datamind.io.old_csvIO as datamind_io
from sulci.common import io
from sulci.models import distribution
		

################################################################################
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

	return parser, parser.parse_args(argv)

def main():
	parser, (options, args) = parseOpts(sys.argv)
	databases = io.read_databaselist(options.database)
	prefix = options.distribdir

	datatypes = databases['data']
	if not (isinstance(datatypes, list) or isinstance(datatypes, tuple)):
		datatypes = [datatypes]
	authorized_datatypes = ['refgravity_center', 'delta_gravity_centers',
		'min_distance']
	datatype_error = False
	for dt in datatypes:
		if not (dt in authorized_datatypes):
			datatype_error = True
	if datatype_error:
		print("at least one datatype is incorrect among : " + \
			str(datatypes))
		sys.exit(1)

	# create output directory
	try:	os.mkdir(prefix)
	except OSError, e:
		print("warning: directory '%s' allready exists" % prefix)

	# db => mean/cov
	# bayesian_*.minf => bayesian_density_*.minf
	h = {'data_type' : databases['data'], 'files' : {},
		'level' : 'segments'}
	for labels, minfname in databases['files'].items():
		print(minfname)
		db, header = datamind_io.ReaderMinfCsv().read(minfname)
		size = len(db)
		if db.getX().std(axis=0).sum() == 0: # all values equal
			g = distribution.Dirac()
		elif size < 4:	g = distribution.FakeGaussian(10e-100)
		elif options.robust:
			g = distribution.RobustGaussian()
		else:
			if size < 8: g = distribution.SphericalGaussianFewDots()
			elif size < 12: g = distribution.SphericalGaussian()
			elif size < 18:	g = distribution.DiagonalGaussian()
			else:	g = distribution.Gaussian()
		if isinstance(labels, list) or isinstance(labels, tuple):
			l1, l2 = labels
			# handle relation intra-label (ex: SC-SC)
			if l1 == l2 and 'delta_gravity_centers' in datatypes:
				g = distribution.OrientedGaussian(g)
		g.fit(db)
		filename = io.node2densityname(prefix, 'gaussian', labels)
		g.write(filename)
		h['files'][labels] = (g.name(), filename)

	# write distribution summary file
	summary_file = options.distribdir + '.dat'
	fd = open(summary_file, 'w')
	fd.write('distributions = \\\n')
	p = pprint.PrettyPrinter(indent=4, width=1000, stream=fd)
	p.pprint(h)
	fd.close()

	

if __name__ == '__main__' : main()
