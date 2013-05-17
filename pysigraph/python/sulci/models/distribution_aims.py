#!/usr/bin/env python

import pickle, numpy, scipy.stats, scipy.special
from soma import aims, aimsalgo
import distribution
from distribution import Distribution, MixtureModel, distribution_map

################################################################################
# SPAM
################################################################################
inf = numpy.inf

class PySpam(Distribution):
	_converter = aims.Converter_BucketMap_FLOAT_AimsData_FLOAT()
	_resampler = aimsalgo.LinearResampler_FLOAT()
	_neighbourhood = numpy.array([
		[-1, -1, -1],
		[-1, -1, 0],
		[-1, -1, 1],
		[-1, 0, -1],
		[-1, 0, 0],
		[-1, 0, 1],
		[-1, 1, -1],
		[-1, 1, 0],
		[-1, 1, 1],
		[0, -1, -1],
		[0, -1, 0],
		[0, -1, 1],
		[0, 0, -1],
		[0, 0, 0],
		[0, 0, 1],
		[0, 1, -1],
		[0, 1, 0],
		[0, 1, 1],
		[1, -1, -1],
		[1, -1, 0],
		[1, -1, 1],
		[1, 0, -1],
		[1, 0, 0],
		[1, 0, 1],
		[1, 1, -1],
		[1, 1, 0],
		[1, 1, 1]]
	)

	def __init__(self, gaussian_std=2, fromlog=False):
		Distribution.__init__(self)
		self._name = 'spam'
		self._gaussian_std = gaussian_std
		self._voxels_n = 0
		self._nodes_n = 0
		self._bb_talairach_offset = None
		self._bb_talairach_size = None
		self._img_density = None
		self._fromlog = fromlog

	def is_fromlog(self): return self._fromlog

	def _bucketmaps_to_bucketmap_bb(self, bucketmaps, weights=None):
		'''
    Compute a bounding box from a list of bucketmaps and return translated
    coordinates of all buckets in this bounding box.

    Warngin : all bucketmaps must have same size of voxels and must not
    overlappe to each other.
		'''
		xmin, ymin, zmin = inf, inf, inf
		xmax, ymax, zmax = -inf, -inf, -inf
		bucket_all = aims.BucketMap_FLOAT.Bucket()
		bucket_out = aims.BucketMap_FLOAT.Bucket()

		# For each bucketmap
		for i, bm in enumerate(bucketmaps):
			size = bm.sizeX(), bm.sizeY(), bm.sizeZ()
			bucket_in = bm[0]
			if weights:
				w = weights[i]
			else:	w = 1.
			for p in bucket_in.keys():
				xmax = max(xmax, p[0])
				ymax = max(ymax, p[1])
				zmax = max(zmax, p[2])
				xmin = min(xmin, p[0])
				ymin = min(ymin, p[1])
				zmin = min(zmin, p[2])
				bucket_all[p] = w

		# compute bounding box
		xd, yd, zd = (xmax - xmin + 1), (ymax - ymin + 1), \
						(zmax - zmin + 1)
		bb_min_coord = bb_offset = xmin, ymin, zmin
		bb_max_coord = xmax, ymax, zmax
		bb_size = xd, yd, zd
		bb_size = [numpy.ceil(s) for s in bb_size]

		# compute bucketmap in bounding box axis
		for b in bucket_all.keys():
			bucket_out[b - bb_offset] = bucket_all[b]
		bucketmap_out = aims.BucketMap_FLOAT()
		bucketmap_out[0] = bucket_out
		bucketmap_out.setSizeX(size[0])
		bucketmap_out.setSizeY(size[1])
		bucketmap_out.setSizeZ(size[2])
		
		return bb_min_coord, bb_max_coord, bb_size, bucketmap_out
	_bucketmaps_to_bucketmap_bb = classmethod(_bucketmaps_to_bucketmap_bb)

	def _bucketmap_bb_to_img_bb_with_motion(cls, bucketmap, motion,
							bb_in, bb_out):
		bb_offset_in, bb_size_in = bb_in
		bb_offset_out, bb_size_out = bb_out

		# we must use voxel size on t1, because output space is isotrope
		bm = bucketmap
		size_in = numpy.array([bm.sizeX(), bm.sizeY(), bm.sizeZ()])

		# transformation :
		t1 = aims.Point3df(numpy.array(bb_offset_in) * size_in)
		t2 = aims.Point3df(-numpy.array(bb_offset_out))
		motion_r = motion
		motion_t1 = aims.Motion()
		motion_t1.setTranslation(t1)
		motion_t2 = aims.Motion()
		motion_t2.setTranslation(t2)
		motion_t = motion_t2 * motion_r * motion_t1

		# buket -> img avec translation
		img = aims.AimsData_FLOAT(*bb_size_in)
		cls._converter.convert(bucketmap, img)
		img_out = aims.AimsData_FLOAT(*bb_size_out)
		# input (anisotropic voxels)
		# -> output (isotropic voxels) in motion space
		cls._resampler.resample(img, motion_t, 0, img_out, False)	
		
		return img_out
	_bucketmap_bb_to_img_bb_with_motion = \
			classmethod(_bucketmap_bb_to_img_bb_with_motion)

	def _compute_corners_with_motion(cls, motion, size_in,
			(xmin, ymin, zmin), (xmax, ymax, zmax)):
		# input bounding box
		corners = [
			(xmin, ymin, zmin),
			(xmax, ymin, zmin),
			(xmin, ymax, zmin),
			(xmin, ymin, zmax),
			(xmin, ymax, zmax),
			(xmax, ymax, zmin),
			(xmax, ymin, zmin),
			(xmax, ymax, zmax)]
		corners_input = [aims.Point3df(c) for c in corners]
		motion_r = motion
		motion_s = aims.Motion()
		motion_s.scale(size_in, [1] * 3)
		motion_t = motion_r * motion_s
		corners_output = [motion_t.transform(c) for c in corners_input]

		# output bounding box estimation
		xmin, ymin, zmin = inf, inf, inf
		xmax, ymax, zmax = -inf, -inf, -inf
		for p in corners_output:
			xmax = max(xmax, p[0])
			ymax = max(ymax, p[1])
			zmax = max(zmax, p[2])
			xmin = min(xmin, p[0])
			ymin = min(ymin, p[1])
			zmin = min(zmin, p[2])
		offset_output1 = xmin, ymin, zmin
		offset_output2 = xmax, ymax, zmax
		return (offset_output1, offset_output2)
	_compute_corners_with_motion = classmethod(_compute_corners_with_motion)

	def fit_graphs(self, infos=None, motions=None, ss=True,
					write_count=None):
		self._infos = infos
		self._motions = motions
		self._ss = ss
		offsets = {}
		bucketmaps = {}
		bb_subjects = {}

		# 1) Bounding box estimations
		# For each subjects
		for (id, graphinfo) in self._infos.items():
			vertices = graphinfo['vertices']
			weights = graphinfo['weights']
			motion = self._motions[id]
			
			# compute subject bounding box
			if self._ss: bucket_name = 'aims_ss'
			else: bucket_name = 'aims_bottom'
			bucketmaps_in = [v[bucket_name].get() for v in vertices]
			self._nodes_n += len(vertices)
			n = numpy.sum([len(b[0].keys()) for b in bucketmaps_in])
			if n == 0:
				print "warning : in file " \
					"'%s', " % graphinfo['name'] + \
					"no voxels found !"
				continue
			self._voxels_n += n
			bucket_min, bucket_max, bb_subject_size, bucketmap = \
				self._bucketmaps_to_bucketmap_bb(bucketmaps_in,
								weights)	
			bb_subject_offset = bucket_min
			bucketmaps[id] = bucketmap
			bb_subjects[id] = bb_subject_offset, bb_subject_size
			bm = bucketmap
			size_in = numpy.array([bm.sizeX(), bm.sizeY(),
							bm.sizeZ()])
			offset_talairach1, offset_talairach2 = \
				self._compute_corners_with_motion(motion,
					size_in, bucket_min, bucket_max)
			offsets[id] = offset_talairach1, offset_talairach2

		# Compute talairach bounding box
		xmin, ymin, zmin = inf, inf, inf
		xmax, ymax, zmax = -inf, -inf, -inf
		for g, (ot1, ot2) in offsets.items():
			xmax = max(xmax, ot2[0])
			ymax = max(ymax, ot2[1])
			zmax = max(zmax, ot2[2])
			xmin = min(xmin, ot1[0])
			ymin = min(ymin, ot1[1])
			zmin = min(zmin, ot1[2])
		xd, yd, zd = (xmax - xmin + 1), (ymax - ymin + 1), \
						(zmax - zmin + 1)
		bb_talairach_offset = xmin, ymin, zmin
		bb_talairach_size = xd, yd, zd
		bb_talairach_size = [numpy.ceil(s) for s in bb_talairach_size]
		# add a 5 voxels width to bounding box to take into account
		# gaussian filter mask width.
		bb_talairach_offset = [(s - 5) for s in bb_talairach_offset]
		bb_talairach_size = [(s + 10) for s in bb_talairach_size]

		# 2) Transform data and count :
		img_count = aims.AimsData_FLOAT(*bb_talairach_size)
		img_count.fill(0.)
		# For each subjects
		for (id, graphinfo) in self._infos.items():
			motion = self._motions[id]
			# when a subject has no buckets
			if not bucketmaps.has_key(id): continue
			bucketmap = bucketmaps[id]
			bb_subject_offset, bb_subject_size =  bb_subjects[id]
			img_talairach=self._bucketmap_bb_to_img_bb_with_motion(\
				bucketmap, motion,
				(bb_subject_offset, bb_subject_size),
				(bb_talairach_offset, bb_talairach_size))
			img_count += img_talairach
		array = img_count.volume().get().arraydata()
		array /= array.sum()
		if write_count: aims.Writer().write(img_count, write_count)

		# gaussian blur
		d = self._gaussian_std
		smoothing = aimsalgo.Gaussian3DSmoothing_FLOAT(d, d, d)
		img_blur = smoothing.doit(img_count)
		array = img_blur.volume().get().arraydata()
		# apply correction if needed
		array[array < 0.] = 0.
		# avoid null values
		array += 10e-50
		# model external pdf energy
		self._missing_energy = 10e-50
		# normalize
		array *= (1 - self._missing_energy) / array.sum()
		if self._fromlog:
			z = (array == 0)
			notz = (z == 0)
			array[notz] = numpy.log(array[notz])
			array[z] = -100
		self._bb_talairach_offset = bb_talairach_offset
		self._bb_talairach_size = bb_talairach_size
		self._img_density = img_blur

	def fit(self, X, ss=True, write_count=None, weights=None):
		'''
    write_count : output filename to write image of sulci counting.
		'''
		self._ss = ss
		# ici on travaille directement sur un nuage de points
		# on peut simuler la reinterpolation de la facon suivante :
		# pour une position (non entiere) de point dans talairach :
		# p=(x, y, z). Pour la position entiere contenant le point et
		# tous ses voisins en 3D (de pos q_i): on calcul un pourcentage
		# d'appartenance : wi = (1-|ci_x|) * (1-|ci_y|) * (1-|ci_z|)
		# avec ci = (q_i - p). On a sum_ci wi = 1.

		# add one more voxel for interpolation + 0.5 shift
		xmin, ymin, zmin = X.min(axis=0) - 1.5
		xmax, ymax, zmax = X.max(axis=0) + 1.5
		xd, yd, zd = (xmax - xmin + 1), (ymax - ymin + 1), \
						(zmax - zmin + 1)
		bb_talairach_offset = xmin, ymin, zmin
		bb_talairach_size = xd, yd, zd
		bb_talairach_size = [numpy.ceil(s) for s in bb_talairach_size]
		# add a 5 voxels width to bounding box to take into account
		# gaussian filter mask width.
		bb_talairach_offset = [(s - 5) for s in bb_talairach_offset]
		bb_talairach_size = [(s + 10) for s in bb_talairach_size]

		img_count = aims.AimsData_FLOAT(*bb_talairach_size)
		img_count.fill(0.)
		array = img_count.volume().get().arraydata()[0] # first timestep
		# invert positions of X, Y et Z (data representation python/C)
		X = (X - numpy.array(bb_talairach_offset)).T[::-1].T
		for i, pos in enumerate(X):
			if weights is not None:
				weight = weights[i]
			else:	weight = 1.
			# integer position are at voxel center
			int_pos = (pos + 0.5).astype('int')
			for v in PySpam._neighbourhood:
				p2 = int_pos + v
				box = 1. - abs(p2 - pos)
				if (box > 0).all():
					w = numpy.prod(box)
					array[tuple(p2)] += w * weight
		array /= array.sum()
		if write_count: aims.Writer().write(img_count, write_count)

		# gaussian blur
		d = self._gaussian_std
		smoothing = aimsalgo.Gaussian3DSmoothing_FLOAT(d, d, d)
		img_blur = smoothing.doit(img_count)
		array = img_blur.volume().get().arraydata()
		# apply correction if needed
		array[array < 0.] = 0.
		# avoid null values
		array += 10e-50
		# model external pdf energy
		self._missing_energy = 10e-50
		# normalize
		array *= (1 - self._missing_energy) / array.sum()
		if self._fromlog:
			z = (array == 0)
			notz = (z == 0)
			array[notz] = numpy.log(array[notz])
			array[z] = -100
		self._bb_talairach_offset = bb_talairach_offset
		self._bb_talairach_size = bb_talairach_size
		self._img_density = img_blur

	def fit_knn(self, X, k=100, ss=True, write_count=None, weights=None):
		'''
    write_count : output filename to write image of sulci counting.
		'''
		self._ss = ss
		# ici on travaille directement sur un nuage de points
		# on peut simuler la reinterpolation de la facon suivante :
		# pour une position (non entiere) de point dans talairach :
		# p=(x, y, z). Pour la position entiere contenant le point et
		# tous ses voisins en 3D (de pos q_i): on calcul un pourcentage
		# d'appartenance : wi = (1-|ci_x|) * (1-|ci_y|) * (1-|ci_z|)
		# avec ci = (q_i - p). On a sum_ci wi = 1.

		# add one more voxel for interpolation + 0.5 shift
		xmin, ymin, zmin = X.min(axis=0) - 1.5
		xmax, ymax, zmax = X.max(axis=0) + 1.5
		xd, yd, zd = (xmax - xmin + 1), (ymax - ymin + 1), \
						(zmax - zmin + 1)
		bb_talairach_offset = xmin, ymin, zmin
		bb_talairach_size = xd, yd, zd
		bb_talairach_size = [numpy.ceil(s) for s in bb_talairach_size]
		# add 5 voxels width to bounding box to take into account
		# gaussian filter mask width.
		bb_talairach_offset = [(s - 5) for s in bb_talairach_offset]
		bb_talairach_size = [(s + 10) for s in bb_talairach_size]

		img_count = aims.AimsData_FLOAT(*bb_talairach_size)

		# with scipy
		import scipy.spatial
		img_count.fill(0.)
		kd = scipy.spatial.cKDTree(X)

		from numpy.lib import index_tricks
		for (x, y) in index_tricks.ndindex(*bb_talairach_size[:2]):
			# all z in one pass
			v = numpy.array([x, y, 0]) + bb_talairach_offset
			Z = range(img_count.dimZ())
			V = v[None].repeat(img_count.dimZ(), 0)
			V[:, 2] += Z
			D, ID = kd.query(V, k)
			for i in range(len(D)):
				d, id, z, v = D[i], ID[i], Z[i], V[i]
				h = d[-1]
				if (h == 0): h = 10e-10
				# knn
				#img_count.setValue(k / (h**3), x, y, z)
				#diff = v - X[id]
				diff = v - X
				dist2 = (diff ** 2).sum(axis=1)
				val = numpy.exp(-0.5*dist2/(h**2)).sum()
				img_count.setValue(val / (h**3), x, y, z)

		# with aims
		#X = (X - numpy.array(bb_talairach_offset)).T[::-1].T
		#if not X.flags['C_CONTIGUOUS']: X = X.copy('C')
		#db = aims.knn.Database()
		#db.init(X)
		#aimsalgo.AimsGeneralizedKnnParzenPdf(db, img_count, k)
		#aimsalgo.AimsKnnPdf(db, img_count, k)
		array = img_count.volume().get().arraydata()
		# apply correction if needed
		array[array < 0.] = 0.
		# avoid null values
		array += 10e-50
		# model external pdf energy
		self._missing_energy = 10e-50
		# normalize
		array *= (1 - self._missing_energy) / array.sum()
		if self._fromlog:
			z = (array == 0)
			notz = (z == 0)
			array[notz] = numpy.log(array[notz])
			array[z] = -100
		if write_count: aims.Writer().write(img_count, write_count)
		self._bb_talairach_offset = bb_talairach_offset
		self._bb_talairach_size = bb_talairach_size
		self._img_density = img_count


	def write(self, filename):
		header = self._img_density.header()
		header['bb_talairach_offset'] = self._bb_talairach_offset
		header['missing_energy'] = self._missing_energy
		header['from_log'] = self._fromlog
		header['ss'] = self._ss
		aims.Writer().write(self._img_density, filename)

	def read(self, filename):
		img = aims.Reader().read(filename)
		img = aims.AimsData_FLOAT(img)
		header = img.header()
		self._bb_talairach_size = img.dimX(), img.dimY(), img.dimZ()
		self._img_density = img
		try:
			l = header['bb_talairach_offset']
		except KeyError:
			self._bb_talairach_offset = 0, 0, 0
		else:
			self._bb_talairach_offset = l[0], l[1], l[2]
		try:
			self._missing_energy = header['missing_energy']
		except KeyError:
			self._missing_energy = 0.
		try:
			self._fromlog = header['from_log']
		except KeyError:
			self._fromlog = False
		try:
			self._ss = header['ss']
		except KeyError:
			self._ss = True

	def prodlikelihoods(self, X, shift=10.):
		'''
    shift :    return likelihood = exp(log(P(Xi|Li=li) + shift)
                                 = P(Xi|Li=li) * exp(shift)
                                 = exp(loglikelihhod + shift)

               usefull to avoid zeros/inf/nan values in likelihood computation
               and shift is removed in posterior computation :
               
                                  P(Xi|Li=li) * P(Li=Li)
               P(Li=li|Xi) = -------------------------------
                              ___
                              \   P(Xi|Li=lj) * P(Li=lj)
                              /__
                               lj

                                  P(Xi|Li=li) * exp(shift) * P(Li=Li)
                           = -----------------------------------------
                              ___
                              \   P(Xi|Li=lj) * exp(shift) * P(Li=lj)
                              /__
                               lj

		'''
		Px = self._img_density.volume()
		P = Px.get().arraydata()
		t = numpy.array(self._bb_talairach_offset)
		s = numpy.array(self._bb_talairach_size)
		X = numpy.array(X - t, dtype='int')
		voxels_n = len(X)
		loglikelihood = 0.
		for p_out in X:
			if (p_out < 0).sum() or (p_out >= s).sum():
				loglikelihood += -50
				continue
			pos = tuple([0] + p_out[::-1].tolist())
			val = P[pos]
			if val:
				loglikelihood += numpy.log(P[pos])
			else:   loglikelihood += -50
		loglikelihood /= voxels_n
		likelihood = numpy.exp(numpy.longdouble(loglikelihood) + shift)
		del P
		del Px
		return loglikelihood, likelihood

	def likelihoods(self, X, shift=10.):
		'''
    shift :    return likelihood = exp(log(P(Xi|Li=li) + shift)
                                 = P(Xi|Li=li) * exp(shift)
                                 = exp(loglikelihhod + shift)

               usefull to avoid zeros/inf/nan values in likelihood computation
               and shift is removed in posterior computation :
               
                                  P(Xi|Li=li) * P(Li=Li)
               P(Li=li|Xi) = -------------------------------
                              ___
                              \   P(Xi|Li=lj) * P(Li=lj)
                              /__
                               lj

                                  P(Xi|Li=li) * exp(shift) * P(Li=Li)
                           = -----------------------------------------
                              ___
                              \   P(Xi|Li=lj) * exp(shift) * P(Li=lj)
                              /__
                               lj

		'''
		Px = self._img_density.volume()
		P = Px.get().arraydata()
		t = numpy.array(self._bb_talairach_offset)
		s = numpy.array(self._bb_talairach_size)
		X = numpy.array(X - t, dtype='int')
		voxels_n = len(X)
		loglikelihoods = []
		for p_out in X:
			if (p_out < 0).sum() or (p_out >= s).sum():
				loglikelihoods.append(-50)
				continue
			pos = tuple([0] + p_out[::-1].tolist())
			val = P[pos]
			if val:
				loglikelihoods.append(numpy.log(P[pos]))
			else:   loglikelihoods.append(-50)
		loglikelihoods = numpy.array(loglikelihoods)
		likelihoods = numpy.exp(numpy.longdouble(loglikelihoods) + shift)
		del P
		del Px
		return loglikelihoods, likelihoods

	def weighted_prodlikelihoods(self, X, weights, shift=10.):
		Px = self._img_density.volume()
		P = Px.get().arraydata()
		t = numpy.array(self._bb_talairach_offset)
		s = numpy.array(self._bb_talairach_size)
		X = numpy.array(X - t, dtype='int')
		voxels_n = len(X)
		loglikelihood = 0.
		for i, p_out in enumerate(X):
			w = weights[i]
			if (p_out < 0).sum() or (p_out >= s).sum():
				loglikelihood += w * (-50)
				continue
			pos = tuple([0] + p_out[::-1].tolist())
			val = P[pos]
			if val:
				loglikelihood += w * numpy.log(P[pos])
			else:   loglikelihood += w * (-50)
		loglikelihood /= voxels_n
		likelihood = numpy.exp(numpy.longdouble(loglikelihood) + shift)
		del P
		del Px
		return loglikelihood, likelihood

	def powered_integral(self, alpha):
		'''
    External energy (out of sulcus bounding box) must be taked into account.
    Indeed, for alpha nearest zero value, the external energy increase really
    fast. If we forgot this external energy the limit distribution when alpha is
    null is a uniform distribution in the bounding box.

    External energy is estimated here by a uniform distribution in a bigger
    bounding box of size 100x100x100. So the local energy in each voxel is equal
    to : E_ext / n with n = number of voxels in the difference between the two
    bounding box.
		'''
		Px = self._img_density.volume()
		P = Px.get().arraydata()
		# volume big bb = 200^3
		# n = (volume big bb) - (volume sulcus bb)
		n = 8000000. - numpy.prod(P.shape)
		# bouding box internal energy
		int = (P ** alpha).sum()
		# bounding box external energy
		ext = n * (self._missing_energy / n) ** alpha
		del P 
		del Px
		return int + ext

	def powered_img_density(self, alpha):
		Px = self._img_density.volume()
		P = Px.get().arraydata()
		array = (P ** alpha)
		# volume big bb = 200^3
		# n = (volume big bb) - (volume sulcus bb)
		n = 8000000. - numpy.prod(P.shape)
		# bouding box internal energy
		int = array.sum()
		# bounding box external energy
		ext = n * (self._missing_energy / n) ** alpha
		array /= (int + ext)
		del P 
		del Px
		return array

	def likelihood_debug(self, sulci, graph, vertex):
		'''only for debug'''
		motion = aims.GraphManip.talairach(graph)

		Px = self._img_density.volume()
		P = Px.get().arraydata()

		if self._ss: bucket_name = 'aims_ss'
		else: bucket_name = 'aims_bottom'
		ss_map = vertex[bucket_name].get()
		t = numpy.array(self._bb_talairach_offset)
		s = numpy.array(self._bb_talairach_size)
		size_in = numpy.array([ss_map.sizeX(), ss_map.sizeY(),
							ss_map.sizeZ()])
		loglikelihood = 0.
		voxels_n = len(ss_map[0].keys())
		img = aims.AimsData_FLOAT(*self._bb_talairach_size)
		Ax = img.volume()
		A = Ax.get().arraydata()
		A[:] = P[:]
		for p_in in ss_map[0].keys():
			p_in = aims.Point3df(p_in * size_in)
			p_out = motion.transform(p_in) - t
			p_out = numpy.array(p_out, dtype='int')
			if (p_out < 0).sum() or (p_out >= s).sum():
				loglikelihood += -30
				continue
			pos = tuple([0] + p_out[::-1].tolist())
			loglikelihood += numpy.log(P[pos])
			A[pos] = 0.0001
		loglikelihood /= voxels_n
		likelihood = numpy.exp(numpy.longdouble(loglikelihood) + 10)
		aims.Writer().write(img, "img_%s.ima" % sulci)
		del P 
		del Px
		return loglikelihood, likelihood
	
	
	def bb_talairach(self):
		return self._bb_talairach_offset, self._bb_talairach_size

	def img_density(self):
		return self._img_density

	def nodes_number(self):
		return self._nodes_n

	def voxels_number(self):
		return self._voxels_n

