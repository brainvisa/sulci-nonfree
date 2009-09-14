#!/usr/bin/env python

import glob, sys, os, numpy, collections
from optparse import OptionParser
from datamind.io import csvIO

def read_global_error(filename):
	r = csvIO.ReaderCsv()
	a = r.read(filename)
	e = numpy.array(a[:, 'Weighted_Mean_Local_SI_Error'])
	return e.mean()

def read_local_error(filename):
	r = csvIO.ReaderCsv()
	a = r.read(filename)
	# list of sulci may change from one database sample to another
	sulci = numpy.unique1d(a[:, 'sulci'].tolist()).tolist()
	res = {}
	sulci_col = a[:, 'sulci']
	for sulcus in sulci:
		e = numpy.array(a[sulci_col == a.code(sulcus), 'size_errors'])
		e *= 100 # percentages
		res[sulcus] = e.mean()
	return res

def compute_global_error_info(dir):
	files = glob.glob('%s/*/test_global.csv' % dir)
	n = len(files)
	errors = numpy.zeros(n)
	for i, file in enumerate(files):
		e = read_global_error(file)
		errors[i] = e
	return errors.mean(), errors.std(), n

def compute_local_error_info(dir):
	files = glob.glob('%s/*/test_local.csv' % dir)
	n = len(files)
	res = collections.defaultdict(lambda: numpy.zeros(n))
	for i, file in enumerate(files):
		r = read_local_error(file)
		for sulcus, error in r.items():
			res[sulcus][i] = error
	for sulcus, errors in res.items():
		res[sulcus] = errors.mean(), errors.std()
	return res, n

################################################################################
def parseOpts(argv):
	usage = "Usage: %prog [OPTIONS] dir1 dir2...\n" + \
		"compute mean/std global or local error rate"
	parser = OptionParser(usage)
	parser.add_option('-o', '--output', dest='output',
		metavar = 'FILE', action='store', default = None,
		help='output csv filename')
	parser.add_option('-m', '--mode', dest='mode',
		metavar = 'FILE', action='store', default = 'global',
		type = 'choice', choices=('global', 'local'),
		help="mode : 'global' or 'local' (default: %default)")
	parser.add_option('-s', '--size', dest='size',
		metavar = 'INT', action='store', default = None,
		help='database size') # size is stored as a string

	return parser, parser.parse_args(argv)

def main():
	# options
	parser, (options, args) = parseOpts(sys.argv)
	directories = args[1:]
	error = []
	if options.output is None:
		error.append("missing output")
	if options.size is None:
		error.append("missing database size")
	if len(directories) == 0:
		error.append("missing directory")
	if len(error):
		if len(error) == 1:
			print "error: %s" % error[0]
		else:	print 'error:\n    - ' + '\n    - '.join(error)
		print
		parser.print_help()
		sys.exit(1)
	# compute errors
	if options.mode == 'global':
		main_global(options, directories)
	else:	main_local(options, directories)

def main_global(options, directories):
	s = []
	for dir in directories:
		mean, std, n = compute_global_error_info(dir)
		print "* " + dir
		print "    mean (std) = %2.2f (%2.2f), n = %d" % (mean, std, n)
		s.append('%2.2f\t%2.2f\t%d' % (mean, std, n))

	# write result
	fd = open(options.output, 'a')
	add_header_if_needed(fd, directories)
	fd.write(options.size + '\t' + '\t'.join(s) + '\n')
	fd.close()

def main_local(options, directories):
	#s = []
	res = collections.defaultdict(lambda: numpy.zeros(len(directories) * 3))
	for i, dir in enumerate(directories):
		r, n = compute_local_error_info(dir)
		for sulcus, (error, std) in r.items():
			a = res[sulcus]
			a[3 * i:3 * i + 3] = error, std, n
	
	# write sulcuswise results
	format = ['%2.2f','%2.2f','%d']
	line_format = format * len(directories)
	for sulcus, r in res.items():
		base, ext = os.path.splitext(options.output)
		fd = open(base + '_' + sulcus + ext, 'a')
		add_header_if_needed(fd, directories)
		fd.write(options.size + '\t' + '\t'.join(\
			map((lambda n,f : f % n), r, line_format)) + '\n')
		fd.close()

	# write modelwise results
	for i, dir in enumerate(directories):
		fd = open(os.path.join(dir, 'test_local.csv'), 'a')
		s = ''
		if fd.tell() == 0: s += 'sulci\tmean\tstd\tnumber\n'
		for sulcus, r in res.items():
			s += sulcus + '\t' + '\t'.join(format) % \
				tuple(r[3 * i: 3 * i + 3]) + '\n'
		fd.write(s)
		fd.close()


def add_header_if_needed(fd, directories):
	if fd.tell() == 0:
		fd.write('size\t' + '\t'.join(d.strip('/') + '_' + s \
			for d in directories
			for s in ['mean', 'std', 'number']) + '\n')

if __name__ == '__main__' : main()
