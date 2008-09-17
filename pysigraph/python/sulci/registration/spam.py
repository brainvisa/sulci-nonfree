import scipy.optimize
from procrust import Registration, MixtureGlobalRegistration, \
				MixtureLocalRegistration
from sulci.models import distribution, distribution_aims
import numpy


class SpamRegistration(Registration):
	'''
    Spam registration with spherical priors on translation and rotation.

    spam :   one spam
    g : gravity center of sulcus
    X :      data (voxels position)
    R_angle_var :  angle prior variance from identity rotation
    R_dir_var :    variance of rotation direction from R_dir_mean mean direction
    R_dir_mean :   mean rotation direction
    t_var :  prior variance from null translation

    if None value are specified the corresponding prior is removed.
	'''
	def __init__(self, spam, g, X, R_angle_var=numpy.pi / 8.,
			R_dir_var=numpy.pi / 8., R_dir_mean=None, t_var=0.5,
			verbose=0):
		Registration.__init__(self, verbose=verbose)
		self._spam = spam
		self._g = g
		self._X = X
		if R_angle_var is None:
			self._angle_prior = None
		else:
			self._angle_prior = distribution.VonMises()
			self._angle_prior.set_mu(0.)
			self._angle_prior.set_kappa(1. / R_angle_var)
			self._angle_prior.update()
		if (R_dir_var is None) or (R_dir_mean is None):
			self._dir_prior = None
		else:
			self._dir_prior = distribution.VonMisesFisher(p=3)
			self._dir_prior.set_mu(R_dir_mean)
			self._dir_prior.set_kappa(1. / R_dir_var)
			self._dir_prior.update()
		if t_var is None:
			self._t_prior = None
		else:
			self._t_prior = distribution.Gaussian()
			id = numpy.identity(3)
			self._t_prior.setMeanCov(numpy.zeros(3), t_var * id)

	def energy(self):
		X2 = self._R * (self._X - self._g) + self._t + self._g
		w_norm = numpy.linalg.norm(self._w)
		if w_norm:
			dir = self._w / w_norm
		else:	dir = self._w
		s_logli, s_li = self._spam_energy(X2)
		if self._angle_prior:
			theta = numpy.arccos((numpy.trace(self._R) - 1.) / 2.)
			a_logli, a_li = self._angle_prior.likelihood(theta)
		else:	a_logli, a_li = 0., 1.
		if self._dir_prior:
			d_logli, d_li = self._dir_prior.likelihood(dir)
		else:	d_logli, d_li = 0., 1.
		if self._t_prior:
			t_logli, t_li = self._t_prior.likelihood(self._t.T)
		else:	t_logli, t_li = 0., 1.
		en = - (s_logli + a_logli + d_logli + t_logli)
		return en

	def _spam_energy(self, X):
		return self._spam.prodlikelihoods(X.T)

class SpamGroupRegistration(SpamRegistration):
	def __init__(self, spam, g, X, weights, groups,
		R_angle_var=numpy.pi / 2., R_dir_var=numpy.pi / 4.,
		R_dir_mean=None, t_var=3., verbose=0):
		SpamRegistration.__init__(self, spam, g, X, R_angle_var,
				R_dir_var, R_dir_mean, t_var, verbose)
		self._weights = numpy.asarray(weights).ravel()
		self._groups = groups

	def _spam_energy(self, X):
		return self._spam.weighted_prodlikelihoods_groups(X.T,
					self._weights, self._groups)

class SpamMixtureRegistration(Registration):
	def __init__(self, X, weights, spams_mixture, groups, verbose=0):
		Registration.__init__(self, verbose=verbose)
		if len(weights) != len(spams_mixture):
			raise ValueError("number of spams/weights" + \
				"mismatch. (%d != %d)" % (len(spams_mixture), \
								len(weights)))
		self._X = X
		self._weights = numpy.asmatrix(weights)
		self._spams_mixture = spams_mixture
		self._groups = groups
		self._size = len(spams_mixture)

	def energy(self):
		X2 = self._R * self._X + self._t
		en = 0.
		spams = self._spams_mixture.get_models()
		for i, spam in enumerate(spams):
			w = numpy.asarray(self._weights[i]).ravel()
			logli, li = spam.weighted_prodlikelihoods_groups(X2.T,
								w, self._groups)
			en -= logli
		return en


MixtureGlobalRegistration._algo_map.update({
	distribution_aims.SpamMixtureModel : SpamMixtureRegistration,
})

MixtureLocalRegistration._algo_map.update({
	distribution_aims.SpamMixtureModel : SpamGroupRegistration,
})
