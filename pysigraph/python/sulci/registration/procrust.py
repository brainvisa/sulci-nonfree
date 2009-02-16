#!/usr/bin/env python

import os, sys
import numpy
from sulci.models import distribution
from sulci.registration.transformation import RigidTransformation, \
						LocalRigidTransformations


################################################################################
PI_div_2 = 1.5707963267948966

def antisymetric_matrix_from_vector(v):
	vx, vy, vz = v
	return numpy.matrix([[0, -vz, vy], [vz, 0, -vx], [-vy, vx, 0]])

def vector_from_antisymetric_matrix(M):
	return numpy.matrix([[M[2, 1]], [M[0, 2]], [M[1, 0]]])

def rotation_from_antisymetric_matrix(J):
	J = numpy.asmatrix(J)
	dim = J.shape[0]
	J2 = J ** 2
	theta = numpy.sqrt(-numpy.trace(J2) / 2.)
	if theta:
		f = numpy.sin(theta) / theta
		g = (1 - numpy.cos(theta)) / (theta ** 2)
	else:	f, g = 1, 0.5
	return numpy.identity(dim) + f * J + g * (J * J)

def rotation_from_vector(v):
	J = antisymetric_matrix_from_vector(v)
	return rotation_from_antisymetric_matrix(J)

def vector_from_rotation(R):
	'''
	"Vision Stereoscopique et perception multisensorielle", Nicholas Ayache
	12.11 Appendice 2 : passage de la matrice au vecteur rotation, p.222
	'''
	w = numpy.asmatrix(numpy.zeros((3, 1)))
	eps = 0.707106781187
	cost = (numpy.trace(R) - 1)/2.
	if 1 + cost > eps:
		theta = numpy.arccos(cost)
		if theta == 0: w[:] = 0
		else:
			f = numpy.sin(theta) / theta
			w[0] = (R[2, 1] - R[1, 2])
			w[1] = (R[0, 2] - R[2, 0])
			w[2] = (R[1, 0] - R[0, 1])
			w /= 2 * f
	else:
		if 1 + cost == 0:
			theta = numpy.pi
		else:
			m = numpy.zeros(3)
			m[0] = (R[2, 1] - R[1, 2])
			m[1] = (R[0, 2] - R[2, 0])
			m[2] = (R[1, 0] - R[0, 1])
			ind = numpy.argmax(numpy.abs(m[m!=0]))
			theta = numpy.arccos(cost)
			if numpy.sign(m[ind]) < 0: theta = 2 * numpy.pi - theta
		g = (1 - cost) / (theta ** 2)
		S = (numpy.diag(R) - cost) / g
		S[S < 0] = 0. # prevent numerical issues
		w[:] = numpy.asmatrix(numpy.sqrt(S)).T
		ind = numpy.argmax(w)
		z = {}
		z[0, 1] = z[1, 0] = (R[1, 0] + R[0, 1]) / g
		z[0, 2] = z[2, 0] = (R[0, 2] + R[2, 0]) / g
		z[1, 2] = z[2, 1] = (R[2, 1] + R[1, 2]) / g
		for i in range(3):
			if i != ind: w[i] *= numpy.sign(z[ind, i])
	return w

J_i = [antisymetric_matrix_from_vector([1, 0, 0]),
	antisymetric_matrix_from_vector([0, 1, 0]),
	antisymetric_matrix_from_vector([0, 0, 1]),
]


class SphericalKmeans(object):
	'''
    X : unitary data
    k : number of clusters
	'''
	def __init__(self, X, k):
		self._u = numpy.array([1., 0, 0]) # ref vector
		n, dim = X.shape
		if dim == 2:
			Z = numpy.zeros((n, 1))
			X = numpy.hstack((X, Z))
		elif dim != 3:
			raise ValueError("handle only dimension : 2 or 3")
		self._X = numpy.asarray(X)
		self._k = k

	def init_centers(self, verbose=False):
		indices = range(len(self._X))
		numpy.random.shuffle(indices)
		indices = indices[:self._k]
		C = numpy.array([self._X[ind] for ind in indices])
		return C

	def algo_euclidian(self, C, eps=0.001, max_iter=10000, verbose=False):
		if verbose: print "* euclidian optimization"
		return self.algo(self.euclidian_cluster,
			self.euclidian_mean, C, eps, max_iter, verbose)

	def algo_riemannian(self, C, eps=0.001, max_iter=10000, verbose=False):
		if verbose: print "* riemannian optimization"
		return self.algo(self.euclidian_cluster,
			self.riemannian_mean, C, eps, max_iter, verbose)

	def compute(self, eps=0.001, max_iter=10000, verbose=False):
		# centers : random init from data
		C = self.init_centers(verbose)
		C, clusters = self.algo_euclidian(C, 0.1, 10, verbose)
		return self.algo_riemannian(C, eps, max_iter, verbose)

	def algo(self, cluster, mean, C,
		eps=0.01, max_iter=10000, verbose=False):

		# algo
		n = 0
		old_en = 2 * numpy.pi
		while 1:
			clusters, dist = cluster(C, self._X)
			en = dist.mean()
			if verbose: print "n = %3d, en = %f" % (n, en)
			for i in range(self._k):
				C[i] = mean(C[i], self._X[clusters == i])
			n += 1
			if n > max_iter: break
			if (old_en - en) < eps: break
			old_en = en

		return C, clusters


