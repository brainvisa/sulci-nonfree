#!/usr/bin/env python

import pickle, numpy, scipy.stats, scipy.special

class Distribution(object):
	def __init__(self):
		'''
    Each user-defined distribution must defined a name and be registred on
    the factory at the end of this file
		'''
		self._name = 'undefine'

	def update(self):
		'''update internal parameters'''
		pass

	def write(self, filename):
		'''dump distribution on disk'''
		fd = open(filename, 'w')
		obj = self.toTuple()
		pickle.dump(obj, fd)
		fd.close()

	def read(self, filename):
		'''read distribution from disk'''
		fd = open(filename, 'r')
		obj = pickle.load(fd)
		try:	self.fromTuple(obj)
		except TypeError: raise IOError
		fd.close()
		self.update()

	def likelihoods(self, X):
		'''
    Likelihoods of each object contained in X.
    Example : X is a matrix containing vectors.

    By default Likelihood of one single object is called.
		'''
		loglis, lis = [], []
		for x in X:
			logli, li = self.likelihood(x)
			loglis.append(logli)
			lis.append(li)
		return numpy.array(loglis), numpy.array(lis)

	def likelihoods_groups(self, X, groups, L=None):
		maxid = numpy.max(groups)
		loglikelihoods, likelihoods = [], []
		for id in range(maxid + 1):
			Xs = X[groups == id]
			if len(Xs) == 0:
				loglikelihood, likelihood = 0., 1. # Cte
			elif L is not None and not L[id]:
				loglikelihood, likelihood = -1000., 0.
			else:	loglikelihood, likelihood = \
						self.prodlikelihoods(Xs)
			loglikelihoods.append(loglikelihood)
			likelihoods.append(likelihood)
		loglikelihoods = numpy.array(loglikelihoods)
		likelihoods = numpy.array(likelihoods)
		return loglikelihoods, likelihoods

	def weighted_prodlikelihoods_groups(self, X, weights, groups,th=0.9999):
		maxid = numpy.max(groups)
		loglikelihoods, likelihoods = [], []
		if th < 1.:
			wsum = weights.sum()
			if wsum:
				s = numpy.sort(weights)[::-1]
				wth = s[s.cumsum() > (th * wsum)][0]
				if wth == s[0]:
					s = numpy.unique1d(s)[::-1]
					try: wth = s[1]
					except KeyError: pass
					except IndexError: wth = 0.
			else:	wth = 0.
		else:	wth = 0.
		weights2 = []
		for id in range(maxid + 1):
			w = weights[id]
			if (w == 0.) or (w < wth): continue
			weights2.append(w)
			Xs = X[groups == id]
			if len(Xs) == 0:
				loglikelihood, likelihood = 0., 1. # Cte
			else:	loglikelihood, likelihood = \
						self.prodlikelihoods(Xs)
			loglikelihoods.append(loglikelihood)
			likelihoods.append(likelihood)
		weights2 = numpy.array(weights2)
		loglikelihoods = numpy.array(loglikelihoods)
		likelihoods = numpy.array(likelihoods)
		loglikelihood = numpy.dot(loglikelihoods, weights2.T)
		likelihood = numpy.dot(likelihoods, weights2.T)
		return loglikelihood, likelihood

	def prodlikelihoods(self, X):
		logli, li = self.likelihood()
		return numpy.sum(logli), numpy.prod(li)

	def name(self): return self._name


################################################################################
# Misc
################################################################################
class Dummy(Distribution):
	def __init__(self, val):
		Distribution.__init__(self)
		self._name = 'dummy'
		self._val = val

	def write(self, filename): pass

	def read(self, filename): pass

	def fit(self, X): pass

	def likelihood(self): return numpy.log(self._val), self._val


################################################################################
# Frequencies
################################################################################
class Frequency(Distribution):
	'''
    P(X) = prod P(X_i) = prod freq(X_i)
    with freq(X_i) = #(X_i) / (sum #(X_i))
	'''
	def __init__(self, freq=None):
		Distribution.__init__(self)
		self._name = 'frequency'
		self._freq = freq

	def set_frequencies(self, freq): self._freq = numpy.asmatrix(self._freq)

	def frequencies(self): return self._freq

	def fit(self, X):
		'''
    X : each row represents one data, each column one label, each value is
        a data of interest measured on a given data over a given label.
		'''
		self._freq = numpy.asmatrix(X.sum(axis=0) / X.sum())

	def likelihood(self, x):
		'''x : array of ordred measures over labels (one per label)'''
		logli = numpy.sum(numpy.log(self._freq) * x)
		logli = numpy.float96(logli)
		return logli, numpy.exp(logli)

	def toTuple(self):
		return (self._name, self._freq)

	def fromTuple(self, tuple):
		self._name, self._freq = tuple

class UniformFrequency(Frequency):
	def __init__(self, dim=None):
		self._name = 'uniform_frequency'
		if dim: self._freq = numpy.asmatrix(numpy.ones(dim))

	def set_frequencies(self, freq): pass

	def fit(self, X):
		self._freq = numpy.asmatrix(numpy.ones(X.shape[1]))


################################################################################
# Gaussians : full, diagonal, spherical, fake (bad-formed uniform)
################################################################################
class Gaussian(Distribution):
	'''
    Multivariate Gaussian with full covariance matrix.
	'''
	def __init__(self):
		Distribution.__init__(self)
		self._name = "full_gaussian"
		self._mean = None
		self._cov = None
		self._norm = None
		self._metric = None

	def _compute_norm(self):
		ndim = len(self._mean)
		det = numpy.linalg.det(self._cov)
		self._norm = numpy.sqrt(numpy.fabs(det)* (2 * numpy.pi) ** ndim)
		self._metric = self._cov.I
		self._det = det
		#FIXME : compute covI in fit and write/read it instead of cov

	def update(self):
		self._compute_norm()

	def read(self, filename):
		Distribution.read(self, filename)
		self._compute_norm()

	def type(self):
		return self._name

	def normalization(self):
		return self._norm

	def lognormalization(self):
		ndim = len(self._mean)
		pi2 = numpy.pi * 2
		return 0.5 * (numpy.log(numpy.fabs(self._det)) + \
				ndim * numpy.log(pi2))

	def covariance(self): return self._cov

	def det(self): return self._det

	def metric(self): return self._metric

	def mean(self): return self._mean

	def set_mean(self, mean):
		self._mean = numpy.asmatrix(mean)
		shape = self._mean.shape
		if shape[0] < shape[1]: self._mean = self._mean.T

	def set_cov(self, cov): self._cov = numpy.asmatrix(cov)

	def setMeanCov(self, mean=None, cov=None):
		'''set mean/cov (if not None) and update internal parameters'''
		if mean != None: self.set_mean(mean)
		if cov != None: self.set_cov(cov)
		self.update()

	def toTuple(self):
		return self._name, self._mean, self._cov

	def fromTuple(self, tuple):
		self._name, self._mean, self._cov = tuple

	def fit(self, db, weights=None):
		try:	X = db.getX()
		except AttributeError:
			X = db
		n = X.shape[0]
		X = numpy.asmatrix(X)
		Xm = X.mean(axis=0)
		Xc = X - Xm
		if weights:
			Xcw = numpy.multiply(weights, weights)
		else:	Xcw = Xc
		cov = Xc.T * Xcw / (n - 1)
		self._mean = Xm
		self._cov = cov
		self.update()

	def likelihood(self, x):
		x = numpy.asmatrix(x) - self._mean
		d = (x * self._metric * x.T)[0, 0]
		logli = -0.5 * d - numpy.log(self._norm)
		logli = numpy.float96(logli)
		return logli, numpy.exp(logli)

