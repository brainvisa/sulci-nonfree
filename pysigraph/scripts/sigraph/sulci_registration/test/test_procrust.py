#!/usr/bin/env python

import os, sys, vtk
import numpy
import sulci.registration as S
import sulci.registration.vtk_helpers as V
from sulci.models import distribution

def update_cloud(procrust, (Y, pcX, tcl, vec, plotter)):
	X = procrust._X
	t = procrust._t
	R = procrust._R
	w = procrust._w
	X2 = (R * X + t)
	pcX.set_X(X2)
	tcl.set_XY(X2, Y)
	
	vec.set((R * X.mean(axis=1) + t), w * 10/ numpy.linalg.norm(w))
	plotter.render()


def test1():
	import pylab
	n = 1000
	Y = numpy.random.multivariate_normal([0, 0, 0],
				numpy.diag([80, 5, 1]), n).T
	Y = numpy.asmatrix(Y)
	Y2 = Y + numpy.random.normal(0, 0.2, size=(3, n))
	#J = S.antisymetric_matrix_from_vector([0, 0, 0])
	J = S.antisymetric_matrix_from_vector([0, 0, numpy.pi/8.])
	R = S.rotation_from_antisymetric_matrix(J)
	t = numpy.array([[40, 20, 10]]).T
	#t = 0
	X = R.T * (Y2 - t)
	print "-- Real transformation --"
	print "t = ", t
	print "R = ", R
	#A = numpy.asmatrix(numpy.identity(3))
	P = S.rotation_from_antisymetric_matrix(\
		S.antisymetric_matrix_from_vector([0, 0, 0.5]))
	A = P * numpy.diag([2, 1., 1.]) * P.T


	pcX = V.PointsCloud(X)
	pcX.set_color([1, 0, 0])
	pcX.set_size(3)
	pcY = V.PointsCloud(Y)
	pcY.set_color([0, 0, 1])
	pcY.set_size(3)
	tcl = V.TwoOrientedCloudsLinks(X, Y)
	tcl.set_color([0.9, 0.9, 0.9])
	vec = V.Vector([0, 0, 0], [10, 10, 10])
	vec.set_color([0, 1, 0])
	vec.set_size(3)
	plotter = V.VtkPlot(600, 600)
	plotter.set_bgcolor([0.7, 0.8, 0.9])
	plotter.plot([tcl, pcX, pcY, vec])
	plotter.render()

	p = S.ProcrustMetric(X, Y, A)
	R, t = p.optimize_riemannian(user_func=update_cloud,
			user_data=(Y, pcX, tcl, vec, plotter))
	print "-- Estimated transformation --"
	print "t = ", t
	print "R = ", R
	AX2 = A * (R * X + t)
	AY = A * Y

	#pylab.plot(X[0], X[1], 'ro')
	#pylab.plot(Y[0], Y[1], 'bo')
	#pylab.figure()
	#pylab.plot(AX2[0], AX2[1], 'ro')
	#pylab.plot(AY[0], AY[1], 'bo')
	#for i in range(n):
	#	pylab.plot([AX2[0, i], AY[0, i]], [AX2[1, i], AY[1, i]], 'g-')
	#pylab.figure()
	#pylab.plot(AX2[1], AX2[2], 'ro')
	#pylab.plot(AY[1], AY[2], 'bo')
	#for i in range(n):
	#	pylab.plot([AX2[1, i], AY[1, i]], [AX2[2, i], AY[2, i]], 'g-')
	#pylab.show()
	plotter.show()



def update_cloud2(procrust, (Y, pcX, fgl, vec, plotter)):
	X = procrust._X
	t = procrust._t
	R = procrust._R
	w = procrust._w
	centers = procrust._centers
	weights = procrust._weights
	X2 = (R * X + t)
	pcX.set_X(X2)
	fgl.set(X2, centers, weights)
	
	vec.set((R * X.mean(axis=1) + t), w * 10/ numpy.linalg.norm(w))
	plotter.render()