class Spam(PySpam):
	def __init__(self, gaussian_std=2, fromlog=False):
		PySpam.__init__(self, gaussian_std, fromlog)
		self._c_spam = None

	def fit_graphs(self, infos=None, motions=None, ss=True,
						write_count=None):
		PySpam.fit_graphs(self, infos, motions, ss, write_count)
		self.update()

	def fit(self, X, ss=True, write_count=None, weights=None):
		PySpam.fit(self, X, ss, write_count, weights)
		self.update()

	def update(self):
		if self._c_spam is None:
			if self._fromlog:
				self._c_spam = aims.Spam()
			else:	self._c_spam = aims.SpamFromLikelihood()
		self._c_spam.set_bb_talairach_offset(self._bb_talairach_offset)
		self._c_spam.set_bb_talairach_size(self._bb_talairach_size)
		self._c_spam.set_img_density(self._img_density.volume().get())

	def read(self, filename):
		PySpam.read(self, filename)
		self.update()
		
	def prodlikelihoods(self, X, shift=10.):
		if X.flags['C_CONTIGUOUS']: X = X.copy('fortran')
		v = aims.Volume_DOUBLE(X)
		logli, li = self._c_spam.prodlikelihoods(v, shift)
		return logli, li
		

################################################################################
class SpamMixtureModel(MixtureModel):
	def __init__(self, models, priors):
		MixtureModel.__init__(self, models, priors)