# works but too much memory required
#	def likelihoods(self, X):
#		X = numpy.asmatrix(X) - self._mean
#		d = numpy.diag(X * self._metric * X.T)
#		logli = -0.5 * d - numpy.log(self._norm)
#		logli = numpy.float96(logli)
#		return logli, numpy.exp(logli)

	def likelihood_expectation(self):
		'''
    Compute likelihood expectation :
                               _
                              /
        E_x[P(x|mean,cov)] = / P(x|mean,cov)^2 .dx
                           _/
                                          1
                           = ----------------------------
                              2^D * pi^(D/2) * |C|^(1/2)

        with D : number of dimensions of the gaussian.
		'''
		d = self._metric.shape[0]
		l2, lpi = numpy.log(2.), numpy.log(numpy.pi)
		logli = - d * (l2 + 0.5 * lpi) - 0.5 * numpy.log(self._det)
		return logli, numpy.exp(logli)

	def appf(self, pct):
		'''inverse of absolute cumulative function
                            _
                           /
    Return th :    with   / P(x|param) = pct
                        _/
                   x / N(x) > th
		'''
		eigval, eigvect = numpy.linalg.eig(self._cov)
		eigval = numpy.sqrt(eigval) # get standard deviation
		loglis, lis = 0., 1.
		for i, v in enumerate(eigval):
			x = scipy.stats.norm.ppf((pct + 1.) / 2., scale=v)
			li = scipy.stats.norm.pdf(x, scale=v)
			lis *= li
			loglis += numpy.log(li)
		return loglis, lis
		


class DiagonalGaussian(Gaussian):
	'''
    Multivariate diagonal matrix.
	'''
	def __init__(self):
		Gaussian.__init__(self)
		self._name = "diagonal_gaussian"

	def fit(self, db, weights=None):
		X = numpy.asmatrix(db.getX())
		Xm = X.mean(axis=0)
		Xc = X - Xm
		self._mean = Xm
		if weights:
			n = X.shape[0]
			std = numpy.multiply(weights, Xm ** 2) / (n + 1)
		else:	std = X.std(axis=0)
		self._cov = numpy.asmatrix(numpy.diag(std))


class SphericalGaussian(Gaussian):
	'''
    Multivariate Spherical Gaussian.
	'''
	def __init__(self):
		Gaussian.__init__(self)
		self._name = "spherical_gaussian"

	def fit(self, db, weights=None):
		X = numpy.asmatrix(db.getX())
		Xm = X.mean(axis=0)
		Xc = X - Xm
		self._mean = Xm
		if weights:
			n = X.shape[0]
			std = numpy.multiply(weights, Xm ** 2) / (n + 1)
		else:	std = X.std(axis=0)
		s = numpy.mean(numpy.asarray(std))
		self._cov = numpy.asmatrix(s * numpy.identity(X.shape[1]))

class SphericalGaussianFewDots(Gaussian):
	'''
    Multivariate Spherical Gaussian for small-sized databases.
	'''
	def __init__(self):
		Gaussian.__init__(self)
		self._name = "spherical_gaussian_few_dots"

	def fit(self, db, weights=None):
		'''multiply estimated standard deviation by 2'''
		X = numpy.asmatrix(db.getX())
		Xm = X.mean(axis=0)
		Xc = X - Xm
		self._mean = Xm
		if weights:
			n = X.shape[0]
			std = numpy.multiply(weights, Xm ** 2) / (n + 1)
		else:	std = X.std(axis=0)
		s = numpy.mean(numpy.asarray(std)) * 2
		self._cov = numpy.asmatrix(s * numpy.identity(X.shape[1]))
	

class FixedSphericalGaussian(Gaussian):
	'''
    Multivariate Spherical with fixed std
	'''
	def __init__(self):
		Gaussian.__init__(self)
		self._name = 'spherical_gaussian_fixed'
		self._std = None

	def _compute_norm(self):
		ndim = len(self._mean)
		det = self._std ** ndim
		self._norm = numpy.sqrt(det * (2 * numpy.pi) ** ndim)
		self._metric = numpy.identity(ndim) / self._std
		self._det = det

	def toTuple(self):
		return self._name, self._mean, self._std

	def fromTuple(self, tuple):
		self._name, self._mean, self._std = tuple

	def setStd(self, std):
		self._std = std
		if self._mean != None: self._compute_norm()

	def fit(self, db):
		X = numpy.asmatrix(db.getX())
		Xm = X.mean(axis=0)
		self._mean = Xm
		self._cov = numpy.asmatrix(self._std * \
				numpy.identity(X.shape[1]))

	def appf(self, pct):
		'''inverse of absolute cumulative function'''
		ndim = len(self._mean)
		x = scipy.stats.norm.ppf((pct + 1.) / 2., scale=self._std)
		li = scipy.stats.norm.pdf(x, scale=self._std) ** ndim
		logli = ndim * numpy.log(li)
		return logli, li



class FakeGaussian(Gaussian):
	'''
    Bad-formed Multivariate Gaussian shaped by an uniform distribution over
    all space.
	'''
	def __init__(self, val=0.):
		Gaussian.__init__(self)
		self._name = "fake_gaussian"
		self._val = val

	def _compute_norm(self): pass

	def toTuple(self):
		return self._name, self._val

	def fromTuple(self, tuple):
		self._name, self._val = tuple

	def write(self, filename): pass

	def read(self, filename): pass

	def fit(self, db): pass

	def likelihood(self, x):
		return numpy.log(self._val), self._val

	def appf(self, pct):
		'''this function can't be well defined.'''
		return -numpy.inf, 0.


class OrientedGaussian(Gaussian):
	'''
    Use an oriented vector (mean re-oriented vector of learned db) to
    compute likelihhod on the right referential.
    '''
	def __init__(self, gaussian=None):
		Gaussian.__init__(self)
		self._name = 'oriented_gaussian'
		self._gaussian = gaussian
		self._ref = None

	def _compute_norm(self):
		self._gaussian._compute_norm()

	def fit(self, db):
		X = numpy.asmatrix(db.getX())
		ref = X[0]
		s = numpy.sign(numpy.dot(X, ref))[None].T
		s[s == 0] = 1
		X *= s
		ref = X.mean(axis=0)
		s = numpy.sign(numpy.dot(X, ref))[None].T
		s[s == 0] = 1
		X *= s
		self._ref = ref
		self._gaussian.fit(db)

	def likelihood(self, x):
		x2 = x * numpy.sign(numpy.dot(x, self._ref))
		return self._gaussian.likelihood(x)

	def toTuple(self):
		return self._name, self._ref, self._gaussian.toTuple()

	def fromTuple(self, tuple):
		type, self._ref, info = tuple
		self._gaussian = distributionFactory(info[0])()
		self._gaussian.fromTuple(info)

	def appf(self, pct):
		return self._gaussian.appf(pct)



