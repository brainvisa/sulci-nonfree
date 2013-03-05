#!/usr/bin/env python
import os, sys, exceptions, numpy
from optparse import OptionParser

import PyQt4.QtGui as qt
qt4 = True

import sigraph
import anatomist.direct.api as anatomist
from soma import aims, aimsalgo
from datamind.tools import *
try:
  import fff.GMM
except:
  print 'warning, fff is not here or does not work. GMM will not be usable'

from sulci.common import io
from sulci.models import check_same_distribution, distribution, \
				distribution_aims, distribution_fff

qApp = qt.QApplication(sys.argv)
a = anatomist.Anatomist()
inf = numpy.inf


def distributionArray3DToMeshes(mesher, bb_talairach_offset,
				img_density, array, prob):
	meshes, img_thresholds = [], []

	# distribution analyse
	array_sort = numpy.sort(array.flatten())[::-1]
	array_cum = array_sort.cumsum()
	mesher.setVerbose(False)
	mesher.setSmoothing(180.0, 5, 0.4, 0.2, 0.2)
	mesher.setDecimation(100.0, 5, 3, 180.)
	mesher.setMinFacetNumber(50)
	for i in range(3):
		ind = numpy.argwhere(array_cum >= prob[i])
		if len(ind) == 0: continue
		ind = ind[0,0]
		t = array_sort[ind]

		# threshold with border
		thresholder = aims.AimsThreshold_FLOAT_S16(\
			aims.AIMS_GREATER_OR_EQUAL_TO, t)
		img_threshold = thresholder.bin(img_density)
		img_threshold_border = addBorder(img_threshold,
						1, -1, True)
	
		# create mesh
		mesh = aims.AimsSurfaceTriangle()
		mesher.getSingleLabel(img_threshold_border, mesh)

		# translate mesh
		transformation = numpy.identity(4)
		transformation = numpy.asarray(transformation).flatten()
		motion = aims.Motion(transformation)
		off3df = aims.Point3df(bb_talairach_offset)
		motion.setTranslation(off3df)
		aims.SurfaceManip.meshTransform(mesh, motion)
		meshes.append(mesh)
		img_thresholds.append(img_threshold)

	return meshes, img_thresholds

def colorMeshes(meshes, color, alphas):
	ameshes = []
	for mesh, alpha in zip(meshes, alphas):
		# add material
		amesh = a.toAObject(mesh)
		material = anatomist.cpp.Material()
		color2 = list(color) + [alpha]
		go =  {'diffuse' : color2}
		material.set(aims.Object(go).get())
		amesh.SetMaterial(material)
		mesh.header()['material'] = material.genericDescription()
		ameshes.append(amesh)
	return ameshes


class Display(object):
	def __init__(self, segments_distrib, hie, selected_sulci, generate_all,
					features, sulci_weights):
		self._segments_distrib = segments_distrib
		self._hie = hie
		self._selected_sulci = selected_sulci
		self._awin = a.createWindow('3D')
		self._awin.setHasCursor(0)
		self._aobjects = []
		self._writer = aims.Writer()
		self._generate_all = generate_all
		self._features = features
		self._handle_features()
		self._sulci_weights = sulci_weights

	def display(self):
		i = 1
		distribs = self._segments_distrib['vertices']
		s = len(distribs)
		for sulcus, distrib in distribs.items():
			if self._selected_sulci is not None:
				if not (sulcus in self._selected_sulci):
					continue
			else:	print "%2d/%d :  %s" % (i, s, sulcus)
			self._display_one(sulcus, distrib)
			i += 1

		a.addObjects(self._aobjects, [self._awin])

	def _handle_features(self):
		msg.warning('feature option not available for this ' \
							'distribution')

	def find_color(self, sulcus):
		if sulcus == 'error': return [0, 0.5, 1]
		elif sulcus == 'ok': return [1, 0.5, 0]
		return self._hie.find_color(sulcus)
		

