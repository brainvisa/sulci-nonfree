#!/usr/bin/env python2
import sys, os, numpy
from optparse import OptionParser
from soma import aims, aimsalgo
from sulci.common import io


################################################################################
# Talairach -> MNI
transformation = "/home/perrot/p4/shared-stable/transformation/talairach_TO_spm_template_novoxels.trm"
tr_a2f = aims.read(transformation)

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
	return img.getSizeX(), img.getSizeY(), img.getSizeZ(), img.getSizeT()

################################################################################
def anat2func(a, tr_a, f, tr_f, options):
	# transformations
	tr_mm2vox = aims.Motion()
	tr_mm2vox.scale([1.] * 3, [3.] *3)
	tr = tr_mm2vox * tr_f.inverse() * tr_a2f * tr_a

	# convert input
	sX, sY, sZ, sT = getSize(a)
	if not options.float:
		a2 = aims.AimsData_S16(sX, sY, sZ, sT)
		converter = aims.Converter_Volume_U8_Volume_S16()
		converter.convert(a, a2)
	else:	a2 = aims.Volume_FLOAT(a)

	# create output
	sX, sY, sZ, sT = getSize(f)
	if not options.float:
		b = aims.AimsData_S16(sX, sY, sZ, sT)
	else:	b = aims.AimsData_FLOAT(sX, sY, sZ, sT)
	if not options.float:
		resampler = aimsalgo.NearestNeighborResampler_S16()
	else:	resampler = aimsalgo.NearestNeighborResampler_FLOAT()

	# resample
	resampler.resample(a2, tr, 0, b, False)
	bheader = b.volume().header()
	bheader['voxel_size'] = f.header()['voxel_size']
	s = aims.vector_STRING()
	s.append('Talairach-MNI template-SPM')
	bheader['referentials'] = s
	bheader['transformations'] = [f.header()['transformations'][0]]
	return b

def func2tal_global(bb, tr_a, f, tr_f, options):
	# transformations
	off, size = bb
	tr_off = aims.Motion()
	tr_off.setTranslation(off)
	tr_mm2vox = aims.Motion()
	tr_mm2vox.scale([1.] * 3, [3.] *3)
	tr = tr_off.inverse() * tr_a * tr_a2f.inverse() * tr_f

	# convert input
	if options.float:
		f2 = f
	else:
		sX, sY, sZ, sT = getSize(f)
		f2 = aims.AimsData_FLOAT(sX, sY, sZ, sT)
		header = f2.volume().header()
		header['voxel_size'] = f.header()['voxel_size']
		converter = aims.Converter_Volume_S16_Volume_FLOAT()
		converter.convert(f, f2)

	# create output
	sX, sY, sZ = size
	sX /= 3
	sY /= 3
	sZ /= 3
	sT = 1
	f3 = aims.AimsData_FLOAT(sX, sY, sZ, sT)
	header = f3.volume().header()
	header['voxel_size'] = f.header()['voxel_size']
	s = aims.vector_STRING()
	s.append('Talairach-AC/PC-Anatomist')
	header['referentials'] = s
	header['transformations'] = [tr_off.toVector()]
	resampler = aimsalgo.LinearResampler_FLOAT()
	resampler.resample(f2, tr, 0, f3, False)

	return f3
	

################################################################################
def parseOpts(argv):
	description = 'Anat -> MNI space:\n' + \
	'\tanat2functional.py -r func -i anat -o anat2 [--float]' + \
	'Functional -> Talairach with global registration:'
	'\tanat2functional.py -r func -d spams -t trm -o func2 --output-space Tal_global [--float]'
	parser = OptionParser(description)
	parser.add_option('-i', '--input', dest='input',
		metavar = 'FILE', action='store', default = None,
		help='input anatomic/func image')
	parser.add_option('-o', '--output', dest='output',
		metavar = 'FILE', action='store', default = None,
		help='output image')
	parser.add_option('-d', '--distrib', dest='distribnames',
		metavar = 'FILE/LIST', action='store', default = None,
		help='(list of) distribution models')
	parser.add_option('-r', '--ref-func', dest='func',
		metavar = 'FILE', action='store', default = None,
		help='input functional image defining the MNI space')
	parser.add_option('-t', '--trm', dest='trm',
		metavar = 'FILE', action='store', default = None,
		help='affine transformation from Talairach space to ' + 
		'refined Talairach space')
	parser.add_option('--float', dest='float',
		metavar = 'FILE', action='store_true', default=False,
		help='floating values or not')
	parser.add_option('--output-space', dest='output_space',
		metavar = 'STR', action='store', type="choice",
		choices=("MNI", "Tal_global"), default="MNI",
		help="'MNI' or 'Tal_global' (default: %default). 'Tal_global' "+
		"is the Talairach space refined with global registration.")
	return parser, parser.parse_args(argv)

def main():
	parser, (options, args) = parseOpts(sys.argv)
	if None in [options.output, options.func] or \
		(options.output_space == 'MNI' and None in [options.input]) or\
		(options.output_space == 'Tal_global' and \
			None in [options.trm]):
		print "missing option(s)"
		parser.print_help()
		sys.exit(1)


	f = aims.read(options.func)
	tr_f = aims.Motion(f.header()['transformations'][0])

	if options.output_space == 'MNI':
		a = aims.read(options.input)
		tr_a = aims.Motion(a.header()['transformations'][0])
		output_volume = anat2func(a, tr_a, f, tr_f, options)
	elif options.output_space == 'Tal_global':
		tr_a = aims.read(options.trm)
		distribnames = options.distribnames.split(',')
		models = {}
		for distribname in distribnames:
			model = io.read_segments_distrib(distribname)
			models.update(model['vertices'])
		bb = compute_bb(models)
		output_volume = func2tal_global(bb, tr_a, f, tr_f, options)

	aims.write(output_volume, options.output)

if __name__ == '__main__': main()
