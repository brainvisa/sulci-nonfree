#!/usr/bin/env python2

import re, os, sys, numpy
from optparse import OptionParser
import sigraph
from soma import aims
from sulci.common import io, add_translation_option_to_parser
from sulci.models import distribution, distribution_aims
from sulci.models import check_same_distribution

################################################################################
# FIXME : rbf ?

#def fit_rbf(spam):
	# fit rbf
#	rbf = distribution.Rbf(mode='normalized_gaussian',std=4., normalize_kernel=True)
	# marchotte
	#rbf.fit_from_Spam(spam, n=1000, pct=0.01, weighted=True,
	#		interpolation=False, normalize_weights=True)
#	rbf.fit_from_Spam(spam, n=3000, pct=0.1, weighted=True,
#			interpolation=False, normalize_weights=True)
#	return rbf

def autofit_rec(spam, n, kmin, kmax, side, (kbest, bic_best), h):
	print "[%d %d]" % (kmin, kmax)
	k0 = 1
	if side == 1:
		klist = [kmin] * 3
	else:	klist = [kmax] * 3
	while 1:
		if side == 1:
			k = k0 * side + kmin
			if k > kmax: break
		elif side == -1:
			k = k0 * side + kmax
			if k < kmin: break
		klist.append(k)
		print "\tk = ", k
		if not h.has_key(k):
			gmm, bic = fit_gmm(spam, n, k)
			h[k] = gmm, bic
		else:	gmm, bic = h[k]
		if bic > bic_best: break
		kbest = k
		bic_best = bic
		k0 *= 2
	if side == 1:
		k1, k2, k3 = klist[-3], klist[-2], klist[-1]
	else:	k1, k2, k3 = klist[-1], klist[-2], klist[-3]
	if abs(k1 - k2) > 1:
		k, bic = autofit_rec(spam, n, k1, k2, -1, (kbest, bic_best), h)
		if bic < bic_best:
			bic_best = bic
			kbest = k
	if abs(k2 - k3) > 1:
		k, bic = autofit_rec(spam, n, k2, k3, 1, (kbest, bic_best), h)
		if bic < bic_best:
			bic_best = bic
			kbest = k
	return kbest, bic_best


def autofit_gmm(spam, n):
	h = {}
	k, bic = autofit_rec(spam, n, 0, 32, 1, (1, numpy.inf), h)
	print "h = ", h
	return h[k]

def fit_gmm(spam, n, k):
	# fit gmm
	gmm = distribution.GmmFromSpam()
	#bic = gmm.fit(spam, n, k, freq=1,
	#		eps=10e-4, itermin=50, verbose=1)
	bic = gmm.fit(spam, n, k, freq=1,
			eps=10e-4, itermin=10, itermax=40,verbose=1)
	return gmm, bic

################################################################################
def update_spam(spam, distr):
	'''
    update likelihoods of spam from distr
	'''
	img = spam._img_density.volume().arraydata()
	t = numpy.array(spam._bb_talairach_offset)
	s = numpy.array(spam._bb_talairach_size)
	from numpy.lib import index_tricks
	shape = tuple(numpy.array(img.shape)[1:][::-1])
	X = numpy.array([x for x in index_tricks.ndindex(shape)])
	X += t
	d = 100
	n = (X.shape[0] / d)
	li = []
	for i in range(d + 1):
		Xi = X[i * n: (i + 1) * n]
		logli, li_i = distr.likelihoods(Xi)
		li.append(li_i)
	li = numpy.hstack(li)
	print "mse = ", (img.ravel() - li).mean()
	img[:] = li.reshape(shape).T[None]


################################################################################
def parseOpts(argv):
	description = 'Convert spams to GMM. Graphs are used to compute ' + \
		'the total number of voxels used to compute each spam' + \
		'./approximateSpams.py [OPTIONS]'
	parser = OptionParser(description)
	add_translation_option_to_parser(parser)
	parser.add_option('-i', '--input', dest='input_distrib',
		metavar = 'FILE', action='store', default = None,
		help='input spam distrib')
	parser.add_option('-o', '--output', dest='output_distrib',
		metavar = 'FILE', action='store',
		default = 'posterior_independent.arg',
		help='output distrib directory')
	parser.add_option('-s', '--sulci', dest='selected_sulci',
		metavar = 'LIST', action='store', default = None,
		help='convert only specified sulci')
	parser.add_option('-n', '--voxels-number', dest='voxels_filename',
		metavar = 'LIST', action='store', default = None,
		help='python dictionnary stored in a file : each sulcus (key)'+\
		' gives its related number of voxels (used to learn SPAM).')
	parser.add_option('-k', '--gaussian-number', dest='k', type='int',
		metavar = 'INT', action='store', default = None,
		help='number of gaussians in GMM')
	parser.add_option('--skip', dest='skip', action='store_true',
		default = False, help='skip model when its output file ' + \
		'already exist')

	return parser, parser.parse_args(argv)

def main():
	# options
	parser, (options, args) = parseOpts(sys.argv)
	if None in [options.input_distrib, options.output_distrib]:
		parser.print_help()
		sys.exit(1)

	if options.selected_sulci is None:
		selected_sulci = None
	else:	selected_sulci = options.selected_sulci.split(',')

	if options.voxels_filename:
		voxels_n = io.read_from_exec(options.voxels_filename,
						'voxels_number')
	else:	voxels_n = None

	# read input
	input_distrib = io.read_segments_distrib(options.input_distrib,
			selected_sulci=selected_sulci)
	check, models_types = check_same_distribution(\
				input_distrib['vertices'])
	if not check:
		print "error : only one model_type is supported. Found : " +\
			str(list(models_types))
		sys.exit(1)
	model_type = list(models_types)[0]
	if model_type != 'spam':
		print "error : '%s' : no supported. " % model_type + \
			"Convert only spam model"
		sys.exit(1)

	# create output directory
	model_type = 'gmm_from_spam'
	prefix = options.output_distrib
	if not options.skip:
		try:	os.mkdir(prefix)
		except OSError, e:
			print "warning: directory '%s' allready exists" % prefix

	level = input_distrib['level']
	data_type = input_distrib['data_type']
	h = {'level' : level, 'data_type' : data_type, 'files' : {}}
	for sulcus, filename in input_distrib['vertices'].items():
		if voxels_n:
			voxels_n_sulcus = voxels_n[sulcus]
		else:	voxels_n_sulcus = 1.
		if (selected_sulci != None) and (sulcus not in selected_sulci):
			continue
		filename = io.node2densityname(prefix,
				'gmm', sulcus)
		if (options.skip and os.path.exists(filename)):
			print "skip '%s'" % sulcus
			continue
		print "*** %s ***" % sulcus
		spam = input_distrib['vertices'][sulcus]
		if options.k is not None:
			gmm, bic = fit_gmm(spam, voxels_n_sulcus, options.k)
		else:	gmm, bic = autofit_gmm(spam, voxels_n_sulcus)
		print "bic, k = ", bic, gmm._k
		gmm.write(filename)
		h['files'][sulcus] = (model_type, filename)

	# write distribution summary file
	summary_file = options.output_distrib + '.dat'
	io.write_pp('distributions', summary_file, h)

if __name__ == '__main__' : main()