class DirectionalKmeans(SphericalKmeans):
	def __init__(self, *args, **kwargs):
		SphericalKmeans.__init__(self, *args, **kwargs)

	def riemannian_mean(self, C, X, eps=0.01):
		'''
    compute intrinsic riemmanian mean of rotations

    C : initialization of mean
		'''
		theta_mean = eps + 1.
		n = X.shape[0]
		while theta_mean > eps:
			W = numpy.cross(X, -C)
			cos_theta = numpy.dot(X, C)
			cos_theta[cos_theta > 1] = 1.
			cos_theta[cos_theta < -1] = -1.
			sin_theta = numpy.sqrt((W ** 2).sum(axis=1)) # norm
			theta = numpy.arccos(cos_theta)
			theta *= (1 -(sin_theta < 0) * 2)
			z = (sin_theta == 0)
			W[z] = 0
			sin_theta[z] = 1.
			W = (W.T * theta / sin_theta).T
			w = W.mean(axis=0)
			theta_mean = numpy.sqrt((w ** 2).sum()) # norm
			C = numpy.dot(numpy.asarray(rotation_from_vector(w)), C)
		return C

	def argmin_riemannian_dist2(self, C, x):
		X = antisymetric_matrix_from_vector(x)
		W = -numpy.asarray((X * C.T).T)
		cos_theta = numpy.dot(C, x)
		cos_theta[cos_theta > 1] = 1.
		cos_theta[cos_theta < -1] = -1.
		sin_theta = numpy.sqrt((W ** 2).sum(axis=1)) # norm
		theta = numpy.arccos(cos_theta)
		theta *= (1 -(sin_theta < 0) * 2)
		ind = numpy.argmin(theta)
		return ind, theta[ind]

	def riemannian_cluster(self, C, X):
		S = [antisymetric_matrix_from_vector(c) for c in C]
		S = numpy.asarray(numpy.vstack(S))
		S.shape = (self._k, 3, 3)
		# cross product between all C and X
		W = numpy.tensordot(S, X.T, axes=(2, 0))
		cos_theta = numpy.dot(C, X.T)
		cos_theta[cos_theta > 1] = 1.
		cos_theta[cos_theta < -1] = -1.
		sin_theta = numpy.sqrt((W ** 2).sum(axis=1)) # norm
		theta = numpy.arccos(cos_theta)
		theta *= (1 -(sin_theta < 0) * 2)
		clusters = numpy.argmin(theta, axis=0)
		dist = numpy.min(theta, axis=0)
		return clusters, dist

	def argmin_euclidian_dist2(self, C, x):
		d = ((C - x) ** 2).sum(axis=1)
		ind = numpy.argmin(d)
		return ind, d[ind]

	def euclidian_mean(self, C, X):
		Xm = X.mean(axis=0)
		n = numpy.sqrt((Xm ** 2).sum()) # norm
		return Xm / n

	def euclidian_cluster(self, C, X):
		s1 = X.shape[0]
		s2 = C.shape[0]
		NX = numpy.reshape(numpy.sum(X * X, 1), (s1, 1))
		NC = numpy.reshape(numpy.sum(C * C, 1), (1, s2))
		D = numpy.repeat(NX, s2, 1)
		D = D + numpy.repeat(NC, s1, 0)
		D = D - 2 * numpy.dot(X, numpy.transpose(C))
		D = numpy.maximum(D, 0)
		clusters = D.argmin(axis=1)
		dist = D.min(axis=1)
		return clusters, dist


