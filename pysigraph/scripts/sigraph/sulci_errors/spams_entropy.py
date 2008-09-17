#!/usr/bin/env python
import os, sys, numpy
from optparse import OptionParser
from soma import aims
from sulci.common import io
from sulci.models import distribution, distribution_aims


################################################################################
def compute_bb(spams):
	cmins = []
	cmaxs = []
	for spam in spams:
		off, size = spam.bb_talairach()
		off = numpy.asarray(off)
		size = numpy.asarray(size)
		cmins.append(off)
		cmaxs.append(off + size)
	off = numpy.min(cmins, axis=0).astype('int')
	size = (numpy.max(cmaxs, axis=0) - off).astype('int')
	return off, size

def compute_entropy_x(spams, x):
	prior = numpy.ones(len(spams)) #FIXME
	likelihoods = []
	for spam in spams:
		logli, li = spam.prodlikelihoods(x[None], shift=0.)
		likelihoods.append(li)
	likelihoods = numpy.array(likelihoods)
	posteriors = likelihoods * prior
	posteriors /= posteriors.sum()

	p = posteriors
	logp = numpy.log(p)
	logp[numpy.isneginf(logp)] = -100
	entropy = -(p * logp).sum()
	return entropy

def compute_entropy(spams, bb):
	off, size = bb
	vol = aims.Volume_FLOAT(*size)
	a = vol.arraydata()
	for x in range(20,size[0]):
		for y in range(20,size[1]):
			for z in range(20,size[2]):
				pos = numpy.array((x, y, z), dtype='float')+off
				e = compute_entropy_x(spams, pos)
				a[0, z, y, x] = e
	return vol

def compute_entropy2(spams, bb):
	off, size = bb

	# normalization factor
	volsum = aims.Volume_FLOAT(*size)
	s = volsum.arraydata()
	s.fill(0.)
	for spam in spams:
		img_density = spam.img_density()
		off_spam, size_spam = spam.bb_talairach()
		li = img_density.volume().get().arraydata() #FIXME * prior
		# limits
		doff = (off_spam - off).astype('int')
		pi, pa = doff, (doff + size_spam).astype('int')
		s[:, pi[2]:pa[2], pi[1]:pa[1], pi[0]:pa[0]] += li

	# entropy
	vol = aims.Volume_FLOAT(*size)
	a = vol.arraydata()
	a.fill(0.)
	for spam in spams:
		img_density = spam.img_density()
		off_spam, size_spam = spam.bb_talairach()
		li = img_density.volume().get().arraydata() #FIXME * prior
		# limits
		doff = (off_spam - off).astype('int')
		pi, pa = doff, (doff + size_spam).astype('int')
		sums = s[:, pi[2]:pa[2], pi[1]:pa[1], pi[0]:pa[0]]
		p = li / sums
		logp = numpy.log(p)
		logp[numpy.isneginf(logp)] = -100
		a[:, pi[2]:pa[2], pi[1]:pa[1], pi[0]:pa[0]] -= p * logp
	a[:] = numpy.exp(a)
	return vol
		

################################################################################
def parseOpts(argv):
	description = 'compute voxel-based entropies for spam models\n' + \
		'./spams_entropy.py [OPTIONS] -d distrib.dat -o output_image'
	parser = OptionParser(description)
	parser.add_option('-d', '--distrib', dest='distribname',
		metavar = 'FILE', action='store', default = None,
		help='distribution models')
	parser.add_option('-o', '--output', dest='output',
		metavar = 'FILE', action='store', default = None,
		help='output 3D volume')

	return parser, parser.parse_args(argv)


def main():
	parser, (options, args) = parseOpts(sys.argv)
	if None in [options.distribname, options.output]:
		print 'error : missing distribution'
		parser.print_help()
		sys.exit(1)

	# read
	model = io.read_bayesian_model(node_distribname=options.distribname)
	sulci = model['vertices'].keys()
	distribs = []
	for sulcus in sulci:
		info = model['vertices'][sulcus]
		distribs.append(info['density'])

	# compute
	bb = compute_bb(distribs)
	entropy_vol = compute_entropy2(distribs, bb)
	aims.Writer().write(entropy_vol, options.output)

if __name__ == '__main__' : main()
