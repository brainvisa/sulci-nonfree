#!/usr/bin/env python
import os, sys, numpy
from optparse import OptionParser


def parseOpts(argv):
	description = 'Annealing to label sulci.'
	parser = OptionParser(description)
	parser.add_option('-n', '--folds-number', dest='folds_number',
		metavar='FILE', action='store', default=None,
		help='number of crossvalidation folds (default : leave ' + \
		'one out)')
	parser.add_option('-f', '--graphs-files', dest='graphsfile',
		metavar='FILE', action='store', default=None,
		help='file with one sulci graph per line')

	return parser, parser.parse_args(argv)

def main():
	parser, (options, args) = parseOpts(sys.argv)
	# read graph names
	graphsfile = options.graphsfile
	fd = open(graphsfile)
	lines = fd.readlines()
	lines = [l.rstrip('\n') for l in lines]
	fd.close()

	if options.folds_number:
		folds_n = int(options.folds_number)
	else:	folds_n = len(lines)

	# cv :
	size = len(lines)
	ind = range(size)
	fold_size = size / folds_n
	s1 = (size - fold_size) / (folds_n - 1)
	for i in range(folds_n):
		# split
		test = ind[i * s1:i * s1 + fold_size]
		train = ind[:i * s1] + ind[i * s1 + fold_size:]

		# directory
		dir = "cv_%d" % i
		os.mkdir(dir)

		# test
		testfile = os.path.join(dir, "test_graphs.dat")
		fd = open(testfile, 'w')
		for i in test: fd.write(lines[i] + ' ')
		fd.close()

		# train
		trainfile = os.path.join(dir, "train_graphs.dat")
		fd = open(trainfile, 'w')
		for i in train: fd.write(lines[i] + ' ')
		fd.close()

if __name__ == '__main__': main()