class AxialKmeans(SphericalKmeans):
	def __init__(self, *args, **kwargs):
		SphericalKmeans.__init__(self, *args, **kwargs)

	def compute(self, eps=0.001, max_iter=10000, verbose=False):
		# centers : random init from data
		C = self.init_centers(verbose)
		return self.algo_riemannian(C, eps, max_iter, verbose)

	def riemannian_mean(self, C, X, eps=0.01):
		'''
    compute intrinsic riemmanian mean of rotations

    C : initialization of mean
		'''
		theta_mean = eps + 1.
		n = X.shape[0]
		while theta_mean > eps:
			W = numpy.cross(X, -C)
			cos_theta = numpy.dot(X, C)
			cos_theta[cos_theta > 1] = 1.
			cos_theta[cos_theta < -1] = -1.
			sin_theta = numpy.sqrt((W ** 2).sum(axis=1)) # norm
			theta = numpy.arccos(cos_theta)
			theta *= (1 -(sin_theta < 0) * 2)
			big = (theta > PI_div_2)
			theta[big] = numpy.pi - theta[big]
			z = (sin_theta == 0)
			W[z] = 0
			sin_theta[z] = 1.
			W = (W.T * theta / sin_theta).T
			w = W.mean(axis=0)
			theta_mean = numpy.sqrt((w ** 2).sum()) # norm
			C = numpy.dot(numpy.asarray(rotation_from_vector(w)), C)
		return C

	def argmin_riemannian_dist2(self, C, x): #FIXME
		X = antisymetric_matrix_from_vector(x)
		W = -numpy.asarray((X * C.T).T)
		cos_theta = numpy.dot(C, x)
		cos_theta[cos_theta > 1] = 1.
		cos_theta[cos_theta < -1] = -1.
		sin_theta = numpy.sqrt((W ** 2).sum(axis=1)) # norm
		theta = numpy.arccos(cos_theta)
		theta *= (1 -(sin_theta < 0) * 2)
		big = (theta > PI_div_2)
		theta[big] = numpy.pi - theta[big]
		ind = numpy.argmin(theta)
		return ind, theta[ind]

	def riemannian_cluster(self, C, X): #FIXME
		S = [antisymetric_matrix_from_vector(c) for c in C]
		S = numpy.asarray(numpy.vstack(S))
		S.shape = (self._k, 3, 3)
		# cross product between all C and X
		W = numpy.tensordot(S, X.T, axes=(2, 0))
		cos_theta = numpy.dot(C, X.T)
		cos_theta[cos_theta > 1] = 1.
		cos_theta[cos_theta < -1] = -1.
		sin_theta = numpy.sqrt((W ** 2).sum(axis=1)) # norm
		theta = numpy.arccos(cos_theta)
		theta *= (1 -(sin_theta < 0) * 2)
		big = (theta > PI_div_2)
		theta[big] = numpy.pi - theta[big]
		clusters = numpy.argmin(theta, axis=0)
		dist = numpy.min(theta, axis=0)
		return clusters, dist

	def argmin_euclidian_dist2(self, C, x): #FIXME
		d1 = ((C - x) ** 2).sum(axis=1)
		d2 = ((C + x) ** 2).sum(axis=1)
		d = numpy.min([d1, d2], axis=0)
		ind = numpy.argmin(d)
		return ind, d[ind]

	def euclidian_cluster(self, C, X): #FIXME
		s1 = X.shape[0]
		s2 = C.shape[0]
		NX = numpy.reshape(numpy.sum(X * X, 1), (s1, 1))
		NC = numpy.reshape(numpy.sum(C * C, 1), (1, s2))
		D = numpy.repeat(NX, s2, 1)
		D = D + numpy.repeat(NC, s1, 0)
		D = D - 2 * numpy.dot(X, numpy.transpose(C))
		D2 = D + 2 * numpy.dot(X, numpy.transpose(C))
		D = numpy.maximum(D, 0)
		D2 = numpy.maximum(D2, 0)
		D = numpy.min([D, D2], axis=0)
		clusters = D.argmin(axis=1)
		dist = D.min(axis=1)
		return clusters, dist




