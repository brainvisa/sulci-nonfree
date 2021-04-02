#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function
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
	parser.add_option('-l', '--lang', dest='lang',
		metavar = 'LANG', action='store', default = 'eng',
		type='choice', choices=('eng', 'fr'),
		help="language of outputs : 'fr' or 'eng' (default)")
	parser.add_option('--no-title', dest='notitle',
		action='store_true', default = False,
		help='default: input csv filename is used as a title')
	parser.add_option('--grid', dest='grid',
		action='store_true', default = False,
		help='display grid (default: no grid)')
	parser.add_option('--legend', dest='legend',
		metavar = 'LIST', action='store', default = None,
		help="comma separated list, len much match number " + \
		"of csv columns divided by 3 (default: use csv header)")
	parser.add_option('--no-std-error', dest='no_stderr',
		action='store_true', default=False,
		help="do not display error bars")

	return parser, parser.parse_args(argv)

def main():
	# options
	parser, (options, args) = parseOpts(sys.argv)
	if None in [options.input]:
		print("error: missing input")
		parser.print_help()
		sys.exit(1)

	if options.input is None and options.show_only is False:
		print("error: missing output")
		parser.print_help()
		sys.exit(1)

	fd = open(options.input)
	header = fd.readline().strip('\n').split('\t')
	if options.model:
		try:
			ind = header.index(options.model + '_mean')
		except ValueError:
			print("wrong model value see header :")
			print(header)
			sys.exit(1)
		labels = options.model
		usecols = [0] + list(ind + numpy.arange(3))
	else:
		usecols = None
		labels = [h.strip('_mean') for h in header \
				if h.endswith('_mean')]
	fd.close()
	if options.legend:
		labels = options.legend.split(',')
	x = numpy.lib.io.genfromtxt(options.input, delimiter='\t',
		skiprows=1, usecols=usecols)
	x = x[numpy.argsort(x[:, 0])]
	n = (x.shape[1] - 1) / 3

	# figure
	f = pylab.figure()
	lines = []
	for i in range(n):
		size, error, std = x[:, 0], x[:, 3 * i + 1], x[:, 3 * i + 2]
		if options.no_stderr:
			lines.append(pylab.plot(size, error)[0])
		else:	lines.append(pylab.errorbar(size, error, std)[0])
	pylab.xticks([1, 10, 20, 30, 40, 50, 62])
	pylab.xlim(-10, 70)
	if options.lang == 'eng':
		xlabel = 'size of database'
		ylabel = 'error rates (%)'
	elif options.lang == 'fr':
		xlabel = u'taille de la base de données'
		ylabel = "taux d'erreurs (%)"
	if options.grid:
		pylab.grid()
	pylab.xlabel(xlabel)
	pylab.ylabel(ylabel)
	pylab.legend(lines, labels,
		prop=matplotlib.font_manager.FontProperties(size=6))
	if not options.notitle:
		pylab.title(os.path.basename(options.input))
	if options.show_only:
		pylab.show()
	else:	pylab.savefig(options.output, dpi=300)


if __name__ == '__main__' : main()