class BlocGaussian(Gaussian):
	'''
    Multivariate Gaussian defined by bloc. Each bloc has a full covariance
    submatrix whereas each bloc are statistically indepent between each others.

    list : list of dimensions clustered each others.
           ex : [ [1, 2, 3], [4, 7], [8, 9] ]
           Each pack will be learn with a full multivariate gaussian.
	'''
	def __init__(self, groups={}):
		Gaussian.__init__(self)
		self._name = 'bloc_gaussian'
		df = distributionFactory
		self._gaussians = {}
		for name, info in groups.items():
			type = info[0]
			g = df(type)()
			if type == 'spherical_gaussian_fixed':
				d, std = info[1:]
				g.setStd(std)
			else:	d = info[1]
			self._gaussians[name] = (d, g)

	def _compute_norm(self):
		for dims, g in self._gaussians.values(): g._compute_norm()

	def groups(self): return self._gaussians

	def fit(self, db):
		for dims, g in self._gaussians.values():
			db2 = db.share()
			# filter dimensions
			X = db.getX()[:, dims]
			# remove NaN
			X = X[numpy.isnan(X).sum(axis=1) == 0]
			db2.setX(X)
			g.fit(db2)
	
	def likelihood(self, x):
		loglis, lis = 0., 1.
		for dims, g in self._gaussians.values():
			xd = x[dims]
			if numpy.isnan(x).any():
				logli, li = g.likelihood_expectation()
			else:	logli, li = g.likelihood(xd)
			loglis += logli
			lis *= li
		return loglis, lis

	def likelihoods(self, x):
		loglis, lis = [], []
		for dims, g in self._gaussians.values():
			xd = x[dims]
			if numpy.isnan(x).any():
				logli, li = g.likelihood_expectation()
			else:	logli, li = g.likelihood(xd)
			loglis.append(logli)
			lis.append(li)
		return numpy.array(loglis), numpy.array(lis)

	def likelihood_expectation(self):
		loglis, lis = 0., 1.
		for dims, g in self._gaussians.values():
			logli, li = g.likelihood_expectation()
			loglis += logli
			lis *= li
		return loglis, lis

	def toTuple(self):
		objs = []
		for name, (dims, g) in self._gaussians.items():
			objs.append((name, dims, g.toTuple()))
		return (self._name, objs)

	def fromTuple(self, tuple):
		self._name, objs = tuple
		for name, dims, info in objs:
			type = info[0]
			g = distributionFactory(type)()
			g.fromTuple(info)
			self._gaussians[name] = (dims, g)

	def appf(self, pct):
		'''inverse of absolute cumulative function'''
		loglis, lis = 0., 1.
		for dims, g in self._gaussians.values():
			logli, li = g.appf(pct)
			loglis += logli
			lis *= li
		return loglis, lis


class RobustGaussian(Gaussian):
	def __init__(self, n=10000):
		Gaussian.__init__(self)
		self._name = "robust_gaussian"
		self._gaussian = Gaussian()
		self._n = n

	def _exp_map(self, m, x):
		m = numpy.asmatrix(m)
		x = numpy.asmatrix(x)
		G, u = numpy.linalg.eigh(m)
		g = u * numpy.diag(numpy.sqrt(G))
		gI = g.I
		Y = gI * x * gI.T
		S, v = numpy.linalg.eigh(Y) 
		gv = g * v
		return gv * numpy.diag(numpy.exp(S)) * gv.T

	def _log_map(self, m, x):
		m = numpy.asmatrix(m)
		x = numpy.asmatrix(x)
		G, u = numpy.linalg.eigh(m)
		g = u * numpy.diag(numpy.sqrt(G))
		gI = g.I
		Y = gI * x * gI.T
		S, v = numpy.linalg.eigh(Y) 
		gv = g * v
		return gv * numpy.diag(numpy.log(S)) * gv.T

	def _matrix_norm(self, m):
		m = numpy.asmatrix(m)
		numpy.sqrt(numpy.linalg.eigh(m.T * m)[0].max())
	
	def _robust_cov(self, covs, eps=10e-5):
		n = eps + 1.
		m = numpy.identity(covs[0].shape[0])
		while n > eps:
			lcov = numpy.zeros_like(covs[0])
			for cov in covs:
				lcov += self._log_map(m, cov)
			lcov /= len(covs)
			m = self._exp_map(m, lcov)
			n = self._matrix_norm(lcov)
		return m

	def fit(self, db, eps = 10e-4):
		X = db.getX()
		db2 = db.share()
		covs = []
		size = len(X)
		for i in range(self._n):
			r = numpy.random.randint(0, size, size)
			Xr = X[r]
			db2.setX(Xr)
			self._gaussian.fit(db2)
			cov = self._gaussian.covariance()
			covs.append(cov + eps * numpy.identity(X.shape[1]))
		robust_cov = self._robust_cov(covs)
		Xm = X.mean(axis=0)
		self._mean = Xm
		self._cov = robust_cov



################################################################################
# Gamma
################################################################################
class Gamma(Distribution):
	def __init__(self, shape=1, scale=1):
		Distribution.__init__(self)
		self._name = 'gamma'
		self._shape = shape
		self._scale = scale
		self._eps = 0.00001
		
	def _check(self, X):
		if X.min() >= 0: return 1
		print 'LGamma : negative or null values. Cannot proceed'
		return 0

	def _dichopsi_log(self, u, v, y, eps = 0.00001):
		if u > v: u, v = v, u
		t = (u + v) / 2.
		if numpy.absolute(u - v) < eps: return t
		else:
			if scipy.special.psi(t) - scipy.special.log(t) > y:
				return self._dichopsi_log(u, t, y, eps)
			else:
				return self._dichopsi_log(t, v, y, eps)

	def shape(self): return self._shape

	def scale(self): return self._scale

	def setShape(self, shape): self._shape = shape

	def setScale(self, scale): self._scale = scale

	def _compute_shape(self, X, weights):
		'''
    This function solves
    solve psi(c)-log(c)=y
    by dichotomy
		'''
		if weights is None:
			y = numpy.log(X).mean() - numpy.log(numpy.mean(X))
		else:
			wz = weights != 0
			X = X[wz]
			weights = weights[wz]
			y = numpy.dot(weights, numpy.log(X)) - \
				numpy.log(numpy.dot(weights, X))
		if y > 0:
			print "y", y
			raise ValueError, "y>0, the problem cannot be solved"
		u = 1.
		if y > scipy.special.psi(u) - scipy.special.log(u):
			while scipy.special.psi(u) - scipy.special.log(u) < y:
				u = 2 * u
			u = u / 2
		else:
			while scipy.special.psi(u) - scipy.special.log(u) > y:
				u = u / 2
		return self._dichopsi_log(u, 2 * u, y, self._eps)

	def _compute_scale(self, X, weights):
		if weights is None:
			return X.mean() / self._shape
		else:	return (numpy.dot(weights, X) / self._shape)

	def fit(self, db, weights=None):
		if weights is not None:
			weights = numpy.asarray(weights).ravel()
			weights = weights / weights.sum()
		try:
			X = db.getX()
			if not self._check(X): return
		except AttributeError:
			X = db
		X = X.ravel()
		self._shape = self._compute_shape(X, weights)
		self._scale = self._compute_scale(X, weights)
	

	def likelihood(self, x):
		logli = numpy.log(scipy.stats.gamma.pdf(x, self._shape, \
						0, self._scale))
		return logli, numpy.exp(logli)

	def likelihoods(self, X):
		return self.likelihood(X)

	def toTuple(self):
		return (self._name, self._shape, self._scale)

	def fromTuple(self, tuple):
		self._name, self._shape, self._scale = tuple


