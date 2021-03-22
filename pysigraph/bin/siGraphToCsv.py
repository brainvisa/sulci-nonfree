#!/usr/bin/env python
from __future__ import print_function
from __future__ import absolute_import
import sys, os, numpy
from optparse import OptionParser
from sulci.common import io, add_translation_option_to_parser
import sigraph
from soma import *

def parseOpts(argv):
	description = 'measure nodewise features and compute their means ' \
		'over sulci labels of several graphs\n' \
		'./siGraphToCsv.py [OPTIONS] graph1.arg graph2.arg...'
	parser = OptionParser(description)
	add_translation_option_to_parser(parser)
	parser.add_option('-f', '--features', dest='features',
		metavar = 'LIST', action='store', default = None,
		help='features list (comma separated) at node level. Only '
		'floating data is supported.')
	parser.add_option('--all-csv', dest='allcsvname',
		metavar = 'FILE', action='store', default = None,
		help='csv file storing all measures')
	parser.add_option('-c', '--csv', dest='csvname',
		metavar = 'FILE', action='store', default = None,
		help='csv file storing measured means')

	return parser, parser.parse_args(argv)

def main():
	# handle options
	parser, (options, args) = parseOpts(sys.argv)
	graphnames = args[1:]
	parser_error = False
	if len(graphnames) == 0:
		print("you must give at least one graph")
		parser_error = True
	if options.features is None:
		print("you must specified at least one feature")
		parser_error = True
	if options.csvname is None:
		print("you must specified an output csv")
		parser_error = True
	if parser_error:
		parser.print_help()
		sys.exit(1)
	features = options.features.split(',')

	graphs = io.load_graphs(options.transfile, graphnames)
	h = {}
	for g in graphs:
		for v in g.vertices():
			if v.getSyntax() != 'fold': continue
			sulcus = v['name']
			data = []
			for feature in features:
				try:
					data.append(v[feature])
				except KeyError as e:
					print("%s (%s) unknown feature %s" % \
						(sulcus, v['index'], e))
					data.append(numpy.nan)
			data = numpy.array(data)
			if sulcus not in h:
				h[sulcus] = [data]
			else:	h[sulcus].append(data)
	h_mean = {}
	for sulcus, data in h.items():
		data = numpy.vstack(data)
		# m = mean without nan counting
		s = numpy.nansum(data, axis=0)
		m = s / (numpy.isnan(data) != True).sum(axis=0)
		h_mean[sulcus] = m

	header = 'sulci'
	for f in features: header += '\t%s' % f
	if options.allcsvname:
		fd2 = open(options.allcsvname, 'w')
		fd2.write(header + '\n')
		for sulcus, data in h.items():
			s = ''
			for d in data:
				s += sulcus + '\t' + '\t'.join(repr(x) \
					for x in d.tolist()) + '\n'
			fd2.write(s)


	fd = open(options.csvname, 'w')
	fd.write(header + '\n')
	for sulcus, data in h_mean.items():
		s = sulcus + '\t'
		s += '\t'.join(repr(x) for x in data.tolist()) + '\n'
		fd.write(s)
	fd.close()

if __name__ == '__main__': main()
