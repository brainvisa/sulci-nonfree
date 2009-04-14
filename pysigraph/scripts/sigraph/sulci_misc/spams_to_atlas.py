#!/usr/bin/env python
import os, sys, numpy, pprint
from optparse import OptionParser
from soma import aims, aimsalgo
from sulci.common import io
from sulci.models import distribution, distribution_aims
from sulci.models.distribution import UniformFrequency


################################################################################
def compute_bb(models):
	cmins = []
	cmaxs = []
	for sulcus, spam in models.items():
		off, size = spam.bb_talairach()
		off = numpy.asarray(off)
		size = numpy.asarray(size)
		cmins.append(off)
		cmaxs.append(off + size)
	off = numpy.min(cmins, axis=0).astype('int') - 1
	size = (numpy.max(cmaxs, axis=0) - off).astype('int') + 1
	return off, size

def getSize(img):
	return img.dimX(), img.dimY(), img.dimZ(), img.dimT()

def select_priors(models, priors_distribs=None, selected_sulci=None):
	sulci = models.keys()
	sulci_n = len(sulci)
	if priors_distribs:
		map = {}
		freqs = []
		for i, priors_distrib in enumerate(priors_distribs):
			freq = priors_distrib['prior'].frequencies()
			freq = numpy.asarray(freq).ravel()
			freqs.append(freq)
			labels = priors_distrib['labels']
			for j, l in enumerate(labels): map[l] = (i, j)
		priors = numpy.zeros(sulci_n)
		for k, sulcus in enumerate(sulci):
			i, j = map[sulcus]
			priors[k] = freqs[i][j]
	else:	priors = UniformFrequency(sulci_n).frequencies()
	return numpy.ravel(numpy.asarray(priors))

################################################################################
def make_atlas_deterministic(models, priors, bb, threshold,
					output, voxels_size):
	off, size = bb
	sulci_map = {'background' : 0}
	priors /= priors.sum()

	# normalization factor
	proba = aims.Volume_FLOAT(*size)
	labels = aims.Volume_U8(*size)
	L = labels.arraydata()
	P = proba.arraydata()
	L.fill(0)
	P.fill(0.)
	for i, (sulcus, spam) in enumerate(models.items()):
		img_density = spam.img_density()
		off_spam, size_spam = spam.bb_talairach()
		li = img_density.volume().get().arraydata() * priors[i]
		# limits
		doff = (off_spam - off).astype('int')
		pi, pa = doff, (doff + size_spam).astype('int')
		ind = i + 1
		Lbb = L[:, pi[2]:pa[2], pi[1]:pa[1], pi[0]:pa[0]]
		Pbb = P[:, pi[2]:pa[2], pi[1]:pa[1], pi[0]:pa[0]]
		Ps = (li > Pbb)
		Lbb[Ps] = ind
		Pbb[Ps] = li[Ps]
		sulci_map[ind] = sulcus
	# brain mask
	shift = 10.
	L[P < threshold * numpy.exp(shift)] = 0

	# add motion
	trans = aims.Motion()
	trans.setTranslation(off)
	del L
	del P
	labels = aims.AimsData_U8(labels)

	# resample
	tr = aims.Motion()
	tr.setToIdentity()
	if voxels_size != 1:
		sX, sY, sZ, sT = getSize(labels)
		labels2 = aims.AimsData_S16(sX, sY, sZ, sT)
		converter = aims.Converter_Volume_U8_Volume_S16()
		converter.convert(labels, labels2)
		sX /= voxels_size
		sY /= voxels_size
		sZ /= voxels_size
		#labels2 = aims.AimsData_U8(sX, sY, sZ, sT)
		labels3 = aims.AimsData_S16(sX, sY, sZ, sT)
		#resampler = aimsalgo.NearestNeighborResampler_U8()
		resampler = aimsalgo.NearestNeighborResampler_S16()
		tr.scale([1.] * 3, [voxels_size] *3)
		resampler.resample(labels2, tr, 0, labels3, False)
	else:	labels3 = labels

	header = labels3.header()
	s = aims.vector_STRING()
	s.append('Talairach-AC/PC-Anatomist')
	header['referentials'] = s
	header['transformations'] = [(trans * tr.inverse()).toVector()]

	# write
	aims.Writer().write(labels3, output)

	return sulci_map
		