class ShiftedGamma(Gamma):
	def __init__(self, shape=1, scale=1, shift=0.):
		Gamma.__init__(self, shape, scale)
		self._shift = shift

	def fit(self, db):
		X = db.getX()
		X += shift
		Gamma.fit(self, db)

	def toTuple(self):
		return (self._name, self._shape, self._scale, self._shift)

	def fromTuple(self, tuple):
		self._name, self._shape, self._scale, self._shift = tuple


################################################################################
# Beta
################################################################################
class Beta(Distribution):
	'''
    Beta Distribution :

    Special Case of Dirichlet distribution with only 2 parameters
    (alpha and beta).
	'''

	def __init__(self, alpha=None, beta=None):
		Distribution.__init__(self)
		self._name = 'beta'
		self._dirichlet = Dirichlet()
		self._alpha = alpha
		self._beta = beta

	def alpha(self):
		return self._alpha

	def beta(self):
		return self._beta

	def fit(self, X):
		Y = 1. - X
		X = numpy.vstack([X, Y]).T
		self._dirichlet.fit(X)
		self._alpha, self._beta = self._dirichlet.alpha()

	def likelihood(self, x):
		x = numpy.array([x, 1. - x])
		return self._dirichlet.likelihood(x)

	def toTuple(self):
		return (self._name, self._alpha, self._beta)

	def fromTuple(self, tuple):
		self._name, self._alpha, self._beta = tuple

	def update(self):
		'''update internal parameters'''
		self._dirichlet.set_alpha([self._alpha, self._beta])
		self._dirichlet.update()

################################################################################
# Exponential
################################################################################
class Exponential(Distribution):
	'''
    Exponential distribution with density :
    P(x|l) = l * e^(- l * x)
	'''
	def __init__(self, _lambda=None):
		Distribution.__init__(self)
		self._name = 'exponential'
		self._lambda = _lambda

	def setLambda(self, _lambda): self._lambda = _lambda

	def getLambda(self): return self._lambda

	def fit(self, X, weights=None):
		if weights is not None:
			weights = numpy.asarray(weights).ravel()
			weights = weights / weights.sum()
		X = X.ravel()
		if weights is None:
			self._lambda = 1. / X.mean()
		else:	self._lambda = 1. / numpy.dot(weights, X)

	def likelihood(self, x):
		logli = numpy.log(self._lambda) - self._lambda * x
		return logli, numpy.exp(logli)

	def likelihoods(self, X):
		return self.likelihood(X)

	def toTuple(self):
		return (self._name, self._lambda)

	def fromTuple(self, tuple):
		name, self._lambda = tuple


class GeneralizedExponential(Distribution):
	'''
    Generalization of Exponential distribution, sub-model of the more general
    family of Generalized Gamma, with density :

    P(x|m, a) = m * exp(-(x * m * a * gamma(a + 1))^(1/a)) with a, m > 0

    m is the density value at (x = 0), a is a shape parameter
	'''
	def __init__(self, mu=None, alpha=None):
		Distribution.__init__(self)
		self._name = 'generalized_exponential'
		self._mu = mu
		self._alpha = alpha
		self._mu_max = 10

	def setAlpha(self, alpha): self._alpha = alpha

	def getAlpha(self): return self._alpha

	def setMu(self, mu): self._mu = mu

	def setMuMax(self, mu): self._mu_max = mu

	def getMu(self): return self._mu

	def fit(self, X, weights=None):
		if weights is not None:
			weights = numpy.asarray(weights).ravel()
			weights = weights / weights.sum()

		def func(p, X, weights):
			a = p[0]
			m = p[1]
			if a <= 1: return numpy.inf
			if m <= 0: return numpy.inf
			if m >= self._mu_max: return numpy.inf
			self.setAlpha(a)
			self.setMu(m)
			self.update()
			energy = 0.
			if weights is not None:
				zw = (weights != 0)
				weights = weights[zw]
				X = X[zw]
			logli, li = self.likelihoods(X)
			if weights is not None:
				energy = -numpy.dot(logli.ravel(), weights)
			else:	energy = -logli.mean()
			return energy

		import scipy.optimize
		p0 = numpy.array([1., 1.])
		eps = 10e-5
		res = scipy.optimize.fmin_powell(func, p0, args=(X, weights),
							disp=0, ftol=eps)
		self._alpha = res[0]
		self._mu = res[1]
		self.update()

	def likelihood(self, x):
		a = self._alpha
		logli = numpy.log(self._mu) - \
			(x * self._mu * scipy.special.gamma(a + 1)) ** (1 / a)
		return logli, numpy.exp(logli)

	def likelihoods(self, X):
		return self.likelihood(X)

	def toTuple(self):
		return (self._name, self._mu, self._alpha)

	def fromTuple(self, tuple):
		name, self._mu, self._alpha = tuple



################################################################################
# Dirac
################################################################################
class Dirac(Distribution):
	def __init__(self, values = None):
		Distribution.__init__(self)
		self._name = 'dirac'
		self._values = values

	def fit(self, db):
		X = db.getX()
		self._values = X.mean(axis=0)

	def toTuple(self):
		return (self._name, self._values)

	def fromTuple(self, tuple):
		name, self._values = tuple

	def likelihood(self, x):
		b = (x == self._values).all() + 0.
		if b == 1:
			return 0., 1.
		# prevent -inf
		else:	return -1000., 0.

	def likelihood_expectation(self):
		return 0., 1.

	def appf(self, pct):
		if pct == 1:
			return 1
		else:	return numpy.nan


################################################################################
# Von Misses
################################################################################
class VonMises(Distribution):
	def __init__(self, mu=0., kappa=0., bias=True):
		'''
    Von Mises model for 1D angles [0...2pi] circular data
    ref : Topics in Circular Statistics, S. Rao Jammalamadaka, Ambar Sengupta

    mu :     mean direction
    kappa :  concentration parameter
    bias :   usefull for kappa estimation from small sample (n < 16).
             Thus the distribution tend to follow a flat distribution when n
	     tends to zero.
		'''
		Distribution.__init__(self)
		self._name = 'von_mises'
		self._mu = mu
		self._kappa = kappa
		self._bias = bias
		self._normalization = None

	def _check(self, X):
		if X.shape[1] == 1: return 1
		print "error : Von Misses : accept only 1D data of angles"
		return 0

	def _Ainv(self, x):
		'''Approximated function of the inverse of I1(K) / I0(K)'''
		if x >= 0 and x < 0.53:
			return 2 * x + x**3 + (5 * x**5) / 6
		elif x < 0.85:
			return -0.4 + 1.39 * x + 0.43 / (1 - x)
		else:	return 1 / (x**3 - 4 * x**2 + 3 * x)

	def kappa(self): return self._kappa

	def mu(self): return mu

	def set_kappa(self, kappa):
		'''Kappa ~= 1 / variance'''
		self._kappa = kappa

	def set_mu(self, mu):
		'''mu = esperance)'''
		self._mu = mu

	def update(self):
		'''update internal parameters'''
		self._normalization = 2 * numpy.pi * \
			scipy.special.i0(self._kappa)

	def compute_mu(self, X):
		'''
    Mean/Mode mle estimate.

    return arctan(sum(sin(x))/sum(cos(X)))
		'''
		S = numpy.sin(X).sum()
		C = numpy.cos(X).sum()
		if C == 0:
			if S == 0: return numpy.nan
			if S > 0: return numpy.pi / 2.
		a = numpy.arctan(S / C)
		if C < 0: return a + numpy.pi
		if S >= 0: return a
		return a + 2 * numpy.pi

	def compute_kappa(self, X, mu):
		'''
    Kappa (concentration parameter) mle estimate.

              I1(K)     R
    Solve    ------- = ---    with I0 first modified Bessel function of order 0
              I0(K)     N          I1   "       "      "       "      "    "  1
    		'''
		n = len(X)
		R = numpy.cos(X - mu).sum()
		kappa = self._Ainv(R / n)
		if self._bias:
			if kappa < 2:
				kappa = numpy.max(kappa - 2. / (n * kappa), 0)
			else:	kappa *= ((n - 1) ** 3) / (n ** 3 + n)
		return kappa

	def fit(self, db):
		X = db.getX()
		if not self._check(X): return
		self._mu = self.compute_mu(X)
		self._kappa = self.compute_kappa(X, self._mu)
		self.update()

	def likelihood(self, x):
		k_mut_x = self._kappa * numpy.cos(x - self._mu)
		logli = k_mut_x - numpy.log(self._normalization)
		li = numpy.exp(k_mut_x) / self._normalization
		return logli, li

	def toTuple(self):
		return (self._name, self._mu, self._kappa)

	def fromTuple(self, tuple):
		self._name, self._mu, self._kappa = tuple


