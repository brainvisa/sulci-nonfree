#!/usr/bin/env python2
import os, sys, numpy
from optparse import OptionParser
import matplotlib
matplotlib.use('QtAgg')
import pylab
from soma.gui.api import chooseMatplotlibBackend
chooseMatplotlibBackend()
from datamind import io as datamind_io
from compute_mixup_labeling_errors import save_matrix

################################################################################
def save_image(filename, sulci, M):
	f = pylab.figure()
	pylab.imshow(M, interpolation='nearest')
	pylab.grid(True, linewidth=0.1, marker='+',
		solid_capstyle='round', dashes='1000')
	f.subplots_adjust(0.0, 0.1, 1, 0.97)
	ax = f.axes[0]
	id = numpy.arange(len(sulci))+0.5
	ax.set_xticks(id)
	ax.set_yticks(id)
	ax.set_xticklabels(sulci, rotation=45,
		horizontalalignment='right', fontsize=3)
	ax.set_yticklabels(sulci, fontsize=3)
	pylab.savefig(filename, dpi=300)
	f.clear()

################################################################################
def parseOpts(argv):
	description = 'synthetize mixup matrices'
	parser = OptionParser(description)
	parser.add_option('--output_count', dest='output_count',
		metavar = 'FILE', action='store', default = None,
		help='number of mixed-up labels over the database')
	parser.add_option('--output_pct', dest='output_pct',
		metavar = 'FILE', action='store', default = None,
		help='percentage of mixed-up labels over the database')
	parser.add_option('--image', dest='image',
		metavar = 'PATTERN', action='store', default = None,
		help='save PATTERN_count.png, PATTERN_pct.png : output ' + \
			'images of synthetized matrices') 
	return parser, parser.parse_args(argv)

def main():
	# options
	parser, (options, args) = parseOpts(sys.argv)
	inputs = args[1:]
	if None in [options.output_count, options.output_pct]:
		print "error: missing options"
		parser.print_help()
		sys.exit(1)

	# reading
	matrices = [datamind_io.ReaderCsv().read(input) for input in inputs]
	sulci = matrices[0].colnames()[1:].tolist()
	sulci_n = len(sulci)

	S = numpy.zeros((sulci_n, sulci_n), dtype='float')
	for m in matrices: S += m[:, 1:]
	save_matrix(options.output_count, sulci, S)
	if options.image: save_image(options.image + '_count.png', sulci, S)
	S /= S.sum(axis=0)
	save_matrix(options.output_pct, sulci, S)
	if options.image: save_image(options.image + '_pct.png', sulci, S)


if __name__ == '__main__' : main()