class GaussianDisplay(Display):
	def __init__(self, *args, **kwargs):
		Display.__init__(self, *args, **kwargs)
		self._ratio = [0.5, 1, 2]
		self._alpha = [1, 0.6, 0.2]
		self._unit_sphere = aims.SurfaceGenerator.sphere([0, 0, 0],
								1, 96)

	def _generate_one(self, sulcus, mean, cov, scale=1):
		color = self.find_color(sulcus)
		mean3df = aims.Point3df(numpy.asarray(mean).flatten())
		eigval, eigvect = numpy.linalg.eig(cov)
		d = numpy.diag(numpy.sqrt(eigval))
		meshes = []
		for i in range(len(self._ratio)):
			color2 = list(color) + [self._alpha[i]]
			cov = eigvect * (d * self._ratio[i]) * eigvect.I
			mesh = aims.AimsTimeSurface_3(self._unit_sphere)
			transformation = numpy.identity(4)
			transformation[:3, :3] = cov
			flattrans = numpy.asarray(transformation).flatten()
			motion = aims.Motion(flattrans)
			if scale != 1:
				motion.scale([1.] * 3, [scale] *3)
			motion.setTranslation(mean3df)
			aims.SurfaceManip.meshTransform(mesh, motion)
			mesh.updateNormals()
			asphere = a.toAObject(mesh)
			material = anatomist.cpp.Material()
			go =  {'diffuse' : color2}
			material.set(aims.Object(go).get())
			asphere.SetMaterial(material)
			self._aobjects += [asphere]
			mesh.header()['material'] = go
			meshes.append(mesh)
		return meshes

	def _display_one(self, sulcus, gaussian):
		if isinstance(gaussian, distribution.Dirac) or \
			gaussian.type() == 'fake_gaussian' :
			return
		mean = gaussian.mean()
		cov = gaussian.covariance()
		meshes = self._generate_one(sulcus, mean, cov)
		for i in range(3):
			self._writer.write(meshes[i],
				'gaussian_%s_%d.mesh' % (sulcus, i))

class BlocGaussianDisplay(Display):
	def __init__(self, *args, **kwargs):
		Display.__init__(self, *args, **kwargs)
		self._gaussian_display = GaussianDisplay(*args, **kwargs)

	def _handle_features(self):
		available = ['extremities', 'gravity']
		if self._features not in available:
			msg.error("'" + self._features + "' is not a valid " \
				"feature, try one among : " + str(available))
			sys.exit(1)
		if self._features == 'help' :
			print "available features are : " + str(available)
			sys.exit(0)

	def _display_one(self, sulcus, gaussian):
		groups = gaussian.groups()
		h = {
			'extremities' : self._display_one_extremities,
			'gravity' : self._display_one_gravity }
		h[self._features](sulcus, groups)

	def _display_one_extremities(self, sulcus, groups):
		try:
			dim1, g1 = groups['extremity1']	
			dim2, g2 = groups['extremity2']
		except KeyError: return
		self._gaussian_display._display_one(sulcus, g1)
		self._gaussian_display._display_one(sulcus, g2)

	def _display_one_gravity(self, sulcus, groups):
		dim, g = groups['gravity']	
		self._gaussian_display._display_one(sulcus, g)

	def _display_direction(self, sulcus, groups): pass

	def _display_normal(self, sulcus, groups): pass


def addBorder(img_in, width=1, border_value=0., aims_border=False):
	# init
	dim_in = [img_in.dimX(), img_in.dimY(), img_in.dimZ()]
	if aims_border:
		dim_out = dim_in + [1, width]
	else:	dim_out = [(d + width * 2) for d in dim_in]
	img_out = img_in.__class__(*dim_out)
	a_in = img_in.volume().get().arraydata()
	a_out = img_out.volume().get().arraydata()
	
	# create mask
	mask = numpy.zeros(a_out.shape, dtype='bool')
	mask[:] = True
	r1 = numpy.arange(width)
	r2 = -numpy.arange(width) - 1
	mask[0, r1, :, :] = mask[0, r2, :, :] = \
		mask[0, :, r1, :] = mask[0, :, r2, :] = \
		mask[0, :, :, r1] = mask[0, :, :, r2] = 0
	# apply
	a_out[mask] = a_in.flatten()
	a_out[mask == 0] = border_value

	return img_out

