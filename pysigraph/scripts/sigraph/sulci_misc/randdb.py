#!/usr/bin/env python2
import os, sys, numpy
from optparse import OptionParser


def parseOpts(argv):
	description = 'Randomly generate training/testing split ' + \
			'based on training database size'
	parser = OptionParser(description)
	parser.add_option('-n', '--training-size', dest='training_size',
		metavar='FILE', action='store', default=None, type='int',
		help='number of subjects for training')
	parser.add_option('-f', '--graphs-files', dest='graphsfile',
		metavar='FILE', action='store', default=None,
		help='file with one sulci graph per line')
	parser.add_option('-r', '--runs', dest='runs', type='int',
		metavar='FILE', action='store', default=1,
		help='number of run')

	return parser, parser.parse_args(argv)

def main():
	# options
	parser, (options, args) = parseOpts(sys.argv)
	if None in [options.training_size, options.graphsfile]:
		print "missing option(s)"
		parser.print_help()
		sys.exit(1)

	# read graph names
	graphsfile = options.graphsfile
	fd = open(graphsfile)
	lines = [l.rstrip('\n') for l in fd.readlines()]
	fd.close()

	# cv :
	size = len(lines)
	for i in range(options.runs):
		indices = numpy.arange(size)
		numpy.random.shuffle(indices)

		# split
		train = indices[:options.training_size]
		test = indices[options.training_size:]

		# directory
		dir = "run_%d" % i
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