################################################################################
class Registration(object):
	def __init__(self, verbose=0):
		self._verbose = verbose
		self._t = numpy.asmatrix(numpy.zeros(3)).T
		self._R = numpy.asmatrix(numpy.identity(3))

	def dJ_dw_0(self):
		'''
	    Return list of derivatives according to w_i taken at w = [0, 0, 0] :

		    dJ(w)   |
	    Di = ---------- |
		    dw_i    |w = 0

	    return [D0, D1, D2] where each D_i is a matrix
		'''
		return J_i


	def dJ_dw_w(self, w):
		'''
	    w : vector representation of an antisymetric matrix

	    list of derivatives according to w_i with w = [w_i]_i taken in w :

		    dJ(w)   |
	    Di = ---------- |
		    dw_i    |w = w

	    return [D0, D1, D2] where each D_i is a matrix
		'''
		l = []
		J = antisymetric_matrix_from_vector(w)
		J2 = J * J
		theta = numpy.sqrt(-numpy.trace(J2) / 2.)
		theta2 = theta ** 2
		theta3 = theta2 * theta
		theta4 = theta2 * theta2
		st, ct = numpy.sin(theta), numpy.cos(theta)
		if theta:
			f = st / theta
			g = (1. - ct) / (theta2)
			u = (ct / theta2) - (st / theta3) # fp/theta
			v = (st / theta3) - 2. * (1. - ct) / theta4 # gp/theta
		else:	f, g, u, v = 1, 0.5, - 1./3, -1./12
		w = vector_from_antisymetric_matrix(J)
		G = u * J + v * J2
		for i in range(3):
			Ji = J_i[i]
			wi = w[i, 0]
			JiJ = Ji * J
			D_i = wi * G + f * Ji + g * (JiJ + JiJ.T)
			l.append(D_i)
		return l

	def d2J_dw_0(self):
		'''
	    list of derivatives according to w_i taken at w = [0, 0, 0] :

		     d^2J(w)     |
	    Di = --------------- |
		   dw_i, dw_j    |w = 0

	    return a dictionnary {(i, j) : D_ij} where each D_ij is a matrix
		'''
		h = {}
		for i in range(3):
			Ji = J_i[i]
			for j in range(3):
				if i < j: continue
				Jj = J_i[j]
				JiJj = Ji * Jj
				h[i, j] = h[j, i] = (JiJj + JiJj.T) / 2.
		return h

	def d2J_dw_w(self, w):
		'''
	    w : vector representation of an antisymetric matrix

	    list of derivatives according to w_i with w = [w_i]_i taken in w :

		     d^2J(w)     |
	    Di = --------------- |
		   dw_i, dw_j    |w = w

	    return a dictionnary {(i, j) : D_ij} where each D_ij is a matrix
		'''
		h = {}
		J = antisymetric_matrix_from_vector(w)
		J2 = J * J
		w = vector_from_antisymetric_matrix(J)
		theta = numpy.sqrt(-numpy.trace(J2) / 2.)
		theta2 = theta ** 2
		theta3 = theta2 * theta
		theta4 = theta2 * theta2
		theta5 = theta2 * theta3
		theta6 = theta3 * theta3
		st, ct = numpy.sin(theta), numpy.cos(theta)
		if theta:
			f = st / theta
			g = (1. - ct) / (theta2)
			u = (ct / theta2) - (st / theta3) # fp/theta
			v = (st / theta3) - 2. * (1. - ct) / theta4 # gp/theta
			upt = - (st / theta3) + 3 * ((st / theta5) - \
				(ct / theta4))
			vpt = (ct / theta4) - (5 * st / theta5) + \
				8 * (1 - ct) / theta6
		else:	f, g, u, v, upt, vpt = \
				1, 0.5, - 1./3, -1./12, 1./15, 1./90
		uptJvptJ2 = upt * J + vpt * J2
		
		for i in range(3):
			wi, Ji = w[i, 0], J_i[i]
			JiJ = Ji * J
			JiJ2 = (JiJ + JiJ.T)
			vJiJ2 = v * JiJ2
			uJi = u * Ji
			for j in range(3):
				if i < j:continue
				wj, Jj = w[j, 0], J_i[j]
				JiJj = Ji * Jj
				JiJj2 = JiJj + JiJj.T
				JjJ = Jj * J
				JjJ2 = (JjJ + JjJ.T)
				M = wi * wj * uptJvptJ2 + g * JiJj2 + \
					wi * (u * Jj + v * JjJ2) + \
					wj * (uJi + vJiJ2)
				h[i, j] = h[j, i] = M
		return h

	def getCurrentEnergy(self):
		return self._energy

	def set_initialization(self, R, t):
		self._R = R
		self._t = t

	def optimize(self, eps=10e-5, user_func=(lambda self,x : None),
			user_data=None, mode='riemannian', affine=False):
		opt = [eps, user_func, user_data]
		if affine == True and mode != 'powell':
			raise ValueError("Affine registration works " + \
					"only with powell optimization")
		if mode == 'riemannian':
			return self.optimize_riemannian(*opt)
		elif mode == 'powell':
			if affine:
				return self.optimize_affine_powell(*opt)
			else:	return self.optimize_rigid_powell(*opt)
		elif mode == 'rotation_vector':
			return self.optimize_vector_rotation(*opt)
		else:	raise ValueError("unknown mode '%s'" % mode)

	def optimize_rotation_vector(self, eps=10e-5,
		user_func=(lambda self,x : None), user_data=None):
		'''
    Find best rigid transformation (R, t) with Newton-Raphson optimization
    scheme in SE(3) Riemannian Tangent space of Identity matrix.
		'''
		n = 0
		self._w = numpy.asmatrix(numpy.zeros((3, 1)))
		old_energy = self.energy()
		if self._verbose > 0: print 'init en = ', old_energy
		R0 = numpy.identity(3)
		while 1:
			n += 1
			grad, H = self.grad_hessian(self._w, R0)

			dg = - H.I * grad
			if (dg.T * grad)[0, 0] >= 0: dg = -grad
			old_R = self._R
			old_t = self._t
			old_w = self._w
			while 1:
				dgw, dgt = dg[:3], dg[3:]
				theta = numpy.sqrt(dgw.T * dgw)[0, 0]
				if theta:
					k = long(theta / (2 * numpy.pi))
					if (theta - 2. * k * numpy.pi) > \
								numpy.pi:
						k += 1
					dgw *= (1. - 2. * k * numpy.pi / theta)
				self._w = old_w + dgw
				self._R = rotation_from_vector(self._w)
				self._t = old_t + dgt
				new_energy = self.energy()
				if new_energy > old_energy:
					dg /= 2.
					if new_energy - old_energy < eps: break
				else: break
			if self._verbose > 0:
				print "%d) en = %f " % (n, new_energy)
			self._energy = new_energy
			user_func(self, user_data)
			if old_energy - new_energy < eps: break
			else:	old_energy = new_energy
		return self._R, self._t


	def optimize_riemannian(self, eps=10e-5,
		user_func=(lambda self,x : None), user_data=None):
		'''
    Find best rigid transformation (R, t) with Newton-Raphson optimization
    scheme in SE(3) Riemannian Tangent space of local Rotation matrix.
		'''
		n = 0
		z = numpy.asmatrix(numpy.zeros((3, 1)))
		old_energy = self.energy()
		end = False
		while not end:
			n += 1
			grad, H = self.grad_hessian(z, self._R)
			dg = - H.I * grad
			if (dg.T * grad)[0, 0] >= 0: dg = -grad
			norm = numpy.linalg.norm(dg)
			if norm > 10: dg = dg * 10. / norm
			old_R = self._R
			old_t = self._t
			while 1:
				dgw, dgt = dg[:3], dg[3:]
				theta = numpy.sqrt(dgw.T * dgw)[0, 0]
				if theta:
					k = long(theta / (2 * numpy.pi))
					if (theta - 2. * k * numpy.pi) \
							> numpy.pi:
						k += 1
					dgw *= (1. - 2. * k * numpy.pi / theta)
				J = antisymetric_matrix_from_vector(dgw)
				self._R = old_R * \
					rotation_from_antisymetric_matrix(J)
				self._t = old_t + dgt
				new_energy = self.energy()
				if new_energy < 0: dg /= 2.
				if new_energy > old_energy:
					dg /= 2.
					if new_energy - old_energy < eps:
						end = True
						break
				else: break
			if self._verbose > 0:
				print "%d) en = %f " % (n, new_energy)
			self._w = vector_from_rotation(self._R)
			self._energy = new_energy
			user_func(self, user_data)
			if old_energy - new_energy < eps: end = True
			else:	old_energy = new_energy
		return self._R, self._t

	def optimize_rigid_powell(self, eps=10e-5,
		user_func=(lambda self,x : None), user_data=None):
		self._n = 0
		def func(x, self, user_func, user_data):
			self._w = numpy.asmatrix(x[:3]).T
			theta = numpy.sqrt(self._w.T * self._w)[0, 0]
			if theta:
				k = long(theta / (2 * numpy.pi))
				if (theta - 2. * k * numpy.pi) > numpy.pi:
					k += 1
				self._w *= (1. - 2. * k * numpy.pi / theta)
			self._t = numpy.asmatrix(x[3:]).T
			self._R = rotation_from_vector(self._w)
			if not (self._n % 100):
				user_func(self, user_data)
			self._energy = self.energy()
			if self._verbose > 0:
				print "powell, en = %f " % (self._energy)
			self._n += 1
			return self._energy
		w = vector_from_rotation(self._R)
		x0 = numpy.hstack([numpy.asarray(w).ravel(),
				numpy.asarray(self._t).ravel()])
		import scipy.optimize
		res = scipy.optimize.fmin_powell(func, x0,
			args=(self, user_func, user_data), disp=0, ftol=eps)
		func(res, self, user_func, user_data)
		return self._R, self._t

	def optimize_affine_powell(self, eps=10e-5,
		user_func=(lambda self,x : None), user_data=None):
		self._n = 0
		def get_rot(w):
			theta = numpy.sqrt(w.T * w)[0, 0]
			if theta:
				k = long(theta / (2 * numpy.pi))
				if (theta - 2. * k * numpy.pi) > numpy.pi:
					k += 1
				w *= (1. - 2. * k * numpy.pi / theta)
			return rotation_from_vector(w)


		def func(x, self, user_func, user_data):
			wu = numpy.asmatrix(x[:3]).T
			D = x[3:6]
			if numpy.any(D <= 0):
				# hack to create a gradient
				self._energy = -numpy.min(D) + 1000000.
				return self._energy
			D2 = numpy.asmatrix(numpy.diag(D))
			wv = numpy.asmatrix(x[6:9]).T
			self._t = numpy.asmatrix(x[9:12]).T
			U = get_rot(wu)
			V = get_rot(wv)
			self._D = D
			self._R = U * D2 * V
			if not (self._n % 100):
				user_func(self, user_data)
			self._energy = self.energy()
			if self._verbose > 0:
				print "powell, en = %f " % (self._energy)
			self._n += 1
			return self._energy
		U, D, V = numpy.linalg.svd(self._R) # R : affine matrix here
		wu = numpy.asarray(vector_from_rotation(U)).ravel()
		D = numpy.asarray(D).ravel()
		wv = numpy.asarray(vector_from_rotation(V)).ravel()
		x0 = numpy.hstack([wu, D.ravel(), wv,
				numpy.asarray(self._t).ravel()])
		import scipy.optimize
		res = scipy.optimize.fmin_powell(func, x0,
			args=(self, user_func, user_data), disp=0, ftol=eps)
		func(res, self, user_func, user_data)
		return self._R, self._t
	