class VonMisesFisher(Distribution):
	def __init__(self, mu=0., kappa=0., p=None):
		'''
    Von Mises model for p-dimensional unitary spherical data
    ref : "Clustering on the Unit Hypersphere using von Mishes-Fisher
    Distributions", A. Banerjee, I. S. Dhillon, J. Ghosh, S. Sra.
    Also known as Langevin distribution.

    optional parameters :
    ---------------------
    mu :     mean direction
    kappa :  concentration parameter
    p :      dimensionality of data
		'''
		Distribution.__init__(self)
		self._name = 'von_mises_fisher'
		self._mu = mu
		self._kappa = kappa
		self._dim = p
		if not None in [mu, kappa, p]:
			self._update()
		else: self._normalization = None

	def _check(self, X):
		if (((X ** 2).sum(axis=1) - 1) ** 2 > 10e-5).any(): return 1
		print "error : Von Misses : accept only unit vectorial data"
		return 0

	def _Ainv_bisect(self, d, r, eps=10e-5):
		# init : approximate value from ref
		k = r * (d - r ** 2) / (1 - r ** 2)
		def f(k):
			Ad_k = scipy.special.iv(d / 2., k) / \
				scipy.special.iv(d / 2. - 1., k)
			return Ad_k - r
		import scipy.optimize
		return scipy.optimize.bisect(f, 10e-100, k, xtol=eps)

	def _Ainv(self, d, r, eps=10e-5):
		# init : approximate value from ref
		k = r * (d - r ** 2) / (1 - r ** 2)
		diff = eps + 1
		# newton-raphson algorithm to refine init
		while diff > eps:
			Ad_k = scipy.special.iv(d / 2., k) / \
				scipy.special.iv(d / 2. - 1., k)
			f_k = Ad_k - r
			fp_k = 1. - Ad_k * (Ad_k + (d - 1.) / k)
			diff = f_k / fp_k
			k -= diff
			if k < 0: k = diff = 0
		return k

	def kappa(self): return self._kappa

	def mu(self): return self._mu

	def set_kappa(self, kappa):
		'''Kappa ~= 1 / variance'''
		self._kappa = kappa

	def set_mu(self, mu):
		'''mu = esperance)'''
		self._mu = self._mu

	def update(self):
		'''update internal parameters'''
		self._normalization = (self._kappa) ** (self._dim / 2. - 1) / \
			((2 * numpy.pi) ** (self._dim / 2.) * \
			scipy.special.iv(self._dim / 2. - 1, self._kappa))

	def compute_mu(self, X, weights=None):
		'''
    Mean/Mode mle estimate.

    mu = E(X) / || E(X) ||
		'''
		if weights is None:
			R = X.sum(axis=0)
		else:	R = numpy.dot(weights * X.shape[0] / weights.sum(), X)
		Rnorm = numpy.linalg.norm(R)
		return Rnorm, R / Rnorm

	def compute_kappa(self, Rnorm, X, mu, weights=None):
		'''
    Kappa (concentration parameter) mle estimate.

               I_d/2(K)      ||R||
    Solve    ------------ = -------
              I_d/2-1(K)       N
	      
    with N :   number of data
         R :   ||E(X)||
         I_d : first modified bessel function of order p
    		'''
		n, self._dim = X.shape
		kappa = self._Ainv(self._dim, Rnorm / n)
		return kappa

	def fit(self, db, weights=None):
		try:
			X = db.getX()
			if not self._check(X): return
		except AttributeError:
			X = db
		R, self._mu = self.compute_mu(X, weights)
		self._kappa = self.compute_kappa(R, X, self._mu, weights)
		self.update()

	def likelihood(self, x):
		x = numpy.asarray(x).ravel()
		k_mut_x = self._kappa * numpy.dot(x, self._mu)
		logli = numpy.log(self._normalization) + k_mut_x
		li = self._normalization * numpy.exp(k_mut_x)
		return logli, li

	def toTuple(self):
		return (self._name, self._mu, self._kappa, self._dim)

	def fromTuple(self, tuple):
		self._name, self._mu, self._kappa, self._dim = tuple


