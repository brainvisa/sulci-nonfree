#!/usr/bin/env python2

# standard
from __future__ import print_function
import os, sys, pickle
import numpy, scipy.stats
from optparse import OptionParser

# matplotlib
import matplotlib
matplotlib.use('QtAgg')
import pylab

# trick to use fff2 instead of fff in datamind
import fff, fff2
sys.modules['fff'] = fff2

# datamind / soma
from datamind.io import ReaderCsv
from datamind.io import DF 
import datamind.ml.classif as CLF
import datamind.ml.resampling as resample
import datamind.ml.func as F


################################################################################
classes_map = {
	'sex' : ['m', 'f'],
	'lateralite' : ['R', 'L']
}

################################################################################
def nan2mean(X, Xm=None):
	for i in range(X.shape[1]):
		z = numpy.isnan(X[:, i])
		if Xm is None:
			X[z, i] = X[z != True, i].mean()
		else:	X[z, i] = Xm[i]
	return X

def classif(X, Y, n, loo_indices, mode):
	if mode == 'svm_linear':
		clf = CLF.SVM_CSVC(CLF.SVM_LINEAR)
	elif mode == 'svm_rbf':
		clf = CLF.SVM_CSVC(CLF.SVM_RBF)
		clf.setParams({'C': 1., 'gamma' : 1./n})
	cv = resample.LOO(Y)
	res = []
	Ye2 = []
	Yt2 = []
	for i, (inda, inde) in enumerate(cv):
		# cv
		Xa, Xe, Ya, Ye = resample.splitTrainTest(inda, inde, X, Y)
		indices = loo_indices[i][:n]
		Xa = Xa[:, indices]
		G0 = nan2mean(Xa[Ya == 0, :])
		G1 = nan2mean(Xa[Ya == 1, :])
		Xa = numpy.vstack((G0, G1))
		# center/reduce
		Xm = Xa.mean(axis=0)
		Xac = Xa - Xm
		std = Xac.std(axis=0)
		Xa = Xac / std
		Xe = nan2mean(Xe[:, indices], Xm)
		Xe = (Xe - Xm) / std
		# fit /predict
		clf.fit(Xa, Ya)
		Yt = clf.predict(Xe)
		Yt2.append(Yt)
		Ye2.append(Ye)
	rate = F.zeroOne_acc(Yt2, Ye2)
	return rate

def pvalues(X, Y):
	G0 = nan2mean(X[Y == 0, :])
	G1 = nan2mean(X[Y == 1, :])

	dim = X.shape[1]
	t, pvals = scipy.stats.ttest_ind(G0, G1)
	pvals[(numpy.std(G0, axis=0) < 10e-20) + \
		(numpy.std(G1, axis=0) <  10e-20)] = 1.
	return pvals

def loo_pvalues(X, Y):
	loo_pvals = []
	loo_indices = []
	cv = resample.LOO(Y)
	for inda, inde in cv:
		# cv / groups
		Xa, Xe, Ya, Ye = resample.splitTrainTest(inda, inde, X, Y)
		pvals = pvalues(Xa, Ya)
		loo_indices.append(numpy.argsort(pvals))
		loo_pvals.append(pvals)
	return numpy.array(loo_pvals), numpy.array(loo_indices)

################################################################################
def parseOpts(argv):
	description = 'Display p-vals'
	parser = OptionParser(description)
	parser.add_option('-i', '--input', dest='input',
		metavar = 'FILE', action='store', default = None,
		help='input data to classify')
	parser.add_option('-o', '--output', dest='output',
		metavar = 'FILE', action='store', default = None,
		help='ranges/LOO test classif rates')
	parser.add_option('-p', '--pvals', dest='output_pvals',
		metavar = 'FILE', action='store', default = None,
		help='output sorted pvals')
	parser.add_option('-m', '--mode', dest='mode',
		metavar = 'FILE', action='store', default = 'svm_linear',
		type='choice', choices=('svm_linear', 'svm_rbf'),
		help="'svm_linear', 'svm_rbf'")
	parser.add_option('-c', '--classname', dest='classname',
		metavar = 'FILE', action='store', default = 'side',
		type='choice', choices=('side', 'sex', 'lateralite'),
		help="'side', 'sex', 'lateralite'")
	parser.add_option('-d', '--dump', dest='dump',
		metavar = 'FILE', action='store', default=None,
		help="dump loo pvals computing (save or reload)")

	return parser, parser.parse_args(argv)


################################################################################
def main():
	parser, (options, args) = parseOpts(sys.argv)
	reader = ReaderCsv()

	print("read...")
	M = reader.read(options.input)
	indices = [ind for ind, name in enumerate(M.colnames()) \
				if name != options.classname]
	X = numpy.asarray(M[:, indices])
	Y = numpy.asarray(M[:, options.classname])
	if options.classname in classes_map.keys():
		# select subject with authorized classes
		sel = False
		for c in classes_map[options.classname]: # logical OR
			sel += (M[:, options.classname] == DF.code(c))
		X = X[sel, :] #FIXME : hack / bug datamind X[sel] ne marche pas
		Y = Y[sel]
		Y = (Y == Y[0]) + 0.

	print("loo pvals...")
	if options.dump is None or not os.path.exists(options.dump):
		loo_pvals, loo_indices = loo_pvalues(X, Y)
		if options.dump:
			fd = open(options.dump, 'w')
			pickle.dump((loo_pvals, loo_indices), fd)
			fd.close()
	else:
		fd = open(options.dump)
		loo_pvals, loo_indices = pickle.load(fd)

	nrange = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15, 17, 20, 25,
			30, 40, 50, 75, 100, 125, 150, 175, 200]
	rates = []
	print("classif...")
	for n in nrange:
		print("n = ", n)
		rates.append(classif(X, Y, n, loo_indices, options.mode))

	fd = open(options.output, 'w')
	fd.write("nrange = " + str(nrange) + '\n')
	fd.write("rates = " + str(rates) + '\n')
	fd.close()

	print("global pvals...")
	dims = M.colnames()
	pvals = pvalues(X, Y)
	fd = open(options.output_pvals, 'w')
	indices = numpy.argsort(pvals)
	fd.write("feature\tpval\n")
	for i in indices:
		fd.write(dims[i] + "\t" + str(pvals[i]) + '\n')
	fd.close()


if __name__ == '__main__': main()