################################################################################
class Procrust(Registration):
	def __init__(self, X, Y, verbose=0):
		Registration.__init__(self, verbose)
		self._X = numpy.asmatrix(X)
		self._Y = numpy.asmatrix(Y)

	def energy(self):
		n = self._X.shape[1]
		P = self._Y - self._R * self._X - self._t * numpy.ones(n).T
		e = 0.5 * numpy.trace(P.T * P)
		return e

	def optimize(self):
		X, Y = self._X, self._Y
		xm = self._X.mean(axis=1)
		ym = self._Y.mean(axis=1)
		U, D, Vt = numpy.linalg.svd((X-xm) * (Y-ym).T)
		self._R = Vt.T * U.T
		self._t = ym - self._R * xm
		return self._R, self._t
		

class ProcrustMetric(Registration):
	def __init__(self, X, Y, A, verbose=0):
		Registration.__init__(self, verbose)
		self._X = numpy.asmatrix(X)
		self._Y = numpy.asmatrix(Y)
		self._A = numpy.asmatrix(A)
		self._xm = self._X.mean(axis=1)
		self._ym = self._Y.mean(axis=1)
		self._t = self._ym - self._xm
		self._R = numpy.asmatrix(numpy.identity(3))
		self._size = self._X.shape[1]

	def energy(self):
		n = self._X.shape[1]
		P = self._Y - self._R * self._X - self._t * numpy.ones(n).T
		e = 0.5 * numpy.trace(P.T * self._A * P)
		return e

	def grad_hessian(self, w, R0):
		T = self._t * numpy.ones(self._size).T
		D = self._X * (self._R * self._X + \
			T - self._Y).T * self._A * R0

		# derivatives
		if numpy.all(w == 0):
			dJ = self.dJ_dw_0()
			d2J = self.d2J_dw_0()
		else:
			dJ = self.dJ_dw_w(w)
			d2J = self.d2J_dw_w(w)

		# gradient
		dt = self._size * self._A * (self._t - self._ym + \
					self._R * self._xm)

		dw = numpy.matrix(numpy.zeros((3, 1)))
		for i in range(3):
			dw[i] = numpy.trace(D * dJ[i])
		grad = numpy.vstack([dw, dt])
		
		# hessian
		H = numpy.matrix(numpy.zeros((6, 6)))
		for i in range(3):
			RdJiXt = (R0 * dJ[i] * self._X).T
			for j in range(3):
				if i < j: continue
				RdJjX = R0 * dJ[j] * self._X
				t1 = numpy.trace(RdJiXt *self._A*RdJjX)
				t2 = numpy.trace(D * d2J[i, j])
				H[i, j] = H[j, i] = t1 + t2
		for i in range(3):
			v = self._size * self._A * R0 * \
						dJ[i] * self._xm
			H[i, 3:] = v.T
			H[3:, i] = v
		H[3:, 3:] = self._size * self._A

		return grad, H