def test2():
	import pylab
	size = 3
	centers = [[0., 0., 0.], [10., -2., -2.], [0., 3., 0.]]
	centers = [numpy.matrix(c).T for c in centers]
	P1 = S.rotation_from_antisymetric_matrix(\
		S.antisymetric_matrix_from_vector([0, 0, 1.]))
	P2 = S.rotation_from_antisymetric_matrix(\
		S.antisymetric_matrix_from_vector([0, 0, 2.5]))
	cov1 = numpy.diag([0.1, 0.1, 0.1])
	cov2 = P1 * numpy.diag([2, 0.1, 0.1]) * P1.T
	cov3 = P2 * numpy.diag([2, 0.1, 0.1]) * P2.T
	covariances = [cov1, cov2, cov3]
	covariances = [numpy.matrix(cov) for cov in covariances]
	metrics = [cov.I for cov in covariances]

	n = 1000
	#J = S.antisymetric_matrix_from_vector([0, 0, 0])
	J = S.antisymetric_matrix_from_vector([0, 0., numpy.pi/10.])
	R = S.rotation_from_antisymetric_matrix(J)
	#t = numpy.array([[20, 0, 0]]).T
	t = numpy.array([[0.], [0.], [0.]])
	print "-- Real transformation --"
	print "t = ", t
	print "R = ", R
	X = R.T * (V.gen_data_gmm(centers, covariances, n) - t)
	weights = []
	sum = 0.
	for i in range(size):
		w = []
		y = centers[i]
		A = metrics[i]
		for j in range(X.shape[1]):
			v = (X[:, j] - y)
			w.append(numpy.exp(-0.5*numpy.trace(v.T * A * v)))
		sum += numpy.array(w)
		#w = [0.1] * n * i + [5.] * n + [0.1] * n * (size - i - 1)
		weights.append(w)
	weights = numpy.asmatrix((numpy.vstack(weights) / sum))

	Y = V.gen_data_gmm(centers, covariances, 1000)

	pcX = V.PointsCloud(X)
	pcX.set_color([1, 0, 0])
	pcX.set_size(3)
	pcY = V.PointsCloud(Y)
	pcY.set_color([0, 0, 1])
	pcY.set_size(3)
	fgl = V.FuzzyGaussianLinks(X, centers, weights)
	fgl.set_color([0.9, 0.9, 0.9])
	vec = V.Vector([0, 0, 0], [10, 10, 10])
	vec.set_color([0, 1, 0])
	vec.set_size(3)
	plotter = V.VtkPlot(600, 600)
	plotter.set_bgcolor([0.7, 0.8, 0.9])
	plotter.plot([fgl, pcX, pcY, vec])
	plotter.render()

	gmm = distribution.GaussianMixtureModel()
	gmm.set_means_covs(centers, covariances)
	pmf = S.ProcrustMetricField(X, weights, gmm)
	#R, t = pmf.optimize(user_func=update_cloud,
	R, t = pmf.optimize_riemannian(user_func=update_cloud2,
                        user_data=(Y, pcX, fgl, vec, plotter))
	print "-- Estimated transformation --"
	print "t = ", t
	print "R = ", R

	#X2 = (R * X + t)
	#pylab.plot(Y[0], Y[1], 'ro')
	#pylab.plot(X[0], X[1], 'bo')
	#pylab.figure()
	#pylab.plot(Y[0], Y[1], 'ro')
	#pylab.plot(X2[0], X2[1], 'bo')
	#pylab.figure()
	#pylab.plot(Y[1], Y[2], 'ro')
	#pylab.plot(X2[1], X2[2], 'bo')
	#pylab.show()
	plotter.show()


def test3():
	import pylab
	size = 3
	centers = [[0., 0., 0.], [10., -2., -2.], [0., 3., 0.]]
	centers = [numpy.matrix(c).T for c in centers]
	P1 = S.rotation_from_antisymetric_matrix(\
		S.antisymetric_matrix_from_vector([0, 0, 1.]))
	P2 = S.rotation_from_antisymetric_matrix(\
		S.antisymetric_matrix_from_vector([0, 0, 2.5]))
	cov1 = numpy.diag([0.1, 0.1, 0.1])
	cov2 = P1 * numpy.diag([2, 0.1, 0.1]) * P1.T
	cov3 = P2 * numpy.diag([2, 0.1, 0.1]) * P2.T
	covariances = [cov1, cov2, cov3]
	covariances = [numpy.matrix(cov) for cov in covariances]
	metrics = [cov.I for cov in covariances]
	priors = [0.33333, 0.33333, 0.33333]

	n = 200
	J = S.antisymetric_matrix_from_vector([0, 0, 0])
	#J = S.antisymetric_matrix_from_vector([0, 0., numpy.pi/5.])
	R = S.rotation_from_antisymetric_matrix(J)
	t = numpy.array([[10, 0, 0]]).T
	#t = 0.
	print "-- Real transformation --"
	print "t = ", t
	print "R = ", R
	#X = R.T * (V.gen_data_gmm(centers, covariances, n) - t)
	X = R * numpy.asmatrix(numpy.hstack(centers)) - t #FIXME
	n = 1 #FIXME
	Y = V.gen_data_gmm(centers, covariances, 1000)

	pcX = V.PointsCloud(X)
	pcX.set_color([1, 0, 0])
	pcX.set_size(3)
	pcY = V.PointsCloud(Y)
	pcY.set_color([0, 0, 1])
	pcY.set_size(3)
	weights = [[1./size] * n * size for i in range(size)]
	weights = numpy.asmatrix((numpy.vstack(weights)))
	fgl = V.FuzzyGaussianLinks(X, centers, weights)
	fgl.set_color([0.9, 0.9, 0.9])
	vec = V.Vector([0, 0, 0], [10, 10, 10])
	vec.set_color([0, 1, 0])
	vec.set_size(3)
	plotter = V.VtkPlot(600, 600)
	plotter.set_bgcolor([0.7, 0.8, 0.9])
	plotter.plot([fgl, pcX, pcY, vec])
	plotter.render()

	gmm = distribution.GaussianMixtureModel()
	gmm.set_means_covs(centers, covariances)
	gmm.set_priors(numpy.array(priors))
	pmf = S.ProcrustMetricField(X, weights, gmm, verbose=2)

	#pmf = S.MixtureGlobalRegistration(X, gmm)
	#R, t = pmf.optimize_translation_powell(eps=0.0001,
	R, t = pmf.optimize_riemannian(eps=0.0001,
	#trans, weights, loglis, lis = pmf.optimize(eps=10.,
		user_func=update_cloud2, user_data=(Y, pcX, fgl, vec, plotter))
	#R = trans._R
	#t = trans._t
	print "-- Estimated transformation --"
	print "t = ", t
	print "R = ", R

	plotter.show()


def main():
	#test1()
	#test2()
	test3()


if __name__ == '__main__': main()
