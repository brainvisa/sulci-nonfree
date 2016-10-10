#!/usr/bin/env python

from __future__ import print_function
import glob, sys, os, numpy, collections, shutil
from optparse import OptionParser
from datamind.io import csvIO
import error

def compute_prototypes(files):
	'''
    compute min, med and max error rated samples.
	'''
	errors = numpy.array([error.samples_global_mean_error(file) \
						for file in files])
	arg = numpy.argsort(errors)
	min, med, max = arg[0], arg[len(errors)/2], arg[-1]
	return {'min' : min, 'med' : med, 'max' : max}

def reformat_and_clean(directories, to_rename={},
		print_only=False, verbose=False):
	'''
    Rename directories in to_rename, and clean others.

    directories :        list of directories
    to_rename :  dictionnary of renamed destination for each indexed directory.
                 format : {'dest_dirname' : indice_src_dirname}
    print_only : do nothing, only print actions
    verbose :    print actions
	'''
	for name, ind in to_rename.items():
		src = directories[ind]
		dst = os.path.join(os.path.dirname(src), name)
		if print_only or verbose:
			if src != dst: print("move %s -> %s" % (src, dst))
		if not print_only:
			if src != dst: shutil.move(src, dst)
	ignore_indices = to_rename.values()
	kept_all_files = ['test_global.csv', 'test_graphs.dat',
		'test_local.csv', 'train_graphs.dat', 'bayesian_angle_distribs',
		'bayesian_angle_distribs.dat', 'bayesian_direction_distribs',
		'bayesian_direction_distribs.dat', 'bayesian_spam_distribs',
		'bayesian_spam_distribs.dat', 'bayesian_translation_distribs',
		'bayesian_translation_distribs.dat', 'gravity_centers',
		'gravity_centers.dat']
	for i, dir in enumerate(directories):
		files = []
		files = glob.glob(os.path.join(dir, '*'))
		if i in ignore_indices: kept_files = kept_all_files
		else:	kept_files = kept_all_files[:4]
		for value in kept_files:
			try:	files.remove(os.path.join(dir, value))
			except ValueError: pass
		for filename in files:
			if print_only or verbose:
				print("rm '%s'" % filename)
			if not print_only:
				if os.path.isfile(filename):
					os.remove(filename)
				elif os.path.isdir(filename):
					shutil.rmtree(filename)
	

################################################################################
def parseOpts(argv):
	usage = "Usage: %prog dir1 dir2...\n" + \
		"clean result directories"
	parser = OptionParser(usage)
	parser.add_option('-v', '--verbose', dest='verbose',
		action='store_true', default = False,
		help='verbose output')
	parser.add_option('-p', '--print', dest='print_only',
		action='store_true', default = False,
		help='only print actions')

	return parser, parser.parse_args(argv)

def main():
	# options
	parser, (options, args) = parseOpts(sys.argv)
	directories = args[1:]
	for dir in directories:
		files = glob.glob('%s/*/test_global.csv' % dir)
		prototypes = compute_prototypes(files)
		reformat_and_clean([os.path.dirname(f) for f in files], 
			prototypes, options.print_only, options.verbose)

if __name__ == '__main__' : main()
