#!/usr/bin/env python
import os, sys, numpy
from optparse import OptionParser
from sulci.common import io
from sulci.models import distribution, distribution_aims


################################################################################
def compute_entropy(sulcus, spam):
	img_density = spam.img_density()
	array = img_density.volume().get().arraydata()
	array = array[array > 0]
	entropy = -(array * numpy.log(array)).sum()
	return entropy

################################################################################
def parseOpts(argv):
	description = 'compute sulcuswise entropy' + \
		'./sulcal_models_entropy.py [OPTIONS]'
	parser = OptionParser(description)
	parser.add_option('-d', '--distrib', dest='distribname',
		metavar = 'FILE', action='store', default = None,
		help='distribution models')
	parser.add_option('-o', '--output', dest='output',
		metavar = 'FILE', action='store', default = None,
		help='csv of sulcuswise entropy')
	parser.add_option('-s', '--sulci', dest='sulci',
		metavar = 'LIST', action='store', default = None,
		help='tag only specified manually tagged sulci.')

	return parser, parser.parse_args(argv)


def main():
	# read
	parser, (options, args) = parseOpts(sys.argv)
	if None in [options.output, options.distribname]:
		parser.print_help()
		sys.exit(1)
	if options.sulci is None:
		selected_sulci = None
	else:	selected_sulci = options.sulci.split(',')
	model = io.read_segments_distrib(options.distribname, selected_sulci)

	# compute
	entropies = {}
	for sulcus, distrib in model['vertices'].items():
		entropy = compute_entropy(sulcus, distrib)
		entropies[sulcus] = entropy

	# write
	if options.output == '-':
		fd = sys.stdout
	else:	fd = open(options.output, 'w')
	s = 'sulci\tentropy\n'
	for sulcus, entropy in entropies.items():
		s += '%s\t%2.2f\n' % (sulcus, entropy)
	s += 'all\t%2.2f\n' % numpy.sum(entropies.values())
	fd.write(s)
	fd.close()

if __name__ == '__main__' : main()
