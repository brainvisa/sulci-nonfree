#!/usr/bin/env python

import os, sys, pprint, numpy
from optparse import OptionParser
import datamind.io.old_csvIO as datamind_io
from datamind.tools import *
from sulci.models import distribution
from sulci.common import io
		

def parseOpts(argv):
	description = 'Learn supervised models for sulci'
	parser = OptionParser(description)
	parser.add_option('-b', '--database', dest='database',
		metavar = 'FILE', action='store',
		default = 'bayesian_sulci_databases.dat',
		help='databases summary file (default : %default).')
	parser.add_option('-o', '--output', dest='output',
		metavar = 'FILE', action='store',
		default = 'bayesian_spam_sigma.dat',
		help='storing computed sigma')
	parser.add_option('-s', '--sulcus', dest='sulcus',
		metavar = 'NAME', action='store', default = None,
		help='compute sigma only for the specified sulcus')
	return parser, parser.parse_args(argv)


def read_database(prefix, sulcidb, sulcus):
	minfname = os.path.join(prefix, sulcidb[sulcus])
	db, header = datamind_io.ReaderMinfCsv().read(minfname)
	labels = header['labels']
	keep = ['gravityCenter_x', 'gravityCenter_y', 'gravityCenter_z']
	ind = []
	for i, l in enumerate(labels):
		if l in keep: ind.append(i)
	X = db.getX()
	X = X[X[:, 0] != 0]
	X = X[:, ind]
	db.setX(X)

	return db


def main():
	parser, (options, args) = parseOpts(sys.argv)
	sulcidb = io.read_databaselist(options.database)

	if sulcidb['data'] != 'sulci_features':
		print 'database data type must be : sulci_features'
		sys.exit(1)

	prefix = os.path.dirname(options.database)

	h = {'sulci' : {}}
	for i, labels in enumerate(sulcidb['files'].keys()):
		if isinstance(labels, list) or isinstance(labels, tuple):
			continue
		else:
			if options.sulcus is not None and \
				options.sulcus != labels: continue
			sulcus = labels
		db = read_database(prefix, sulcidb['files'], sulcus)
		g = distribution.Gaussian()
		g.fit(db)
		cov = g.covariance() + 0.00001 * numpy.identity(3)
		det = numpy.prod(numpy.linalg.eigh(cov)[0])
		sigma = numpy.sqrt(det ** (1. / 3.))
		h['sulci'][sulcus] = sigma

	if not h['sulci'].has_key('unknown'): h['sulci']['unknown'] = 2.

	# write a file with all sigma
	fd = open(options.output, 'w')
	fd.write('sigma = \\\n')
	p = pprint.PrettyPrinter(indent=4, width=1000, stream=fd)
	p.pprint(h)
	fd.close()
	

if __name__ == '__main__' : main()
