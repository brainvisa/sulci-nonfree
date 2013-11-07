#!/usr/bin/env python

import pickle, numpy, scipy.stats, scipy.special
try:
  import fff.GMM
except:
  # print 'warning, fff is not here or does not work. GMM will not be usable'
  pass
from distribution import Distribution, distribution_map


################################################################################
# GMM
################################################################################
class Gmm(Distribution):
	'''
    Gaussian Mixture Model

    type : full, diagonal, spherical
	'''
	def __init__(self, type='full'):
		Distribution.__init__(self)
		self._name = 'gmm'
		self._gmm = None
		if type == 'full': self._prec_type = 0
		elif type == 'diagonal': self._prec_type = 1
		elif type == 'spherical': self._prec_type = 2

	def fit(self, db):
		X = db.getX()
		kmax = 20
		# hack for small size databases
		if len(X) / kmax <= 2: kmax = len(X) / 5
		kvals = range(2, kmax)
		gmm = fff.GMM.GMM(dim=X.shape[1], prec_type=self._prec_type)
		labels, logli, bic = gmm.optimize_with_BIC(X, kvals, \
						ninit=5, verbose=0)
		self._gmm = gmm

	def likelihood(self, x):
		x = numpy.asmatrix(x)
		labels, loglis, bic = self._gmm.partition(x)
		logli = loglis.mean()
		logli = numpy.longdouble(logli)
		return logli, numpy.exp(logli)

	def write(self, filename):
		fd = open(filename, 'w')
		obj = self._gmm.k, self._gmm.dim, \
			self._gmm.prec_type, self._gmm.centers, \
			self._gmm.precision, self._gmm.weights
		pickle.dump(obj, fd)
		fd.close()

	def read(self, filename):
		fd = open(filename, 'r')
		obj = pickle.load(fd)
		fd.close()
		try:
			k, dim, prec_type, centers, precision, weights = obj
			self._gmm = fff.GMM.GMM(k, dim, prec_type, centers, \
							precision, weights)
			self._prec_type = prec_type
		except TypeError: raise IOError


class FixedBGmm(Gmm):
	'''
    Bayesian Mixture Model

    type : diagonal
    kmax : number of gaussians
	'''
	def __init__(self, type='diagonal', kmax=2):
		Gmm.__init__(self, type)
		self._name = 'fixed_bgmm'
		self._kmax = kmax

	def fit(self, db):
		X = db.getX()
		self._gmm = fff.GMM.BGMM(self._kmax, X.shape[1], \
						self._prec_type)
		self._gmm.set_empirical_priors(X)
		self._gmm.VB_estimate(X)

	def likelihood(self, x):
		import fff.clustering as FC
		x = numpy.asmatrix(x)
		g = self._gmm
		logli = FC.bayesian_gmm_sampling(g.prior_centers,
			g.prior_precision, g.prior_mean_scale, g.prior_weights,
			g.prior_dof, g.centers, g.precisions, g.mean_scale,
			g.weights, g.dof, x)
		return logli, numpy.exp(logli)

	def write(self, filename):
		fd = open(filename, 'w')
		obj = self._gmm.k, self._gmm.dim, \
			self._gmm.prec_type, self._gmm.centers, \
			self._gmm.precisions, self._gmm.mean_scale, \
			self._gmm.weights, self._gmm.dof, self._gmm.prior_dof, \
			self._gmm.prior_weights, self._gmm.prior_mean_scale, \
			self._gmm.prior_centers, self._gmm.prior_precision
		pickle.dump(obj, fd)
		fd.close()

	def read(self, filename):
		fd = open(filename, 'r')
		obj = pickle.load(fd)
		fd.close()
		try:
			k, dim, prec_type, centers, precision, mean_scale, \
			weights, dof, prior_dof, prior_weights, \
			prior_mean_scale, prior_centers, prior_precision = obj
			self._gmm = fff.GMM.BGMM(k, dim, prec_type, centers, \
							precision, weights)
			self._gmm.dof = dof
			self._prec_type = prec_type
			self._gmm.set_priors(prior_centers, prior_weights,
				prior_precision, prior_dof, prior_mean_scale)
			self._gmm.precisions = precision
			self._gmm.mean_scale = mean_scale
		except TypeError: raise IOError

################################################################################
distribution_map.update({\
	'gmm' : Gmm,
	'fixed_bgmm' : FixedBGmm,
})