################################################################################

class DepthWeightedSpam(PySpam):
	def __init__(self, gaussian_std=2, fromlog=False):
		PySpam.__init__(self, gaussian_std, fromlog)
		self._name = 'depth_weighted_spam'
		self._gaussian = distribution.Gaussian()

	def gaussian(self): return self._gaussian

	def fit(self, X, ss=True, write_count=None, weights=None):
		PySpam.fit(self, X, ss, write_count, weights)

	def toTuple(self):
		return (self._name, self._imafilename, self._gaussian.toTuple())

	def fromTuple(self, tuple):
		self._name,  self._imafilename, g = tuple
		self._gaussian = distribution.Gaussian()
		self._gaussian.fromTuple(g)

	def read(self, filename):
		import re
		Distribution.read(self, filename)
		PySpam.read(self, self._imafilename)
		self.update()

	def write(self, filename):
		import re
		self._imafilename = re.sub('\.data$', '.ima', filename)
		Distribution.write(self, filename)
		PySpam.write(self, self._imafilename)


	def update(self):
		self._gaussian.update()

################################################################################
class Bingham(Distribution):
	def __init__(self, M=None, Z=None):
		'''
    Bingham distribution for data living on a p-dimensional sphere.
    Provide a (full covariance) gaussian-like distribution on axial data.

    optional parameters :
    ---------------------
    M :       orthogonal matrix
    Z :       vector (numpy array) representation of a diagonal matrix

    given the following density function :

               exp (xT.A.x)
    f(x) = ----------------------  with A = M.Z.MT (M orthogonal, Z : diagonal)
               1F1(0.5, 1.5, Z)

    ref : "An Antipodally Symmetric Distribution on the Sphere", C. Bingham
		'''
		Distribution.__init__(self)
		self._name = 'bingham'
		self._M = M
		self._Z = Z
		if not None in [M, Z]: self.update()
		else:
			self._lognormalization = None
			self._A = None

	def setUniform(self, dim):
		self._M = numpy.asmatrix(numpy.identity(dim))
		self._Z = numpy.ones(dim)
		self.update() # A + normalization

	def GetMeanDirection(self):
		ind = numpy.argmax(self._Z)
		return numpy.array(self._M[:,ind]).ravel()

	def M(self): return self._M

	def Z(self): return self._Z

	def update(self):
		'''update internal parameters'''
		p = self._M.shape[0]
		# warning : huge value for nb_iter is good for precision
		# but can gives NaN results if Z components value are too big.
		nb_iter = 100
		self._lognormalization = numpy.log(aimsalgo.hyp1f1(0.5, p/ 2.,
							self._Z, nb_iter))
		self._A = numpy.dot(self._M,
			numpy.dot(numpy.diag(self._Z), self._M.T))

	def compute_Z(self, X, weights):
		def func(z, X, weights):
			self._Z = numpy.asarray(z).ravel()
			self.update()
			logli, li = self.likelihoods(X)
			if numpy.isnan(self._lognormalization):
				energy = numpy.inf
			else:	energy = -logli.sum()
			if weights is None: energy /= len(X)
			return energy

		import scipy.optimize
		z0 = numpy.zeros(self._M.shape[0])
		eps = 10e-5
		try:
			res = scipy.optimize.fmin_powell(func, z0,
					args=(X, weights), disp=0, ftol=eps)
		except RuntimeError:
			print "warning: Bingham: optimization failed."
			return None
		return numpy.asarray(res).ravel()

	def fit(self, X, weights=None):
		'''
    mle for M and numerical optimization (powell) for Z
		'''
		X = numpy.asarray(X)
		n = X.shape[0]
		if weights is not None:
			weights = numpy.asarray(weights) / weights.sum()
		if weights is None:
			S = (numpy.asmatrix(X).T * numpy.asmatrix(X)) / n
		else:	S = numpy.dot(X.T, (X.T * weights).T)
		val, vec = numpy.linalg.eig(S)
		ind = numpy.argsort(val)
		self._M = numpy.asmatrix(vec[:, ind])
		self._Z = self.compute_Z(X, weights)
		if self._Z is None:
			return False
		self.update()
		return True

	def likelihoods(self, X):
		X = numpy.asarray(X).reshape(-1, X.shape[-1])
		AX = numpy.dot(numpy.asarray(self._A), X.T).T
		XTAX = (X * AX).sum(axis=1)
		logli = XTAX - self._lognormalization
		li = numpy.exp(logli)
		return logli, li

	def likelihood(self, x):
		x = numpy.asarray(x)
		x = x.reshape(-1, x.shape[-1])
		AX = numpy.dot(numpy.asarray(self._A), x.T).T
		XTAX = (x * AX).sum(axis=1)[0]
		logli = XTAX - self._lognormalization
		li = numpy.exp(logli)
		return logli, li

	def toTuple(self):
		return (self._name, self._A, self._M, self._Z)

	def fromTuple(self, tuple):
		self._name,  self._A, self._M, self._Z = tuple