# Old Gmm display
#class GmmDisplay(Display):
#	def __init__(self, *args, **kwargs):
#		Display.__init__(self, *args, **kwargs)
#		self._probs = [0.3, 0.6, 0.8]
#		self._alphas = [1, 0.6, 0.2]
#		self._mesher = aimsalgo.Mesher()
#
#	def _sample(self, gmm, gd, shape):
#		array = gmm.sample(gd, None, verbose=0).reshape(shape)
#		return numpy.exp(array)
#
#	def _display_one(self, sulcus, gmm):
#		gmm = gmm._gmm
#		n = gmm.k
#		#limits = []
#		# find bounding box of 2 std limits border of each gaussian
#		# weights are not considered
#		#for k in range(n):
#		#	c  = gmm.centers[k]
#		#	w = gmm.weights[k]
#		#	if gmm.prec_type == 0:
#		#		metric = gmm.precision[k].reshape(3, 3)
#		#	elif gmm.prec_type == 1:
#		#		metric = numpy.diag(gmm.precision[k])
#		#	eigval, eigvect = numpy.linalg.eig(metric)
#		#	for i in range(3):
#		#		l, v = eigval[i], eigvect[i]
#		#		v = 2 * numpy.sqrt(1./l) * v * w
#		#		limits += [c + v, c - v]
#		#limits = numpy.vstack(limits)
#		c = (numpy.asarray(gmm.centers) * \
#			gmm.weights[None].T).sum(axis=0)
#		e1 = numpy.array([40, 0, 0])
#		e2 = numpy.array([0, 40, 0])
#		e3 = numpy.array([0, 0, 40])
#		limits = numpy.array([c + e1, c + e2, c + e3,
#					c - e1, c - e2, c - e3])
#		xmin, ymin, zmin = limits.min(axis=0)
#		xmax, ymax, zmax = limits.max(axis=0)
#		gd = fff.GMM.grid_descriptor(3)
#		shape = [int(xmax - xmin), int(ymax - ymin), int(zmax - zmin)]
#		shape = numpy.array(shape)
#		#shape /= min(30, shape.min())
#		gd.getinfo([xmin, xmax, ymin, ymax, zmin, zmax], shape)
#		array = self._sample(gmm, gd, shape)
#		array /= array.sum() # spurious normalization
#		shape = array.shape
#		img_density = aims.AimsData_FLOAT(*array.shape)
#		img_density.volume().arraydata()[:] = array.T
#		if self._generate_all:
#			self._writer.write(img_density, 'img_%s.ima' % sulcus)
#		meshes, img_thresholds = distributionArray3DToMeshes(\
#				self._mesher, (xmin, ymin, zmin),
#				img_density, array, self._probs)
#		color = self._hie.find_color(sulcus)
#		self._aobjects += colorMeshes(meshes, color, self._alphas)
#		for i, mesh in enumerate(meshes):
#			self._writer.write(mesh, 'gmm_%s_%d.mesh' %(sulcus, i))
#		if self._generate_all:
#			for i, img_threshold in enumerate(img_thresholds):
#				self._writer.write(img_threshold,
#					'img_threshold_%s_%d.ima' % (sulcus, i))

#class BGmmDisplay(GmmDisplay):
#	def _sample(self, gmm, gd, shape):
#		return gmm.VB_sample(gd, None).reshape(shape)


class SpamDisplay(Display):
	def __init__(self, *args, **kwargs):
		Display.__init__(self, *args, **kwargs)
		self._prob = [0.3, 0.6, 0.8]
		self._alpha = [1, 0.6, 0.2]
		self._mesher = aimsalgo.Mesher()

	def _get_data(self, sulcus, spam):
		if self._sulci_weights != None:
			w = self._sulci_weights[sulcus]
			array = spam.powered_img_density(1. - w)
			img_density = aims.Volume_FLOAT(array)
		else:	img_density = spam.img_density()
		return img_density

	def _display_one(self, sulcus, spam):
		color = self.find_color(sulcus)

		bb_talairach_offset, bb_talairach_size = spam.bb_talairach()
		img_density = self._get_data(sulcus, spam)
		try:	array = img_density.volume().get().arraydata()
		except:	array = img_density.arraydata()
		if spam.is_fromlog(): array = numpy.exp(array)
			
		if numpy.isnan(array).sum():
			msg.warning("skip mesh generation from sulcus " \
				"'%s' : nan values found." % sulcus)
			return
	
		if self._generate_all:
			# sulcus in subject axis
			self._writer.write(img_density,
					'img_density_%s.ima' % sulcus)

		meshes, img_thresholds = distributionArray3DToMeshes(\
				self._mesher, bb_talairach_offset,
				img_density, array, self._prob)
		self._aobjects += colorMeshes(meshes, color, self._alpha)
		for i, mesh in enumerate(meshes):
			self._writer.write(mesh, 'spam_%s_%d.mesh' %(sulcus, i))
		if self._generate_all:
			for i, img_threshold in enumerate(img_thresholds):
				self._writer.write(img_threshold,
					'img_threshold_%s_%d.ima' % (sulcus, i))

class GmmFromSpamDisplay(SpamDisplay):
	def __init__(self, *args, **kwargs):
		SpamDisplay.__init__(self, *args, **kwargs)

	def _get_data(self, sulcus, gmm):
		bb_talairach_offset, bb_talairach_size = gmm.bb_talairach()

		bb_talairach_offset = numpy.array(bb_talairach_offset)
		from numpy.lib import index_tricks
		X = numpy.array([x for x in index_tricks.ndindex( \
					tuple(bb_talairach_size))])
		X += bb_talairach_offset
		d = 100
		n = (X.shape[0] / d)
		li = []
		for i in range(d + 1):
			Xi = X[i * n: (i + 1) * n]
			logli, li_i = gmm.likelihoods(Xi)
			li.append(li_i)
		li = numpy.hstack(li)
		array = li.reshape(bb_talairach_size).T[None].astype('float32')
		return aims.Volume_FLOAT(array)


