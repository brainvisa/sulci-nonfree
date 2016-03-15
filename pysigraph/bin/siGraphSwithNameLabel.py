#!/usr/bin/env python2

import os, sys, numpy, pprint
from optparse import OptionParser
import sigraph
from soma import aims


def parseOpts(argv):
	description = 'Switch label and names values'
	parser = OptionParser(description)
	parser.add_option('-i', '--ingraph', dest='input_graphname',
		metavar = 'FILE', action='store', default = None,
		help='input data graph')
	parser.add_option('-o', '--outgraph', dest='output_graphname',
		metavar = 'FILE', action='store',
		default = None,
		help='output data graph (default : input)')
	return parser, parser.parse_args(argv)

def main():
	parser, (options, args) = parseOpts(sys.argv)
	if None in [options.input_graphname]:
		parser.print_help()
		sys.exit(1)
	if options.output_graphname:
		output = options.output_graphname
	else:	output = options.input_graphname

	g = aims.read(options.input_graphname)
	for v in g.vertices():
		if v.getSyntax() != 'fold': continue
		try: name = v['name']
		except KeyError: name = 'unknown'
		try: label = v['label']
		except KeyError: label = 'unknown'
		v['label'], v['name'] = name, label

	w = sigraph.FoldWriter(output)
	w.write(g)

if __name__ == '__main__' : main()
