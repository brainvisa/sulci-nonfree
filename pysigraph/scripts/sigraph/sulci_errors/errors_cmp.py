#!/usr/bin/env python
import os, sys, numpy, scipy.stats
from optparse import OptionParser
import datamind.io as datamind_io
from sulci.common import io as sulci_io, add_translation_option_to_parser



################################################################################
def compute_local(X1, X2, test):
	if hasattr( numpy, 'unique1d' ):
		subjects = numpy.unique1d(list(X1[:, 0]))
		sulci = numpy.unique1d(list(X1[:, 1]))
	else:
		subjects = numpy.unique(list(X1[:, 0]))
		sulci = numpy.unique(list(X1[:, 1]))
	sulci_n = len(sulci)
	res = {}
	for sulcus in sulci:
		ind1 = [i for i, s in enumerate(X1[:, 1]) if s == sulcus]
		ind2 = [i for i, s in enumerate(X2[:, 1]) if s == sulcus]
		X1s, X2s = X1[ind1, :], X2[ind2, :]
		val1, val2 = [], []
		for s in subjects:
			i1 = [i for i, x in enumerate(X1s[:, 0]) if x == s]
			i2 = [i for i, x in enumerate(X2s[:, 0]) if x == s]
			# missin entry -> 0 error
			if len(i1) == 0:
				v1 = X1s[0, 2:] * 0.
			else:	v1 = X1s[i1[0], 2:]
			if len(i2) == 0:
				v2 = X2s[0, 2:] * 0.
			else:	v2 = X2s[i2[0], 2:]
			val1.append(v1)
			val2.append(v2)
		val1 = numpy.array(val1)
		val2 = numpy.array(val2)
		print "*", sulcus
		pvals = []
		for n in range(min(val1.shape[1], val2.shape[1])):
			t, pval = test(val1[:, n],val2[:, n])
			pval *= sulci_n
			pvals.append(pval)
			print "-%s) t (bonferoni corrected pval) = %f, (%e)" % \
				(X1.colnames()[n + 2], t, pval)
		res[sulcus] = pvals
	return res


def compute_global(X1, X2, test):
	subjects = X1[:, 0]
	val1, val2 = [], []
	for s in subjects:
		i1 = [i for i, x in enumerate(X1[:, 0]) if x == s]
		i2 = [i for i, x in enumerate(X2[:, 0]) if x == s]
		v1 = X1[i1[0], 1:-1]
		v2 = X2[i2[0], 1:-1]
		val1.append(v1)
		val2.append(v2)
	val1 = numpy.array(val1)
	val2 = numpy.array(val2)
	pvals = []
	for n in range(val1.shape[1]):
		t, pval = test(val1[:, n], val2[:, n])
		pvals.append(pval)
		print "-%s) t (pval) = %f (%e)" %(X1.colnames()[n + 1], t, pval)
	return {'global' : pvals}


################################################################################
def parseOpts(argv):
	description = 'compare global and local errors and compute p-values '+\
		'to detect significant modifications'
	parser = OptionParser(description)
	parser.add_option('-1', '--data1', dest='data1',
		metavar = 'FILE', action='store', default = None,
		help='first input CSV data')
	parser.add_option('-2', '--data2', dest='data2',
		metavar = 'FILE', action='store', default = None,
		help='second input CSV data')
	parser.add_option('-o', '--output', dest='output',
		metavar = 'FILE', action='store', default = None,
		help='csv output file')
	parser.add_option('-m', '--mode', dest='mode', type='choice',
		metavar = 'MODE', action='store', default = 'wilcoxon',
		choices = ('wilcoxon', 'ttest'), help='mode : wilcoxon, ttest')

	return parser, parser.parse_args(argv)


def main():
	# options
	parser, (options, args) = parseOpts(sys.argv)
	inputs = args[1:]
	if None in [options.data1, options.data2]:
		parser.print_help()
		sys.exit(1)

	# read
	datamind_reader = datamind_io.ReaderCsv()
	X1 = datamind_reader.read(options.data1)
	X2 = datamind_reader.read(options.data2)
	dims1 = X1.colnames()
	dims2 = X1.colnames()
	if numpy.any(dims1 != dims2):
		print "error: headers of the 2 input files does not matched."
		sys.exit(1)

	# compute	
	if options.mode == 'wilcoxon': test = scipy.stats.wilcoxon
	elif options.mode == 'ttest': test = scipy.stats.ttest_ind
	if dims1[0] == 'subjects' and dims1[1] == 'sulci':
		res = compute_local(X1, X2, test)
		fd = open(options.output, 'w')
		header = "sulci\t"
		header += "\t".join(X1.colnames()[2:])
		fd.write(header + '\n')
		for sulcus, pvals in res.items():
			line = sulcus + '\t'
			line += "\t".join(("%e" % p) for p in pvals)
			fd.write(line + '\n')
	elif dims1[0] == 'Subject':
		res = compute_global(X1, X2, test)
	else:	print "error : unknown file format"


if __name__ == '__main__' : main()
