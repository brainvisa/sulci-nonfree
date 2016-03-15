#!/usr/bin/env python2
import numpy, os, pylab, sys, svm
import datamind
import sigraph.datamind_backend

from  datamind.ml import reader
from datamind.ml import validation
from datamind.ml.classifier import optimizers
from datamind.ml.classifier import ofunc
from datamind.ml import plugins
from datamind.ml import dimreduction
from datamind.ml import database

plugins.plugin_manager.load_plugin('Sigraph')
plugins.plugin_manager.load_plugin('libsvm')


# read data
def read():
	#modelpath = '/home/Panabase/data_sulci_ccrt/base2000/models/R_fd2_svr_cliqueMemorizer_28_08_06/model'
	modelpath = '/home/revilyo/cea_data_backup/base2000/models/R_fd5_svm_cliqueMemorizer_28_07_06/model'
	filename = os.path.join(modelpath, 'adap', 'S.C._right_svm1.minf')
	db = reader.Reader('Sigraph').read(filename)
	train, test = db.filter('none', True)
	return train, test



# take good Y value and put float64 dtype (libsvm need it)
def convert_db_to_svm(db):
	Y = db.getY()["outp"].astype('f8')
	return database.DbNumpy(db.getX(), Y)



# scaling
def scale(train, test):
	import datamind.ml.scaling
	scale = datamind.ml.scaling.NormalizedXScale()
	scaled_train = scale.fit(train)
	scaled_test = scale.predict(test)
	return scaled_train, scaled_test



def compute_weights(db):
	classes = db.getY()["outp"] >= 0.
	w0 = 1. / (classes == 0).sum()
	w1 = 1. / (classes == 1).sum()
	weights = (classes * w1 + (1 - classes) * w0).ravel()
	return weights



def print_dimreduction_info(dr):
	u, d, v = dr.get_udv()
	inf = d / d.sum()
	q = 0
	for i in inf:
		q += i
		print q * 100, "/ 100"



# dimension reduction of data
def test_dim_reduction(dr, train, test, train_weights, test_weights):
	res = []
	for ndim in range(2, 20):
		train_reduced = dr.get_reduced_train(train, ndim)
		test_reduced = dr.reduce(test, ndim)
		res.append(test_clf(train_reduced, test_reduced,
				train_weights, test_weights))
		print res
	return res



def test_clf(train, test, train_weights, test_weights):
	# parameters
	nbfolds = 10
	steps = 10
	dim = 23.
	d = 1. / dim
	grange = d * numpy.logspace(-2, 2, steps)
	crange = numpy.logspace(-1, 5, steps)


	# clf
	clf = datamind.ml.classifier.ESvrLibSvm(svm.RBF)
	val = validation.NativeCV(train, nbfolds)
	of = ofunc.ModelOFunc(clf, val, 'wmse', {'weights' : train_weights})
	#of = ofunc.ModelOFunc(clf, val, 'mse')
	grid = optimizers.Grid({'C' : crange, 'gamma' : grange},
				strategy = 'min')
	res = grid.optimize(of)

	# fit data with best learned parameters
	clf.setParams(res['best_params'])
	clf.fit(train)
	print "best params = ", res['best_params']

	# predict
	res = clf.predict(test)
	res.compute_wmse(test_weights)
	res.compute_mse()
	print res
	return res



def main():
	train, test = read()
	train2 = convert_db_to_svm(train)
	test2 = convert_db_to_svm(test)
	scaled_train, scaled_test = scale(train2, test2)
	train_weights = compute_weights(train)
	test_weights = compute_weights(test)
	dr = dimreduction.SvdNumpy(scaled_train)
	print_dimreduction_info(dr)
	sys.exit(1)
	res = test_dim_reduction(dr, scaled_train, scaled_test,
				train_weights, test_weights)
	print "=================="
	print res


def test(train):
	X = train.getX()
	Y = train.getY()

	# test data on a 2D grid
	xstep = ystep = 100
	xmin, ymin = min(X[:, 0]), min(X[:, 1])
	xmax, ymax = max(X[:, 0]), max(X[:, 1])
	dx, dy = (xmax - xmin) * 0.3, (ymax - ymin) * 0.3
	xmin, ymin = xmin - dx, ymin - dy
	xmax, ymax = xmax + dx, ymax + dy


	test = numpy.array([(x, y) for x in numpy.linspace(xmin, xmax, xstep)
			for y in numpy.linspace(ymin, ymax, ystep)])
	test = datamind.ml.database.DbNumpy(test)
	values = clf.predict(test).predict_values
	Z = numpy.array(values).reshape(xstep, ystep).transpose()

	# show data
	ex = (xmin, xmax, ymin, ymax)

	pylab.figure()
	pylab.imshow(Z, aspect='auto', origin='lower', interpolation = 'nearest', 
		extent = ex)
	pylab.colorbar()
	#cset = pylab.contour(Z, [1.5], origin='lower', linewidths=2, extent = ex)

	for y, x in zip(Y, X):
		if y == 1:
			pylab.plot([x[0]], [x[1]], 'bo')
		else:   pylab.plot([x[0]], [x[1]], 'ro')

	# 3D
	import matplotlib.axes3d as axes3d
	fig = pylab.figure()
	ax3d = axes3d.Axes3DI(fig)
	plt = fig.axes.append(ax3d)

	ax3d.scatter3d(X[:, 0], X[:, 1], train_reduced2.getY().ravel())


	# show
	pylab.show()

if __name__ == '__main__': main()