class Kent(Distribution):
	def __init__(self, gamma=None, kappa=None, beta=None):
		'''
    Kent distribution for data living on a 2 dimensional sphere.
    also known as Fisher-Bingham FB5 distribution.

    The Fisher-Bingham Distribution on the Sphere

    optional parameters :
    ---------------------
    gamma :  3x3 orthogonal matrix [gamma1, gamm2, gamm3] with :
       - gamma 1 : mean direction
       - gamma 2 : major axes of distribution
       - gamma 3 : minor axes of distribution
    kappa :   concentration parameter (float)
    beta :    ellipticity of distribution (float) with 0 <= 2 * beta < kappa


    given the following density function :

            exp (k*<x,g1> + b * (<x,g2>^2 - <x,g3>^2)) * 2 * pi
    f(x) = -----------------------------------------------------
                       exp(-k) * sqrt(k^2 - 4*b^2)
		'''
		Distribution.__init__(self)
		self._name = 'kent'
		self._gamma = gamma
		self._kappa = kappa
		self._beta = beta
		if not None in [gamma, kappa, beta]:
			self._update()
		else: self._normalization = None

	def update(self):
		'''update internal parameters'''
		k, b = self._kappa, self._beta
		if b == 0 and k == 0: return 1./ (numpy.pi * 4)
		if b:
			self._normalization = numpy.exp(-k) * \
				numpy.sqrt(k ** 2 - 4 * b ** 2) / (2 * numpy.pi)
		else:	self._normalization = k / (4 * numpy.pi * numpy.sinh(k))
		if b:
			self._lognormalization = -k + 0.5 * \
				(numpy.log(k - 2 * b) + numpy.log(k + 2 * b)) -\
				 numpy.log(2 * numpy.pi)
		else:	self._lognormalization = numpy.log(k) - \
				numpy.log(4 * numpy.pi) - \
				numpy.log(numpy.sinh(k))

	def polar_coordinates(self, x):
		'''
    carthesian coordinates to polar coordinates.

    return g1, g2 = dip angle, dip direction
    with 0 <= g1 <= 2pi and 0 <= g2 <= pi/2
		'''
		x1, x2, x3 = x / numpy.linalg.norm(x)
		dip_angle = numpy.arctan2(x3, x2)
		dip_direction = numpy.arccos(x1)
		return dip_angle, dip_direction
	
	def fit(self, X, weights=None):
		'''
    parameter estimation based on first and second moments, tends to be close to
    Maximum Likelihoods Estimators when data size tends to infinity.
		'''
		n = X.shape[0]
		if weights is not None: weights = weights / weights.sum()
		if weights is None:
			xm = X.mean(axis=0)
		else:	xm = numpy.dot(weights, X)
		phi, theta = self.polar_coordinates(xm)
		R = numpy.linalg.norm(xm)
		if weights is None:
			S = (numpy.asmatrix(X).T * numpy.asmatrix(X)) / n
		else:	S = numpy.dot(X.T, (X.T * weights).T)
		
		H = numpy.matrix([\
			[numpy.cos(theta), -numpy.sin(theta), 0],
			[numpy.sin(theta) * numpy.cos(phi),
			numpy.cos(theta) * numpy.cos(phi), -numpy.sin(phi)],
			[numpy.sin(theta) * numpy.sin(phi),
			numpy.cos(theta) * numpy.sin(phi), numpy.cos(phi)]])
		B = H.T * S * H
		B2 = B[1:, 1:]
		psi = 0.5 * numpy.arctan2(2. * B[1,2], B[1,1] - B[2, 2])
		K = numpy.matrix([[1, 0, 0],
			[0, numpy.cos(psi), -numpy.sin(psi)],
			[0, numpy.sin(psi), numpy.cos(psi)]])
		G = (H * K)
		T = G.T * S * G
		w = T[1, 1] - T[2, 2]
		R2 = 2 - 2 * R
		Rw1 = 1. / (R2 - w)
		Rw2 = 1. / (R2 + w)
		self._kappa = Rw1 + Rw2
		self._beta = 0.5 * (Rw1 - Rw2)
		self._gamma = G.T
		self.update()

	def likelihood(self, x):
		k, b = self._kappa, self._beta
		g1, g2, g3 = self._gamma
		dots = (k * numpy.dot(g1, x) + b * (numpy.dot(g2, x) ** 2 - \
						numpy.dot(g3, x) ** 2))[0, 0]
		logli = dots + self._lognormalization
		li = numpy.exp(dots) * self._normalization
		return logli, li

	def toTuple(self):
		return (self._name, self._gamma, self._kappa, self._beta)

	def fromTuple(self, tuple):
		self._name, self._gamma, self._kappa, self._beta = tuple


class Watson(Distribution):
	def __init__(self, mu=None, kappa=None):
		'''
    Watson distribution for data living on a p-dimensional sphere.
    Provide a spherical gaussian-like distribution on axial data.

    optional parameters :
    ---------------------
    mu :     mean direction
    kappa :  concentration parameter

    given the following density function :

            exp (k * (<x, mu>)^2)
    f(x) = -----------------------
                    C(k)
                                        k
    here C(k) is approximate by :  -----------,   valid for hig value of kappa
                                    pi.exp(k)

    ref : "Cross-Subject Comparison of Principal Diffusion Direction Maps"
    ref : "Equatorial Distributions on a Sphere", G.S. Watson
          It provides approximations for large/small values of kappa and a
	  table for intermediate values which can be used doing linear
	  interpolation.
		'''
		Distribution.__init__(self)
		self._name = 'watson'
		self._mu = mu
		self._kappa = kappa
		if not None in [mu, kappa]:
			self._update()
		else: self._normalization = self._lognormalization = None

	def kappa(self): return self._kappa

	def mu(self): return self._mu

	def update(self):
		'''update internal parameters'''
		k, m = self._kappa, self._mu
		self._normalisation = k / (numpy.pi * numpy.exp(k))
		self._lognormalisation = numpy.log(k) - numpy.log(numpy.pi) - k

	def fit(self, X, weights=None):
		'''
    fit parameters by maximum likelihood estimation with some approximations
    valid when the concentration parameter (kappa) is high.
		'''
		n = X.shape[0]
		if weights is not None: weights = weights / weights.sum()
		if weights is None:
			S = (numpy.asmatrix(X).T * numpy.asmatrix(X)) / n
		else:	S = numpy.dot(X.T, (X.T * weights).T)
		val, vec = numpy.linalg.eig(S)
		self._mu = vec[numpy.argmax(val)]
		C = numpy.dot(X, self._mu.T)
		if weights is None:
			C /= n
		else:	C = (C.T * weights).T
		self._kappa = 1. / (1. - (numpy.asarray(C) ** 2).sum())
		self.update()

	def likelihood(self, x):
		k, m = self._kappa, self._mu
		d = numpy.dot(x, mu)
		logli = k * (d ** 2) + self._lognormalisation
		li = numpy.exp(k * (d ** 2)) * self._normalisation
		return logli, li

	def toTuple(self):
		return (self._name, self._mu, self._kappa)

	def fromTuple(self, tuple):
		self._name, self._mu, self._kappa = tuple


################################################################################
# Dirichlet and generalized Dirichlet
################################################################################
class Dirichlet(Distribution):
	'''
    Dirichlet Distribution :

    Multivariate generalization of the beta distribution on a simplex.

    ref: A Bayesian approach employing generalized dirichlet priors in
         predicting microchip yields.
	'''
	def __init__(self, alpha=None):
		Distribution.__init__(self)
		self._name = 'dirichlet'
		self._alpha = alpha
		if alpha is not None:
			self.update()
		else:	self._logB = None

	def _check(self, X):
		if ((X.sum(axis=1) - 1) ** 2 < 10e-5).all(): return 1
		print "error : Dirichlet accepts only data on the " \
			"(dim-1) simplex : for each X, sum(X)=1."
		return 0

	def update(self):
		self._logB = self.compute_lognormalization()

	def compute_lognormalization(self):
		return numpy.sum(scipy.special.gammaln(self._alpha)) - \
			scipy.special.gammaln(numpy.sum(self._alpha))

	def set_alpha(self, alpha):
		self._alpha = alpha

	def alpha(self): return self._alpha

	def marginal_esperance(self):
		return self._alpha / self._alpha.sum()

	def marginal_variance(self):
		a0 = self._alpha.sum()
		return self._alpha * (a0 - self._alpha) / (a0 ** 2 * (a0 + 1))

	def fit(self, X):
		'''
    estimate k = sum(alpha_i) through minimizing MSE :
    k = argmin_k sum (var_i - marginal_variance(X_i))^2
		'''
		if not self._check(X): return
		fi = X.mean(axis=0)
		var_fi = X.var(axis=0)
		f0 = fi.sum()
		a = ((fi * (f0 - fi)) ** 2).sum()
		b = f0 ** 2 * (fi * (f0-fi) * var_fi).sum()
		k = (a / b - 1) / f0
		self._alpha = k * fi
		self.update()

	def likelihood(self, x):
		'''x must sum to 1'''
		logli = ((self._alpha - 1) * numpy.log(x)).sum() - self._logB
		if numpy.isnan(logli): return -numpy.inf, 0.
		return logli, numpy.exp(logli)

	def toTuple(self):
		return (self._name, self._alpha, self._logB)

	def fromTuple(self, tuple):
		self._name, self._alpha, self._logB = tuple


