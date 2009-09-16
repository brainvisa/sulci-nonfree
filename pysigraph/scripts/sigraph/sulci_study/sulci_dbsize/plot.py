#!/usr/bin/env python

import os, sys
import numpy, pylab
from optparse import OptionParser
import matplotlib


################################################################################
def parseOpts(argv):
	usage = "Usage: %prog [OPTIONS]\n" + \
		"plot error results"
	parser = OptionParser(usage)
	parser.add_option('-i', '--input', dest='input',
		metavar = 'FILE', action='store', default = None,
		help='input result filename (csv format)')
	parser.add_option('-o', '--output', dest='output',
		metavar = 'FILE', action='store', default = None,
		help='output figure/image of results')
	parser.add_option('-m', '--model', dest='model',
		metavar = 'FILE', action='store', default = None,
		help='compute results for only one selected model from ' + \
		'csv header (default: all models)')
	parser.add_option('-s', '--show-only', dest='show_only',
		metavar = 'FILE', action='store_true', default = False,
		help='only show result')
	return parser, parser.parse_args(argv)

def main():
	# options
	parser, (options, args) = parseOpts(sys.argv)
	if None in [options.input]:
		print "error: missing input"
		parser.print_help()
		sys.exit(1)

	if options.input is None and options.show_only is False:
		print "error: missing output"
		parser.print_help()
		sys.exit(1)

	fd = open(options.input)
	header = fd.readline().strip('\n').split('\t')
	if options.model:
		try:
			ind = header.index(options.model + '_mean')
		except ValueError:
			print "wrong model value see header :"
			print header
			sys.exit(1)
		labels = options.model
		usecols = [0] + list(ind + numpy.arange(3))
	else:
		usecols = None
		labels = [h.strip('_mean') for h in header \
				if h.endswith('_mean')]
	fd.close()
	x = numpy.lib.io.genfromtxt(options.input, delimiter='\t',
		skiprows=1, usecols=usecols)
	x = x[numpy.argsort(x[:, 0])]
	n = (x.shape[1] - 1) / 3

	# figure
	f = pylab.figure()
	lines = []
	for i in range(n):
		size, error, std = x[:, 0], x[:, 3 * i + 1], x[:, 3 * i + 2]
		lines.append(pylab.errorbar(size, error, std)[0])
	pylab.xticks([1, 10, 20, 30, 40, 50, 62])
	pylab.xlim(-10, 70)
	pylab.xlabel('size of database')
	pylab.ylabel('error rates (%)')
	pylab.legend(lines, labels,
		prop=matplotlib.font_manager.FontProperties(size=6))
	pylab.title(os.path.basename(options.input))
	if options.show_only:
		pylab.show()
	else:	pylab.savefig(options.output, dpi=300)


if __name__ == '__main__' : main()