class ProcrustMetricField(Registration):
	'''
	Find R, t minimizing the following energy :

                  ____                             2
	E = 0.5 * \    w_il * ||y_l - R * x_i - t||
                  /___                             A_l
                   l,i
                  ____                              2
          = 0.5 * \    || (Y_l - R X - T) W_l^0.5 ||     avec T = t * ones.T
                  /___ ||                         ||A_l      W_l = diag({W_li}i)
                    l
	'''
	def __init__(self, X, weights, gaussian_mixture, verbose=0):
		Registration.__init__(self, verbose)
		centers = gaussian_mixture.get_means()
		metrics = gaussian_mixture.get_metrics()
		self._X = numpy.asmatrix(X)
		self._weights = numpy.asmatrix(weights)
		self._centers = [numpy.asmatrix(c).T for c in centers]
		self._metrics = [numpy.asmatrix(c) for c in metrics]
		if len(centers) != len(metrics):
			raise ValueError("number of centers/metrics" + \
					"mismatch.")
		if weights.shape[0] != len(centers):
			raise ValueError("number of weights/centers " + \
					"mismatch.")
		if weights.shape[1] != X.shape[1]:
			raise ValueError("number of weights/data " + \
					"mismatch.")
		self._size = len(centers)
		self._cum_weights = numpy.asarray(\
				self._weights.sum(axis=1)).ravel()
		self._t = numpy.asmatrix(numpy.zeros(3)).T
		self._R = numpy.asmatrix(numpy.identity(3))

	def energy(self):
		n = self._X.shape[1]
		e = 0.
		for l in range(self._size):
			A, p = self._metrics[l], self._weights[l]
			y = self._centers[l]
			spt = numpy.sqrt(p)
			t1 = numpy.dot(y - self._t, spt)
			t2 = numpy.multiply(self._R * self._X, spt)
			P = t1 - t2
			e += numpy.trace(P.T * A * P)
		return 0.5 * e

	def grad_hessian(self, w, R0):
		zeros_vec = numpy.matrix(numpy.zeros((3, 1)))	

		# derivatives
		if numpy.all(w == 0):
			dJ = self.dJ_dw_0()
			d2J = self.d2J_dw_0()
		else:
			dJ = self.dJ_dw_w(w)
			d2J = self.d2J_dw_w(w)

		# gradient
		dt = zeros_vec.copy()
		R, t, X = self._R, self._t, self._X
		D = numpy.asmatrix(numpy.zeros_like(X.T))
		RX = R * X
		for l in range(self._size):
			A, p = self._metrics[l], self._weights[l]
			y, cum = self._centers[l], self._cum_weights[l]
			Aty = A * (t - y)
			ARX = A * RX
			dt += cum * Aty + ARX * p.T
			Atyp = Aty * p
			D += ((numpy.multiply(ARX, p) + Atyp)).T
		D = X * D * R0
		dw = zeros_vec.copy()
		for i in range(3): dw[i] = numpy.trace(D * dJ[i])
		grad = numpy.vstack([dw, dt])

		# hessian
		H = numpy.matrix(numpy.zeros((6, 6)))

		RdJiX = []
		for i in range(3): RdJiX.append(R0 * dJ[i] * X)
		for l in range(self._size):
			A, p = self._metrics[l], self._weights[l]
			spt = numpy.sqrt(p)
			for i in range(3):
				Kijt = numpy.multiply(RdJiX[i], spt).T
				for j in range(3):
					if i < j: continue
					Kji = numpy.multiply(RdJiX[j], spt)
					val = numpy.trace(Kijt * A * Kji)
					H[i, j] += val
					H[j, i] += val

		for i in range(3):
			for j in range(3):
				if i < j: continue
				val = numpy.trace(D * d2J[i, j])
				H[i, j] += val
				H[j, i] += val
	
		for i in range(3):
			v = zeros_vec.copy()
			for l in range(self._size):
				A, p = self._metrics[l], self._weights[l]
				v += A * RdJiX[i] * p.T
			H[i, 3:] = v.T
			H[3:, i] = v
		for l in range(self._size):
			cum, A = self._cum_weights[l], self._metrics[l]
			H[3:, 3:] += cum * A

		return grad, H