def make_atlas_probabilistic(models, priors, bb, threshold,
					output, voxels_size):
	output = os.path.splitext(output)
	off, size = bb
	sulci_map = {'background' : 0}
	priors /= priors.sum()

	# normalization factor
	for i, (sulcus, spam) in enumerate(models.items()):
		proba = aims.Volume_FLOAT(*size)
		P = proba.arraydata()
		P.fill(0.)
		img_density = spam.img_density()
		off_spam, size_spam = spam.bb_talairach()
		li = img_density.volume().get().arraydata() * priors[i]
		# limits
		doff = (off_spam - off).astype('int')
		pi, pa = doff, (doff + size_spam).astype('int')
		ind = i + 1
		Pbb = P[:, pi[2]:pa[2], pi[1]:pa[1], pi[0]:pa[0]]
		Ps = (li > Pbb)
		Pbb[Ps] = li[Ps]
		sulci_map[ind] = sulcus

		# brain mask
		shift = 10.
		P[P < threshold * numpy.exp(shift)] = 0

		# add motion
		trans = aims.Motion()
		trans.setTranslation(off)
		del P
		proba = aims.AimsData_FLOAT(proba)


		# resample
		tr = aims.Motion()
		tr.setToIdentity()
		if voxels_size != 1:
			sX, sY, sZ, sT = getSize(proba)
			sX /= voxels_size
			sY /= voxels_size
			sZ /= voxels_size
			proba2 = aims.AimsData_FLOAT(sX, sY, sZ, sT)
			resampler = aimsalgo.NearestNeighborResampler_FLOAT()
			tr.scale([1.] * 3, [voxels_size] *3)
			resampler.resample(proba, tr, 0, proba2, False)
		else:	proba2 = proba

		header = proba2.header()
		s = aims.vector_STRING()
		s.append('Talairach-AC/PC-Anatomist')
		header['referentials'] = s
		header['transformations'] = [(trans * tr.inverse()).toVector()]

		# write
		aims.Writer().write(proba2,output[0] + "_" + sulcus + output[1])
		del header
		del proba
		del proba2

	return sulci_map
	

################################################################################
def parseOpts(argv):
	description = 'compute voxel-based entropies for spam models\n' + \
		'./spams_to_atlas.py [OPTIONS] -d distrib.dat -o output_image -m sulci_map'
	parser = OptionParser(description)
	parser.add_option('-d', '--distrib', dest='distribnames',
		metavar = 'FILE/LIST', action='store', default = None,
		help='(list of) distribution models')
	parser.add_option('-p', '--prior', dest='priornames',
		metavar = 'FILE/LIST', action='store', default = None,
		help='(list of) related prior files (default : no prior)')
	parser.add_option('-o', '--output', dest='output',
		metavar = 'FILE', action='store', default = None,
		help='output 3D volume')
	parser.add_option('-m', '--sulci-map', dest='sulci_map',
		metavar = 'FILE', action='store', default = None,
		help='output sulcus map (indices)')
	parser.add_option('-t', '--threshold', dest='threshold', type='int',
		metavar = 'INT', action='store', default = 10e-15,
		help='threshold to create a masked atlas (default: %default)')
	parser.add_option('-s', '--sulci', dest='sulci',
		metavar = 'LIST', action='store', default = None,
		help='tag only specified manually tagged sulci.')
	parser.add_option('--mode', dest='mode',
		metavar = 'STR', action='store', default='deterministic',
		type='choice', choices=('deterministic', 'probabilistic'),
		help="'deterministic' or 'probabilistic'")
	parser.add_option('--voxels-size', dest='voxels_size',
		metavar = 'float', action='store', default=1., type='float',
		help="size of voxels in millimeters use '3' for functionnal "+ \
		"standard size and '1' for anatomical standard size "+\
		"(default: %default)")

	return parser, parser.parse_args(argv)


def main():
	parser, (options, args) = parseOpts(sys.argv)
	if None in [options.distribnames, options.output, options.sulci_map]:
		print 'error : missing distribution'
		parser.print_help()
		sys.exit(1)

	# options
	if options.sulci is None:
		selected_sulci = None
	else:	selected_sulci = options.sulci.split(',')


	# read
	distribnames = options.distribnames.split(',')
	models = {}
	for distribname in distribnames:
		model = io.read_segments_distrib(distribname, selected_sulci)
		models.update(model['vertices'])
	if options.priornames:
		priornames = options.priornames.split(',')
		priors_distribs = []
		for priorname in priornames:
			priors_distribs.append(io.read_labels_prior_model(\
								priorname))
	else:	priors_distribs = None

	# compute
	bb = compute_bb(models)
	priors = select_priors(models, priors_distribs, selected_sulci)
	opt = [models, priors, bb, options.threshold, options.output,
		options.voxels_size]
	if options.mode == 'deterministic':
		sulci_map = make_atlas_deterministic(*opt)
	else:	sulci_map = make_atlas_probabilistic(*opt)

	# write
	fd = open(options.sulci_map, 'w')
	fd.write('sulci_map = \\\n')
	p = pprint.PrettyPrinter(indent=4, width=1000, stream=fd)
	p.pprint(sulci_map)
	fd.close()


if __name__ == '__main__' : main()