class MatrixVonMisesFisher(Distribution):
	def __init__(self, shape=None, F=None):
		'''
    Matrix Von Mises Fisher distribution for data living on the Stiefel
    manifold : data verifying : XT.X = Id. Provide a (full covariance)
    gaussian-like distribution on rotation matrices or directions set.

    optional parameters :
    ---------------------
    shape : (n, p) size of Stiefel manifold.
    F :     parameter matrix

    given the following density function :

                exp (FX^t)
    f(x) = ----------------------  with F = DPG (svd decomposition)
              0F1(p/2, P^2/4)           DD^t = G^tG = Id, p : dimension of axes

    ref : "The Von Mises-Fisher Matrix Distribution in Orientations Statistics",
          C.G. Khatri, K.V. Mardia.
		'''
		Distribution.__init__(self)
		self._name = 'matrix_von_mises_fisher'
		self._shape = shape
		self._F = F
		if not None in [shape, F]: self._update()
		else:
			self._lognormalization = None
			self._F = self._D = self._P = self._G = None

	def F(self): return self._F

	def update(self):
		'''update internal parameters'''
		dim = self._G.shape[1]
		nb_iter = 100
		self._lognormalization = aimsalgo.loghyp0f1(dim / 2.,
					(self._P ** 2) / 4., nb_iter)
		S = numpy.zeros((self._D.shape[1], self._G.shape[0]))
		for i, p in enumerate(self._P): S[i, i] = p
		self._F = numpy.dot(self._D, numpy.dot(S, self._G))

	def compute_P(self, X):
		def func(p, X):
			self._P = numpy.asarray(p).ravel()
			self.update()
			energy = 0.
			for x in X:
				x = x.reshape(self._shape)
				logli, li = self.likelihood(x)
				energy -= logli
			if weights is None: energy /= len(X)
			return energy

		import scipy.optimize
		p0 = numpy.zeros(self._D.shape[1])
		eps = 10e-5
		res = scipy.optimize.fmin_powell(func, p0, args=(X,),
							disp=0, ftol=eps)
		return numpy.asarray(res).ravel()

	def fit(self, X, weights=None):
		'''
    mle for D, G and numerical optimization (powell) for P
		'''
		X = numpy.asarray(X)
		if weights is not None:
			weights = numpy.asarray(weights) / weights.sum()
		if weights is None:
			xm = X.mean(axis=0)
		else:	xm = numpy.dot(weights, X)
		xm = xm.reshape(self._shape)
		self._D, dummy, self._G = numpy.linalg.svd(xm)
		self._P = self.compute_P(X)
		self.update()

	def likelihood(self, X):
		'''
    X : matrix on Stiefeld manifold.
		'''
		X = numpy.asarray(X)
		d = numpy.trace(numpy.dot(self._F, X.T))
		logli = d - self._lognormalization
		li = numpy.exp(logli)
		return logli, li

	def toTuple(self):
		return (self._name, self._shape, self._F,
				self._D, self._P, self._G)

	def fromTuple(self, tuple):
		self._name, self._shape, self._F, \
			self._D, self._P, self._G = tuple