class GmmFromGaussians(Display):
	def __init__(self, *args, **kwargs):
		Display.__init__(self, *args, **kwargs)
		self._gaussian_display = GaussianDisplay(*args, **kwargs)


	def _display_one(self, sulcus, gaussian):
		all_meshes = []
		pimax = numpy.max(gaussian._pi)
		for i in range(gaussian._k):
			mean = numpy.asmatrix(gaussian._C[i].astype('float'))
			cov = numpy.asmatrix(gaussian._S[i].astype('float'))
			scale = gaussian._pi[i]
			self._gaussian_display._ratio = [1]
			self._gaussian_display._alpha = [scale / pimax]
			meshes = self._gaussian_display._generate_one(sulcus,
							mean, cov)

			all_meshes.append(meshes[0])
		for i, m in enumerate(all_meshes):
			self._writer.write(m, 'gmm_%s_%d.mesh' % (sulcus, i))



################################################################################
def parseOpts(argv):
	hierarchy = os.path.join(aims.carto.Paths.shfjShared(),
					'nomenclature', 'hierarchy',
					'sulcal_root_colors.hie')
	description = 'Display learned bayesian local models ' \
			'(one for each sulci).'
	parser = OptionParser(description)
	parser.add_option('-d', '--distrib', dest='distribname',
		metavar = 'FILE', action='store', default = None,
		help='distribution models')
	parser.add_option('--hierarchy', dest='hierarchy',
		metavar = 'FILE', action='store', default = hierarchy,
		help='hierarchy (links between names and colors), ' + \
		'default : %default')
	parser.add_option('-a', '--all', dest='all',
		action='store_true', default = False,
		help='generate all intermediate image needed to compute meshes')
	parser.add_option('-s', '--sulci', dest='sulci',
		metavar = 'LIST', action='store', default = None,
		help='tag only specified manually tagged sulci.')
	parser.add_option('-f', '--features', dest='features',
		metavar = 'NAME', action='store', default = None,
		help='Display specified features (depends on distrib) : try'\
		'--feature help to see available features.')
	parser.add_option('--sulci-weights', dest='sulci_weights_filename',
		metavar='FILE', action='store', default=None,
		help="csv storing sulci_weights : need 'sulci' and " \
		"'size_errors' columns name")
	parser.add_option('--repr-number', dest='repr_number',
		metavar='INT', action='store', default=0, type='int',
		help="number of the representation (default: %default)")
	return parser, parser.parse_args(argv)

def main():
	parser, (options, args) = parseOpts(sys.argv)
	if None in [options.distribname]:
		parser.print_help()
		sys.exit(1)

	if options.sulci_weights_filename:	
		import datamind.io as datamind_io
		r = datamind_io.ReaderCsv()
		d = r.read(options.sulci_weights_filename)
		errors = numpy.asarray(d[:, 'size_errors'])
		sulci = d[:, 'sulci'].tolist()
		sulci_weights = dict(zip(sulci, errors))
	else:	sulci_weights = None

	hie = aims.Reader().read(options.hierarchy)

	print "read..."
	if options.sulci is None:
		selected_sulci = None
	else:	selected_sulci = options.sulci.split(',')
	sulcimodel = io.read_full_model(None,#options.graphmodelname,
		segmentsdistribname=options.distribname,
		selected_sulci=selected_sulci)
	distribs = sulcimodel.segments_distrib()['vertices']
	val, model_types = check_same_distribution(distribs)
	if not val:
		print "error: mix of different models is not handle, " + \
			"%s found" % str(list(model_types))
		sys.exit(1)
	model_type = list(model_types)[0]
	if not (model_type in ['gaussian', 'spam', 'depth_weighted_spam',
						'gmm_from_spam']):
		print "error : unhandle model type '%s'" % model_type
		sys.exit(1)
	else:	print "model type '%s' found" % model_type
	args = [sulcimodel.segments_distrib(), hie, selected_sulci,
			options.all, options.features, sulci_weights]
	if model_type == 'gaussian' : d = GaussianDisplay(*args)
	#if model_type == 'bloc_gaussian' : d = BlocGaussianDisplay(*args)
	elif model_type == 'gmm_from_spam':
		if options.repr_number == 0:
			d = GmmFromSpamDisplay(*args)
		elif options.repr_number == 1:
			d = GmmFromGaussians(*args)
	elif model_type in ['spam', 'depth_weighted_spam']:
		d = SpamDisplay(*args)
	d.display()
	if qt4:
		qt.qApp.exec_()
	else:
		qt.qApp.exec_loop()
	

if __name__ == '__main__' : main()
