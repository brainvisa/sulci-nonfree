#!/usr/bin/env python
from __future__ import print_function
from __future__ import absolute_import
import os, sys, numpy, pprint
from optparse import OptionParser
import sigraph
from soma import aims
import datamind.io.old_csvIO as datamind_io
from datamind.tools import *
from sulci.common import io, add_translation_option_to_parser
from sulci.models import distribution, distribution_all

def parseOpts(argv):
	description = 'Compute Model based on voxels.'
	parser = OptionParser(description)
	add_translation_option_to_parser(parser)
	parser.add_option('-b', '--database', dest='database',
		metavar = 'FILE', action='store',
		default = 'bayesian_voxels_databases.dat',
		help='databases summary file (default : %default).')
	parser.add_option('-d', '--distribdir', dest='distribdir',
		metavar = 'FILE', action='store', default = None,
		help='output distribution directory (default : ' \
			'bayesian_<TYPE>_distribs). A file named FILE.dat ' \
			'is created to store labels/databases links.')
	parser.add_option('-s', '--sulcus', dest='sulcus',
		metavar = 'NAME', action='store', default = None,
		help='Compute gmm only for the specified sulcus ' \
			'(default : compute all gmm)')
	parser.add_option('--type', dest='type', metavar = 'TYPE',
		action='store', default = 'gmm_full', help='type among : ' \
		'gmm_full (no param), gmm_diag (no param), gmm_spherical ' \
		'(no param), fixed_bgmm_diag (param kmax) (default : %default)')
	parser.add_option('--pct', dest='pct', metavar = 'FLOAT',
		action='store', default = None, help='Learn only the ' + \
		'specified ratio of the data definied between 0 and 1 ' + \
		'(default : take all data)')
	parser.add_option('--params', dest='params', metavar = 'LIST',
		action='store', default = None, help='parameters of selected ' \
		'model, each parameters separated by one space, all params ' \
		'took between quotes. i.e : --params "p1 p2 p3"')

	return parser, parser.parse_args(argv)

def filter_database(db, pct):
	'''
    shuffle data and select a specified ratio of this data.
	'''
	db2 = db.share()
	X = db2.getX()
	sizeX = len(X)
	ind = numpy.arange(sizeX)
	numpy.random.shuffle(ind)
	if pct is not None:
		sizeX2 = int(sizeX * float(pct))
		if sizeX2 < 100: sizeX2 = 100
		X2 = X[ind[:sizeX2]]
	else:	X2 = X[ind]
	db2.setX(X2)
	return db2


def learn_gmm_full(db):
	'''gmm : full covariance'''
	gmm = distribution.Gmm('full')
	gmm.fit(db)
	return gmm

def learn_gmm_diag(db):
	'''gmm : diagonal covariance'''
	gmm = distribution.Gmm('diagonal')
	gmm.fit(db)
	return gmm

def learn_gmm_spherical(db):
	'''gmm : spherical covariance'''
	gmm = distribution.Gmm('spherical')
	gmm.fit(db)
	return gmm

def learn_fixed_bgmm_diagonal(db, kmax):
	'''bayesian gmm : diagonal covariance, fixed number of gaussians'''
	gmm = distribution.FixedBGmm('diagonal', int(kmax))
	gmm.fit(db)
	return gmm

def main():
	parser, (options, args) = parseOpts(sys.argv)
	if None in [options.database]:
		parser.print_help()
		sys.exit(1)

	database = io.read_databaselist(options.database)

	if database['data'] != 'voxels_coordinates':
		print('database data type must be : voxels_coordinates')
		sys.exit(1)

	dbdir = options.distribdir
	type = options.type
	if not options.params: params = []
	else:	params = options.params.split(' ')
	if dbdir is None: dbdir = 'bayesian_%s_distribs' % options.type

	# choose model
	models_table = {
		'gmm_full' : (0, 'gmm', learn_gmm_full),
		'gmm_diag' : (0, 'gmm', learn_gmm_diag),
		'gmm_spherical' : (0, 'gmm', learn_gmm_spherical),
		'fixed_bgmm_diag' : (1, 'fixed_bgmm',learn_fixed_bgmm_diagonal),
	}
	if type not in models_table:
		msg.error("invalid type'%s', valid types are : %s" % \
			(type, str(list(models_table.keys()))))
		sys.exit(1)
	nb_params, model_type, learn = models_table[type]
	if nb_params != len(params):
		msg.error("model must have exactly %d parameters" % nb_params)
		sys.exit(1)

	# create output directory
	try:	os.mkdir(dbdir)
	except OSError as e:
		print("warning: directory '%s' allready exists" % dbdir)

	voxels_total = 0
	h = {'model' : model_type, 'files' : {}}
	for sulcus, minfname in database['files'].items():
		if options.sulcus is not None and sulcus != options.sulcus:
			continue
		print(minfname)
		db, header = datamind_io.ReaderMinfCsv().read(minfname)
		db = filter_database(db, options.pct)
		size = len(db)
		voxels_total += size
		distr = learn(db, *params)
		filename = io.node2densityname(dbdir, type, sulcus)
		distr.write(filename)
		h['files'][sulcus] = (model_type, filename, [size])
	h['priors_relations_names'] = []
	h['priors_relations_total'] = []
	h['priors_nodes_names'] = ['voxels_number']
	h['priors_nodes_total'] = [voxels_total]

	# write distribution summary file
	summary_file = dbdir + '.dat'
	fd = open(summary_file, 'w')
	fd.write('distributions = \\\n')
	p = pprint.PrettyPrinter(indent=4, width=1000, stream=fd)
	p.pprint(h)
	fd.close()

if __name__ == '__main__' : main()