class GeneralizedDirichlet(Dirichlet):
	def __init__(self, alpha=None, beta=None):
		Dirichlet.__init__(self)
		self._name = 'generalized_dirichlet'
		self._alpha = alpha
		self._beta = beta
		if (alpha is not None) and (beta is not None):
			self.update()
		else:
			self._logB = None
			self._gamma_i = None
			self._marginal_esperance = None
			self._marginal_variance = None

	def update(self):
		self._logB = self.compute_lognormalization(\
					self._alpha, self._beta)
		self._gamma_i = self.compute_gamma_power(self._alpha,self._beta)
		self._marginal_esperance = self.compute_marginal_esperance(\
						self._alpha, self._beta)
		self._marginal_variance = self.compute_marginal_variance(\
			self._marginal_esperance, self._alpha, self._beta)

	def alpha(self): return self._alpha

	def beta(self): return self._beta

	def marginal_esperance(self): return self._marginal_esperance

	def marginal_variance(self): return self._marginal_variance

	def compute_marginal_esperance(self, alpha, beta):
		p = alpha / (alpha + beta)
		p2 = numpy.cumprod(1 - p)
		e = numpy.hstack((p * numpy.hstack((1., p2[:-1])), p2[-1]))
		return e

	def compute_marginal_variance(self, marginal_esperance, alpha, beta):
		e = marginal_esperance
		c = (alpha + beta + 1)
		p, p2 = (alpha + 1) / c, (beta + 1) / c
		p2 = numpy.cumprod(p2)
		v = e[:-1] * p * \
			numpy.hstack((1., p2[:-1])) - e[:-1] ** 2
		vk = e[-1] * p2[-1] - e[-1] ** 2
		return numpy.hstack((v, vk))
		
	def compute_lognormalization(self, alpha, beta):
		return numpy.sum(scipy.special.betaln(alpha, beta))

	def compute_gamma_power(self, alpha, beta):
		'''
    gamma_i = beta_i - alpha_(i+1) - beta_(i+1)
    gamma_k = beta_k - 1
		'''
		return numpy.hstack((beta[:-1] - alpha[1:] - beta[1:],
							beta[-1]-1))

	def fit(self, X):
		if not self._check(X): return
		v = X.var(axis=0)
		m = X.mean(axis=0)
		alpha, beta = [], []
		k, h = 1., 1.
		for i in range(X.shape[1] - 1):
			m_i, v_i = m[i], v[i]
			n = (m_i * (h - m_i) - v_i)
			d = (m_i ** 2 * (k - h) + k * v_i)
			if not d:
				print "error fitting generalized dirichlet"
				return
			div = n / d
			a_i = m_i * div
			b_i = (k - m_i) * div
			alpha.append(a_i)
			beta.append(b_i)
			k *= b_i / (a_i + b_i)
			h *= (b_i + 1) / (a_i + b_i + 1)
		self._alpha = numpy.array(alpha)
		self._beta = numpy.array(beta)
		self.update()

	def likelihood(self, x):
		'''x must sum to 1, thus last value is not directly used'''
		if (x <= 0).any() or x[:-1].sum() >= 1: return -numpy.inf, 0
		x_inv = 1 - numpy.cumsum(x[:-1])
		logli = numpy.sum((self._alpha - 1) * numpy.log(x[:-1]) + \
			self._gamma_i * numpy.log(x_inv)) - self._logB
		return logli, numpy.exp(logli)

	def toTuple(self):
		return (self._name, self._alpha, self._beta,
					self._logB, self._gamma_i)

	def fromTuple(self, tuple):
		self._name, self._alpha, self._beta, \
			self._logB, self._gamma_i = tuple

class ShiftedDirichlet(Dirichlet):
	'''
    Variant of dirichlet where data are not already on a simplex.
    So, it is possible to add a shift to the learned/estimated values to avoid
    null likelihood for the joint density when at least one of the estimated
    variable is null.

    shift : number to shift all learned/estimated values.
            float (same shift fort all variables) or array of float (one shift
	    per variable)
	'''
	def __init__(self, shift=0., alpha=None):
		Dirichlet.__init__(self, alpha)
		self._name = 'shifted_dirichlet'
		self._shift = shift

	def fit(self, X):
		X2 = X + self._shift
		X2 = (X2.T / X2.sum(axis=1)).T
		Dirichlet.fit(self, X2)

	def native_likelihood(self, x):
		'''x is supposed to be already shifted and normalized'''
		return Dirichlet.likelihood(self, x)

	def likelihood(self, x):
		'''x is supposed to be unshifted an unnormalized'''
		x2 = (x + self._shift)
		x2 /= x2.sum()
		logli, li = Dirichlet.likelihood(self, x2)
		return logli, li

	def toTuple(self):
		return (self._name, self._shift, self._alpha, self._logB)

	def fromTuple(self, tuple):
		self._name, self._shift, self._alpha, self._logB = tuple




class ShiftedGeneralizedDirichlet(GeneralizedDirichlet):
	'''
    Variant of generalized dirichlet where data are not already on a simplex.
    So, it is possible to add a shift to the learned/estimated values to avoid
    null likelihood for the joint density when at least one of the estimated
    variable is null.
	'''
	def __init__(self, shift=0., alpha=None, beta=None):
		GeneralizedDirichlet.__init__(self, alpha, beta)
		self._name = 'shifted_generalized_dirichlet'
		self._shift = shift

	def fit(self, X):
		X2 = X + self._shift
		X2 = (X2.T / X2.sum(axis=1)).T
		GeneralizedDirichlet.fit(self, X2)

	def native_likelihood(self, x):
		'''x is supposed to be already shifted and normalized'''
		return GeneralizedDirichlet.likelihood(self, x)

	def likelihood(self, x):
		'''x is supposed to be unshifted an unnormalized'''
		x2 = (x + self._shift)
		x2 /= x2.sum()
		logli, li = GeneralizedDirichlet.likelihood(self, x2)
		return logli, li

	def toTuple(self):
		return (self._name, self._shift, self._alpha, self._beta,
					self._logB, self._gamma_i)

	def fromTuple(self, tuple):
		self._name, self._shift, self._alpha, self._beta, \
			self._logB, self._gamma_i = tuple


