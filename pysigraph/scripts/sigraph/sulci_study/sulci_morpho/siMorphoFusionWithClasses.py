#!/usr/bin/env python

# standard
from __future__ import print_function
from __future__ import absolute_import
import os, sys, glob, re
import numpy
from optparse import OptionParser

# matplotlib
import matplotlib
matplotlib.use('QtAgg')
import pylab

# trick to use fff2 instead of fff in datamind
import fff, fff2
sys.modules['fff'] = fff2

# datamind / soma
from datamind.io import DF 
from datamind.io import ReaderCsv, WriterCsv

################################################################################
seg_fd3 = ['extremity1x', 'extremity1y', 'extremity1z',
	'extremity2x', 'extremity2y', 'extremity2z',
	'gravityCenter_x', 'gravityCenter_y', 'gravityCenter_z',
	'normal_x', 'normal_y', 'normal_z',
	'direction_x', 'direction_y', 'direction_z',
	'volume', 'geodesicDepthMax', 'surface']

seg_fd3_red1 = ['hullJunctionsSize', 'geodesicDepthMax', 'surface']

rel_fd2 = ['volumeS1', 'volumeS2', 'corticalSize', 'distanceMin',
	'direction_x', 'direction_y', 'direction_z',
	'distanceToExtremity1', 'distanceToExtremity2',
	'gravityCenter_x', 'gravityCenter_y', 'gravityCenter_z',
	'corticalDotProduct']

classes_map = {
	'sex' : ['m', 'f'],
	'lateralite' : ['R', 'L']
}

################################################################################
def sort_data_along_subject(I, filename, descr):
	reader = ReaderCsv()
	X = reader.read(filename)
	valid = X[:, 'valid'] == 1
	if valid.sum() < int(X.shape[0] * 0.9): return [None] * 4
	if X.shape[1] == 32:
		features = descr[0]
	else:	features = descr[1]
	if features == None: return [None] * 4
	X2 = X[:, features]
	if numpy.any(I[:, 'subject'] != X[:, 'subject']):
		print("error I != X")
		sys.exit(1)
	# bug datamind X2[valid != True] = plop ne marche pas
	# pb avec __setitem__ sur les dmarray
	X2[valid != True, :] = numpy.nan
	if None in X2: return [None] * 4
	sulcus = X[0, "label"]
	side = X[0, "side"]
	return X2, sulcus, side, features


def extract_all(descr, I, prefix):
	M = []
	header = []
	for side in [('Left', 'L'), ('Right', 'R')]:
		dirname = os.path.join(prefix + '_' + side[0], 'morpho')
		globs = glob.glob(os.path.join(dirname, "siMorpho*.dat"))
		for filename in globs:
			X, sulcus, side, features = sort_data_along_subject(I,
							filename, descr)
			if X is None: continue
			M.append(X)
			header += [sulcus + '_' + side + '_' + \
					d for d in features]
	M = numpy.hstack(M)
	D = DF(colnames=header, data=M)
	for cat in classes_map.keys():
		D2 = DF(colnames=[cat], data=[[s] for s in I[:, cat]])
		D = D.concatenate(D2, axis=1)
	return D

def extract_all_LR(descr, I, prefix):
	M = []
	header = []
	for side in [('Left', 'L')]:
		dirname = os.path.join(prefix + '_' + side[0], 'morpho')
		globs = glob.glob(os.path.join(dirname, "siMorpho*.dat"))
		for filenameL in globs:
			filenameR = re.sub('_left', '_right', filenameL)
			filenameR = re.sub('_Left', '_Right', filenameR)
			if not os.path.exists(filenameR): continue
			XL, sulcus, side, features = sort_data_along_subject(I,
							filenameL, descr)
			XR, sulcus, side, features = sort_data_along_subject(I,
							filenameR, descr)
			if None in [XL, XR]: continue
			M.append(numpy.vstack([XL, XR]))
			header += [sulcus + '_' + d for d in features]
	header.append("side") # Y
	Y = numpy.array([0] * 151 + [1] * 151) # L/R
	M.append(Y[None].T)
	M = numpy.hstack(M)
	return DF(colnames=header, data=M)
	
			
################################################################################
def parseOpts(argv):
	description = 'Concatenate files'
	parser = OptionParser(description)
	parser.add_option('-p', '--prefix', dest='prefix',
		metavar = 'STR', action='store', default = 'icbm',
		help='prefix of studied labeling')
	parser.add_option('-o', '--output', dest='output',
		metavar = 'FILE', action='store', default = None,
		help='output matrix data to classify')
	parser.add_option('-m', '--mode', dest='mode',
		metavar = 'FILE', action='store', default='LR',
		type='choice', choices=('LR', 'cat'),
		help="'LR', 'cat'")
	parser.add_option('--seg-descr', dest='segdescr',
		metavar = 'STR', action='store', type='choice',
		choices=('fd3', 'fd3_red1'), default = None,
		help="'fd3', 'fd3_red1' (hullJunctionsSize, " + \
		"geodesicDepthMax, surface) ")
	parser.add_option('--rel-descr', dest='reldescr',
		metavar = 'STR', action='store', type='choice',
		choices=('fd2',), default = None,
		help="'fd2'")

	return parser, parser.parse_args(argv)



################################################################################
def main():
	# options
	parser, (options, args) = parseOpts(sys.argv)
	descr = [None, None]
	if options.segdescr == 'fd3': descr[0] = seg_fd3
	elif options.segdescr == 'fd3_red1': descr[0] = seg_fd3_red1
	if options.reldescr == 'fd2': descr[1] = rel_fd2

	# read
	infoname = 'info_icbm.dat'
	reader = ReaderCsv()
	I = reader.read(infoname)


	# compute
	if options.mode == 'cat':
		D = extract_all(descr, I, options.prefix)
	elif options.mode == 'LR':
		D = extract_all_LR(descr, I, options.prefix)
	WriterCsv().write(D, options.output)


if __name__ == '__main__': main()