class MixtureGlobalRegistration(Registration):
	'''
    Generic Global registration between a mixture of distribution and a set
    of realisation from this distribution move with unknown transformation R, t.

    X :          data : numpy array/matrix
    mixture :    MixtureModel
    groups :     numpy.array/list given labels to data in X to group them in
                 likelihoods computation.
	'''
	_algo_map = {
		distribution.GaussianMixtureModel : ProcrustMetricField,
	}
	def __init__(self, X, mixture, groups=None, available_labels=None,
		is_affine=False, verbose=0):
		Registration.__init__(self, verbose)
		self._X = numpy.asmatrix(X)
		self._mixture = mixture
		self._groups = groups
		self._available_labels = available_labels
		self._is_affine = is_affine
		n, self._size = self._X.shape[1], len(self._mixture)
		self._weights = numpy.ones((self._size, n))
		self._R = numpy.asmatrix(numpy.identity(3))
		self._t = numpy.asmatrix(numpy.zeros(3)).T

	def get_registration_algo(self):
		return MixtureGlobalRegistration._algo_map[\
				self._mixture.__class__]

	def posteriors(self, X2):
		if self._groups is not None:
			weights, loglikelihoods, likelihoods = \
				self._mixture.posteriors_groups(X2.T,
				self._groups, self._available_labels)
		else:
			weights, loglikelihoods, likelihoods = \
				self._mixture.posteriors(X2.T,
				self._available_labels)
		return weights.T, loglikelihoods.T, likelihoods.T


	def optimize(self, eps=10e-5, maxiter=numpy.inf,
			user_func=(lambda self,x : None),
			user_data=None, mode='riemannian', affine=False):
		'''
    mode : riemannian, powell, rotation_vector
		'''
		X, weights = self._X, self._weights
		old_energy = numpy.inf
		X2 = X
		n = 0
		while 1:
			if n >= maxiter: break
			if self._verbose > 0: print "**** %d *****" % n
			weights, loglikelihoods, likelihoods = \
						self.posteriors(X2)
			Algo = self.get_registration_algo()
			if self._groups is not None:
				algo = Algo(X, weights, self._mixture,
					self._groups, self._is_affine,
					verbose=self._verbose - 1)
			else:	algo = Algo(X, weights, self._mixture,
					verbose=self._verbose - 1)
			algo.set_initialization(self._R, self._t)
			self._R, self._t = algo.optimize(eps,
				user_func, user_data, mode, affine)
			new_energy = algo.getCurrentEnergy()
			if self._verbose > 0:
				print "mixture registration : en = ", new_energy
			n += 1
			if old_energy - new_energy < eps: break
			else:	old_energy = new_energy
			X2 = self._R * X + self._t
		weights, loglikelihoods, likelihoods = self.posteriors(X2)
		trans = RigidTransformation(self._R, self._t)
		return trans, weights, loglikelihoods, likelihoods