################################################################################
# Mixture Model
################################################################################
class MixtureModel(Distribution):
	'''
             ___
    P(X=x) = \   P(X=x|L=li) P(L=li)
             /__
              li

    models : list of P(X|L=li) Distribution with likelihoods method
    priors : frequencies (1D numpy.array)
	'''
	def __init__(self, models=None, priors=None):
		if not None in [models, priors] and len(priors) != len(models):
			raise ValueError("number of models/priors mismatch.")
		self._models = models
		self._priors = priors

	def set_models(self, models): self._models = models

	def set_priors(self, priors): self._priors = priors

	def get_models(self): return self._models

	def get_priors(self): return self._priors

	def likelihoods(self, X):
		'''
    return matrices with shape (number of vectors, number of labels)
		'''
		loglikelihoods = []
		likelihoods = []
		for m in self._models:
			logli, li = m.likelihoods(X)
			loglikelihoods.append(logli)
			likelihoods.append(li)
		loglikelihoods = numpy.vstack(loglikelihoods)
		likelihoods = numpy.vstack(likelihoods)
		return loglikelihoods.T, likelihoods.T

	def likelihoods_groups(self, X, groups, L=None):
		'''
    return matrices with shape (number of vectors, number of labels)
		'''
		loglikelihoods = []
		likelihoods = []
		for i, m in enumerate(self._models):
			if L is not None:
				L_i = L[:, i]
			else:	L_i = None
			logli, li = m.likelihoods_groups(X, groups, L_i)
			loglikelihoods.append(logli)
			likelihoods.append(li)
		loglikelihoods = numpy.vstack(loglikelihoods)
		likelihoods = numpy.vstack(likelihoods)
		return loglikelihoods.T, likelihoods.T

	def posteriors(self, X):
		loglikelihoods, likelihoods = self.likelihoods(X)
		if self._priors is not None:
			posteriors = numpy.multiply(likelihoods, self._priors)
		else:	posteriors = likelihoods
		posteriors = numpy.divide(posteriors.T,
				posteriors.sum(axis=1)).T
		return posteriors, loglikelihoods, likelihoods

	def posteriors_groups(self, X, groups, L=None):
		'''
    L : array 2D of available labels with shape : (data's number, labels'number)
		'''
		loglikelihoods, likelihoods = self.likelihoods_groups(X,
								groups, L)
		if self._priors is not None:
			posteriors = numpy.multiply(likelihoods, self._priors)
		else:	posteriors = likelihoods
		posteriors = numpy.divide(posteriors.T,
				posteriors.sum(axis=1)).T
		return posteriors, loglikelihoods, likelihoods

	def __len__(self): return len(self._models)

	def toTuple(self):
		t = {}
		for m in self._models:
			t[m.name()] = m.toTuple()
		return (self._name, t, self._priors)

	def fromTuple(self, tuple):
		self._name, t, self._priors = tuple
		df = distributionFactory
		self._models = []
		for name, subtuple in t.items():
			model = distributionFactory(name)()
			model.fromTuple(subtuple)
			self._models.append(model)


class GaussianMixtureModel(MixtureModel):
	def __init__(self, gaussians=None, priors=None):
		MixtureModel.__init__(self, gaussians, priors)
		self._name = 'gaussian_mixture'

	def set_means_covs(self, means, covariances):
		if len(means) != len(covariances):
			raise ValueError("number of means/covariances " + \
								"mismatch.")
		models = []
		for i, mean in enumerate(means):
			cov = covariances[i]
			g = Gaussian()
			g.setMeanCov(mean, cov)
			models.append(g)
		self.set_models(models)

	def get_means(self):
		return [m.mean() for m in self._models]

	def get_metrics(self):
		return [m.metric() for m in self._models]

	def get_covariances(self):
		return [m.covariance() for m in self._models]



class GammaExponentialMixtureModel(MixtureModel):
	def __init__(self, models=None, priors=None):
		MixtureModel.__init__(self, models, priors)
		self._name = 'gamma_exponential_mixture'

	def em(self, X, maxiter=1000, eps=10e-5):
		# models
		e = GeneralizedExponential()
		g = Gamma()

		# init of EM
		size = len(X)
		zind = (X == 0).ravel()
		Y = numpy.sort(X[X!= 0].ravel())
		lY = len(Y)
		if lY:
			r = numpy.random.randint(5, 20)
			v = Y[(r * lY)/ 100]
		else: # default mixture model if all values are null
			e.setMu(10)
			e.setAlpha(5.)
			g.setScale(1.)
			g.setShape(1.)
			models = [e, g]
			priors = [0.99, 0.01]
			en = 0.
			return models, priors, en
		we = (X.ravel() <= v) + 0.
		wg = 1. - we
		mu = max(1., float(zind.sum())) / len(X)
		we[zind] = 1.
		wg[zind] = 0.

		n = 0
		# EM optimization
		old_en = numpy.inf
		gamma = 1.001
		while 1:
			# M step
			# priors P(z=c|pi) = pi_c with P(pi|gamma) = dir(gamma)
			d = 2 * size * (gamma - 1.)
			prior_e = we.sum() + d
			prior_g = wg.sum() + d
			prior_sum = prior_e + prior_g
			prior_e /= prior_sum
			prior_g /= prior_sum
			e.fit(X, we)
			g.fit(X, wg)
			# force gaussian-like shape rather than exponential one
			if g.shape() < 1.: # gamma(x=0) = 0
				g.setShape(1.)
				scale = g._compute_scale(X.ravel(), wg)
				g.setScale(scale)
				g.update()
			# to avoid gamma parameters ranges given nan value
			# for likelihood
			if g.shape() >= 100:
				g.setShape(100.)
				scale = g._compute_scale(X.ravel(), wg)
				if scale < 0.01: scale = 0.01
				g.setScale(scale)
				g.update()

			# E step
			pe = e.likelihoods(X)[1].ravel() * prior_e
			pg = g.likelihoods(X)[1].ravel() * prior_g
			ze = (we != 0)
			zg = (wg != 0)
			en = -(we[ze] * numpy.log(pe[ze])).sum() + \
				(wg[zg] * numpy.log(pg[zg])).sum()
			en -= 2 * size * (gamma - 1) * (numpy.log(prior_e) + \
							numpy.log(prior_g))
			en /= size
			s = pe + pg
			we = pe / s
			wg = pg / s
			# avoid NaN and 0 values for gamma distribution
			z = (numpy.isnan(we) + numpy.isnan(wg)) != 0
			we[z] = 0.5
			wg[z] = 0.5
			we[zind] = 1.
			wg[zind] = 0.

			n += 1
			if (n > 2 and old_en - en < eps) or n >= maxiter: break
			old_en = en
		
		models = [e, g]
		priors = [prior_e, prior_g]
		return models, priors, en

	def fit(self, X, maxiter=1000, eps=10e-5):
		best_en = numpy.inf
		best_data = None
		n = 0
		while (best_data is None) or (n < 5):
			models, priors, en = self.em(X, maxiter, eps)
			if en < best_en:
				best_en = en
				best_data = models, priors
			if models: n += 1
		self._models = best_data[0]
		self._priors = best_data[1]

	def likelihoods(self, X):
		logli0, li0 = self._models[0].likelihoods(X)
		logli1, li1 = self._models[1].likelihoods(X)
		li = li0 * self._priors[0] + li1 * self._priors[1]
		return numpy.log(li), li


################################################################################
distribution_map = { \
	'full_gaussian' : Gaussian,
	'diagonal_gaussian' : DiagonalGaussian,
	'spherical_gaussian' : SphericalGaussian,
	'spherical_gaussian_few_dots' : SphericalGaussianFewDots,
	'spherical_gaussian_fixed' : FixedSphericalGaussian,
	'fake_gaussian' : FakeGaussian,
	'oriented_gaussian' : OrientedGaussian,
	'bloc_gaussian' : BlocGaussian,
	'robust_gaussian' : RobustGaussian,
	'gamma' : Gamma,
	'shifted_gamma' : ShiftedGamma,
	'exponential' : Exponential,
	'generalized_exponential' : GeneralizedExponential,
	'beta' : Beta,
	'dirac' : Dirac,
	'von_mises' : VonMises,
	'von_mises_fisher' : VonMisesFisher,
	'kent' : Kent,
	'watson' : Watson,
	'frequency' : Frequency,
	'dirichlet' : Dirichlet,
	'shifted_dirichlet' : ShiftedDirichlet,
	'generalized_dirichlet' : GeneralizedDirichlet,
	'shifted_generalized_dirichlet' : ShiftedGeneralizedDirichlet,
	'gaussian_mixture' : GaussianMixtureModel,
	'gamma_exponential_mixture' : GammaExponentialMixtureModel,
}


def distributionFactory(name):
	return distribution_map[name]