class MatrixBingham(Distribution):
	def __init__(self, shape=None, A=None):
		'''
    Matrix Bingham distribution for data living on the Stiefel(n,p)
    manifold : data verifying : XT.X = Id. Provide a (full covariance)
    gaussian-like distribution on rotation matrices or axes set.

    optional parameters :
    ---------------------
    shape : (n, p) size of Stiefel manifold.
    A :     parameter matrix

    given the following density function :

                exp (X.A.X^t)
    f(x) = ----------------------  with A = MDM^t (spectral decomposition)
              1F1(n/2, p/2, D)          p : dimension of axes
                                        n : number of axes 

    ref : "Maximum Likelihood Estimators for the Matrix Von Mises-Fisher and
           Bingham Distributions", P.E. Jupp and K.V. Mardia
		'''
		Distribution.__init__(self)
		self._name = 'matrix_bingham'
		self._shape = shape
		self._A = A
		if not None in [shape, A]: self._update()
		else:
			self._lognormalization = None
			self._A = self._M = self._D = None

	def A(self): return self._A

	def M(self): return self._M

	def D(self): return self._D

	def update(self):
		'''update internal parameters'''
		n, p = self._shape
		# copy else strange memory error (shape of D might be modified)
		nb_iter = 100
		L = numpy.log(aimsalgo.hyp1f1(n / 2., p / 2.,
					self._D.copy(), nb_iter))
		self._lognormalization = L
		self._A = numpy.dot(self._M,
			numpy.dot(numpy.diag(self._D), self._M.T))

	def compute_D(self, X, weights):
		def func(d, X, weigths):
			# max concentration parameter set to 0
			self._D = numpy.hstack([numpy.asarray(d).ravel(), 0.])
			self.update()
			energy = 0.
			for i, x in enumerate(X):
				x = x.reshape(self._shape)
				logli, li = self.likelihood(x)
				if weights is not None: logli *= weights[i]
				energy -= logli
			if weights is None: energy /= len(X)
			if numpy.isnan(energy): energy = numpy.inf
			return energy

		import scipy.optimize
		d0 = numpy.zeros(self._M.shape[0] - 1)
		eps = 10e-5
		res = scipy.optimize.fmin_powell(func, d0, args=(X, weights),
							disp=0, ftol=eps)
		return numpy.hstack([numpy.asarray(res).ravel(), 0.])

	def fit(self, X, weights=None):
		'''
    mle for M and numerical optimization (powell) for D
		'''
		X = numpy.asarray(X)
		N = X.shape[0]
		if weights is not None:
			weights = numpy.asarray(weights) / weights.sum()
		X2 = X.reshape(len(X), self._shape[0], self._shape[1])
		if weights is None:
			Y = numpy.tensordot(X2, X2 , ([0, 1], [0, 1])) / N
		else:	Y = numpy.tensordot(X2, (X2.T * weights).T, \
							([0, 1], [0, 1]))
		n, p = self._shape
		I = numpy.identity(p)
		tY = Y - (float(n) / p) * I # (n,p) with null trace
		val, vec = numpy.linalg.eig(tY)
		ind = numpy.argsort(val)
		self._M = numpy.asmatrix(vec[:, ind])
		self._D = self.compute_D(X, weights)
		self.update()

	def likelihood(self, X):
		'''
    X : matrix on Stiefeld manifold.
		'''
		X = numpy.asarray(X)
		Y = numpy.dot(X.T, X)
		d = numpy.trace(numpy.dot(self._A, Y))
		logli = d - self._lognormalization
		li = numpy.exp(logli)
		return logli, li

	def toTuple(self):
		return (self._name, self._shape, self._A, self._M, self._D)

	def fromTuple(self, tuple):
		self._name, self._shape, self._A, self._M, self._D = tuple




# TODO : distribution a ajouter :
# 1) matrix (rotations) Cayley distribution
# f(X) = C(X) * exp(trace(X * F.T)) avec X : matrice orthogonale d'orientation
# 2) matrix_watson_orientation_distribution (cas special de matrix von mises)
# avec F = kappa * G0 (G0 : matrix)
################################################################################
distribution_map.update({\
	'spam' : Spam,
	'spam_mixture' : SpamMixtureModel,
	'depth_weighted_spam' : DepthWeightedSpam,
	'bingham' : Bingham,
	'matrix_von_mises_fisher' : MatrixVonMisesFisher,
	'matrix_bingham' : MatrixBingham,
})