class MixtureLocalRegistration(Registration):
	'''
    Generic Local registration between a mixture of distribution and a set
    of realisation from this distribution move with unknown transformation R, t.

    X :          data : numpy array/matrix
    mixture :    MixtureModel
    groups :     numpy.array/list given labels to data in X to group them in
                 likelihoods computation.
	'''
	_algo_map = {}
	def __init__(self, X, gravity_centers, mixture, translation_prior=None,
		direction_prior=None, angle_prior=None,
		groups=None, available_labels=None, verbose=0):
		Registration.__init__(self, verbose)
		self._X = numpy.asmatrix(X)
		self._gravity_centers = gravity_centers
		self._mixture = mixture
		if translation_prior is None and \
			direction_prior is None and\
			angle_prior is None:
			self._rotation_priors = None
		else:	self._rotation_priors = translation_prior, \
					direction_prior, angle_prior
		self._groups = groups
		self._available_labels = available_labels
		n, self._size = self._X.shape[1], len(self._mixture)
		self._weights = numpy.ones((self._size, n))
		R = numpy.asmatrix(numpy.identity(3))
		t = numpy.asmatrix(numpy.zeros(3)).T
		self._trans = [(R.copy(), t.copy()) for i in range(self._size)]

	def get_registration_algo(self):
		return MixtureLocalRegistration._algo_map[\
				self._mixture.__class__]

	def _posteriors(self, X2):
		if self._groups is not None:
			posteriors, loglikelihoods, likelihoods = \
				self._mixture.posteriors_groups(X2.T,
				self._groups, self._available_labels)
		else:
			posteriors, loglikelihoods, likelihoods = \
				self._mixture.posteriors(X2.T,
				self._available_labels)
		return posteriors.T, loglikelihoods.T, likelihoods.T


	def optimize(self, eps=10e-5, maxiter=numpy.inf,
			user_func=(lambda self,x : None),
			user_data=None, mode='riemannian', affine=False):
		'''
    mode : riemannian, powell, rotation_vector
		'''
		X, weights = self._X, self._weights
		old_energy = numpy.inf
		n = 0
		priors = self._mixture.get_priors()
		posteriors, loglikelihoods, likelihoods = self._posteriors(X)
		L = self._available_labels
		while 1:
			if self._verbose > 0: print "**** %d *****" % n
			loglikelihoods, likelihoods = [], []
			new_energy = 0.
			for i, spam in enumerate(self._mixture.get_models()):
				if L is not None:
					L_i = L[:, i]
				else:	L_i = None
				g = self._gravity_centers[i]
				Algo = self.get_registration_algo()
				if self._groups is not None:
					algo = Algo(spam, g, X, posteriors[i],
						self._groups,
						verbose=self._verbose - 2)
				else:	algo = Algo(spam, g, X, posteriors[i],
						verbose=self._verbose - 2)
				if self._rotation_priors:
					t_prior = self._rotation_priors[0][i]
					d_prior = self._rotation_priors[1][i]
					a_prior = self._rotation_priors[2][i]
					algo.setPriors(t_prior, \
						d_prior, a_prior)
				R, t = self._trans[i]
				algo.set_initialization(R, t)
				R, t = algo.optimize(eps,
					user_func, user_data, mode, affine)
				local_en = algo.getCurrentEnergy()
				new_energy += local_en
				if self._verbose > 1:
					print "local en = ", local_en
				self._trans[i] = R, t
				Xi = R * (X - g) + (t + g)
				logli, li = spam.likelihoods_groups(Xi.T,
							self._groups, L_i)
				loglikelihoods.append(logli)
				likelihoods.append(li)
			loglikelihoods = numpy.vstack(loglikelihoods).T
			likelihoods = numpy.vstack(likelihoods).T
			posteriors = numpy.multiply(likelihoods, priors)
			posteriors = numpy.divide(posteriors.T,
					posteriors.sum(axis=1))

			if self._verbose > 0:
				print "local registrations : en = ", new_energy
			n += 1
			if n >= maxiter: break
			if old_energy - new_energy < eps: break
			else:	old_energy = new_energy
		trans2 = []
		for i, (R, t) in enumerate(self._trans):
			g = self._gravity_centers[i]
			t2 = t + g - R * g
			trans2.append((R, t2))
		trans = LocalRigidTransformations(trans2)
		return trans, posteriors, loglikelihoods.T, likelihoods.T
