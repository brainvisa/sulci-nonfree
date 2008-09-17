#!/usr/bin/env python

import sys, os, pprint, numpy
from optparse import OptionParser
import sigraph
from soma import aims
from sulci.common import io

def parseOpts(argv):
	description = 'Remove double (or more) labels edges from graphmodel ' +\
		'created by create_graphmodel.py script\n.' + \
		'./clean_graphmodel.py input output'
	parser = OptionParser(description)
	return parser, parser.parse_args(argv)

def read_graphmodel(filename):
	try:
		execfile(filename)
		o = locals()['graphmodel']
		return o
	except Exception, e:
		print e
	sys.exit(1)

def write_graphmodel(filename, graphmodel):
	io.write_pp('graphmodel', filename, graphmodel)

def main():
	parser, (options, args) = parseOpts(sys.argv)
	h = read_graphmodel(sys.argv[1])
	h['edges'] = list(set(h['edges']))
	write_graphmodel(sys.argv[2], h)

if __name__ == '__main__' : main()
