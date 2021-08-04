#!/usr/bin/env python

from __future__ import print_function
from __future__ import absolute_import
import sys, os, numpy, pprint
from optparse import OptionParser
import sigraph
from soma import aims
import datamind.io.old_csvIO as datamind_io
from sulci.common import io, add_translation_option_to_parser, vertex2voxels
from datamind.ml.database import DbNumpy


def compute(graph_list):
	print("creating database...")
	voxels_dict = {}
	size = len(graph_list)
	for i, g in enumerate(graph_list):
		sys.stdout.write("%d/%d\r" % (i + 1, size))
		sys.stdout.flush()
		motion = aims.GraphManip.talairach(g)
		for v in g.vertices():
			if v.getSyntax() != 'fold': continue
			label = v['name']
			voxels = vertex2voxels(motion, v)
			if label in voxels_dict:
				voxels_dict[label] += voxels
			else:	voxels_dict[label] = voxels
	return voxels_dict


def save2csv(dbdir, voxels_dict):
	print("saving to csv...")
	w = datamind_io.WriterCsv()
	size = len(voxels_dict)
	h = { 'data' : 'voxels_coordinates', 'files' : {} }
	for i, (label, voxels) in enumerate(voxels_dict.items()):
		csvname, minfname = io.sulci2databasename(dbdir, label)
		sys.stdout.write("%d/%d\r" % (i + 1, size))
		sys.stdout.flush()
		voxels = numpy.vstack(voxels)
		db = DbNumpy(voxels)
		header = { 'X' : ['X', 'Y', 'Z'] }
		w.write(csvname, db, header, minfname)
		h['files'][label] = minfname
	summary_file = dbdir + '.dat'
	io.write_databases(summary_file, h)


def parseOpts(argv):
	description = 'Create databases for voxels based model.\n' \
		'./create_voxels_databases.py [OPTIONS] ' \
		'graph1.arg graph2.arg...'
	parser = OptionParser(description)
	add_translation_option_to_parser(parser)
	parser.add_option('-d', '--dbdir', dest='dbdir',
		metavar = 'FILE', action='store',
		default = 'bayesian_voxels_databases',
		help='output databases directory (default : %default).' \
			'A file named FILE.dat is created to store ' \
			'labels/databases links.')
	return parser, parser.parse_args(argv)


def main():
	parser, (options, args) = parseOpts(sys.argv)
	graphnames = args[1:]
	if len(graphnames) == 0:
		parser.print_help()
		sys.exit(1)

	try:	os.mkdir(options.dbdir)
	except OSError as e:
		print("warning: directory '%s' allready exists" % dbdir)

	graphs = io.load_graphs(options.transfile, graphnames)
	voxels_dict = compute(graphs)
	save2csv(options.dbdir, voxels_dict)


if __name__ == '__main__' : main()
