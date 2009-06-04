#!/usr/bin/env python
import os, sys, numpy
from optparse import OptionParser
from datamind.tools import *
from sulci.common import io
from sulci.models import check_same_distribution, distribution, \
				distribution_aims, distribution_fff
from sulci.registration import procrust
from soma import aims


################################################################################
class GlobalModelDisplay(object):
	def __init__(self, gaussians_distrib, local_distrib,
		hie, options, output, selected_sulci=None):
		self._gaussians_distrib = gaussians_distrib
		self._local_distrib = local_distrib
		self._hie = hie
		self._options = options
		self._output = output
		self._selected_sulci = selected_sulci
		self._sulci = set()
		self._sulci.update(gaussians_distrib['vertices'].keys())

	def display_one(self, sulcus, gd, ld):
		color = self._hie.find_color(sulcus)

		# mesher
		Mesher, mesher_opt = mesherFactory(ld.name(), self._options)
		mesher = Mesher(ld)
		mesher.setColor(color)
		meshes = mesher.mesh(*mesher_opt)
		
		# translate
		pos = numpy.asarray(gd.mean()).ravel()
		motion = aims.Motion()
		motion.setTranslation(pos)
		for m in meshes: aims.SurfaceManip.meshTransform(m, motion)

		# write
		for i, m in enumerate(meshes):
			if len(meshes) != 1 :
				id = '_%d' % i
			else:	id = ''
			aims.Writer().write(m, '%s_%s%s.mesh' % \
					(self._output, sulcus, id))


class SegmentsDisplay(GlobalModelDisplay):
	def __init__(self, *args, **kwargs):
		GlobalModelDisplay.__init__(self, *args, **kwargs)

	def display(self):
		print "compute meshes..."
		bar = ProgressionBarPct(len(self._sulci), '#', color = 'blue')
		for i, sulcus in enumerate(self._sulci):
			if self._selected_sulci is not None and \
				sulcus not in self._selected_sulci: continue
			try: gd = self._gaussians_distrib['vertices'][sulcus]
			except KeyError: continue
			try: ld = self._local_distrib['vertices'][sulcus]
			except KeyError: continue
			bar.display(i)
			self.display_one(sulcus, gd, ld)


class RelationsDisplay(GlobalModelDisplay):
	def __init__(self, *args, **kwargs):
		GlobalModelDisplay.__init__(self, *args, **kwargs)

	def display(self):
		print "compute meshes..."
		relations = self._local_distrib['edges'].keys()
		bar = ProgressionBarPct(len(relations), '#', color = 'blue')
		for i, relation in enumerate(relations):
			# skip : intra/inter global models
			if not isinstance(relation, tuple): continue
			label1, label2 = relation
			if self._selected_sulci is not None and \
				(label1 not in self._selected_sulci or \
				label2 not in self._selected_sulci): continue
			try: gd1 = self._gaussians_distrib['vertices'][label1]
			except KeyError: continue
			try: gd2 = self._gaussians_distrib['vertices'][label2]
			except KeyError: continue
			try: ld = self._local_distrib['edges'][relation]
			except KeyError: continue
			bar.display(i)
			if label1 != label2:
				self.display_one(relation, gd1, gd2, ld)
			else:	GlobalModelDisplay.display_one(self, label1,
								gd1, ld)

	def display_one(self, relation, gd1, gd2, ld):
		#FIXME : ne marche que pour certaines fonctions d'orientation
		# mesher
		Mesher, mesher_opt = mesherFactory(ld.name(), self._options)
		mesher = Mesher(ld)
		meshes = mesher.mesh(*mesher_opt)
	
		# split the mesh in 2 parts along a plane directed by
		# the mean direction
		dir = numpy.hstack((ld.GetMeanDirection(), 0))
		plane1 = tuple(-dir)
		plane2 = tuple(dir)
		borderline1 = aims.AimsTimeSurface_2()
		borderline2 = aims.AimsTimeSurface_2()
		mesh1 = aims.AimsSurfaceTriangle()
		mesh2 = aims.AimsSurfaceTriangle()
		for m in meshes:
			m1 = aims.AimsSurfaceTriangle()
			m2 = aims.AimsSurfaceTriangle()
			aims.SurfaceManip.cutMesh(m, plane1, m1, borderline1)
			aims.SurfaceManip.cutMesh(m, plane2, m2, borderline2)
			mesh1 += m1
			mesh2 += m2

		# add color
		label1, label2 = relation
		color1 = self._hie.find_color(label1)
		go =  {'diffuse' : color1}
		mesh1.header()['material'] = go
		color2 = self._hie.find_color(label2)
		go =  {'diffuse' : color2}
		mesh2.header()['material'] = go

		# translate
		pos1 = numpy.asarray(gd1.mean()).ravel()
		pos2 = numpy.asarray(gd2.mean()).ravel()
		pos = (pos1 + pos2) / 2.
		motion = aims.Motion()
		motion.setTranslation(pos)
		aims.SurfaceManip.meshTransform(mesh1, motion)
		aims.SurfaceManip.meshTransform(mesh2, motion)

		# write
		for i, m in enumerate(meshes):
			if len(meshes) != 1 :
				id = '_%d' % i
			else:	id = ''
			#aims.Writer().write(mesh, '%s_%s%s.mesh' % \
			#		(self._output, sulcus, id))
			aims.Writer().write(mesh1, '%s_%s,%s_1%s.mesh' % \
					(self._output, label1, label2, id))
			aims.Writer().write(mesh2, '%s_%s,%s_2%s.mesh' % \
					(self._output, label1, label2, id))





################################################################################
class LocalModelMesher(object):
	'''
    ld : local distribution
	'''
	def __init__(self, ld):
		self._color = None
		self._ld = ld
		self._unit_sphere = aims.SurfaceGenerator.sphere([0, 0, 0],
								1, 4096)
	def setColor(self, color): self._color = color

	def _color_mesh(self, mesh):
		go =  {'diffuse' : self._color}
		mesh.header()['material'] = go

class OrientationMesher(LocalModelMesher):
	'''
    ld : orientation distribution
	'''
	def __init__(self, *args, **kwargs):
		LocalModelMesher.__init__(self, *args, **kwargs)

	def mesh(self, scale, size, s):
		mesh = aims.AimsTimeSurface_3(self._unit_sphere)
		for v in mesh.vertex():
			logli, li = self._ld.likelihood(v * s)
			v += li * v / scale
			v *= size
		mesh.updateNormals()
		if self._color is not None: self._color_mesh(mesh)
		return [mesh]

class BinghamOrientationMesher(OrientationMesher):
	'''
    ld : bingham distribution
	'''
	def __init__(self, *args, **kwargs):
		OrientationMesher.__init__(self, *args, **kwargs)

	def mesh(self, scale, size, s):
		return OrientationMesher.mesh(self, scale, size / 5, s)


# FIXME : old test, not already integrated in new API
class MatrixBinghamMesher(LocalModelMesher):
	def __init__(self, *args, **kwargs):
		LocalModelMesher.__init__(self, *args, **kwargs)

	def create_marginal_mesh(self, i):
		n = 100
		mesh = aims.AimsTimeSurface_3(self._unit_sphere)
		for v in mesh.vertex():
			lis = 0.
			vx = v.arraydata()
			ind = numpy.argwhere(vx != 0)[0]
			u = numpy.zeros(3)
			u[ind] = vx[(ind + 1) % 3]
			u[(ind + 1) % 3] = -vx[ind]
			u[(ind + 2) % 3] = 0
			u /= numpy.linalg.norm(u)
			u = numpy.asmatrix(u).T
			for theta in numpy.linspace(0, numpy.pi, n):
				R = procrust.rotation_from_vector(vx * theta)
				R = numpy.asmatrix(R)
				vy = numpy.asarray(R * u).ravel()
				if i == 0:
					X = numpy.vstack([vx, vy])
				elif i == 1:
					X = numpy.vstack([vy, vx])
				else:
					vz = numpy.cross(vx, vy)
					vz /= numpy.linalg.norm(vz)
					X = numpy.vstack([vz, vx])
				logli, li = self._ld.likelihood(X)
				lis += li
			lis *= numpy.pi / n
			v += 2 * lis * v / self._scale
			v *= self._size
		mesh.updateNormals()
		return mesh

	def mesher(self):
		motion = aims.Motion()
		motion.setToIdentity()
		motion.setTranslation(self._mean)
		color = self._p._hie.find_color(self._sulcus)
		go =  {'diffuse' : color}

		for i in range(3):
			mesh = self.create_marginal_mesh(i)
			mesh.header()['material'] = go
			aims.SurfaceManip.meshTransform(mesh, motion)
			aims.Writer().write(mesh, '%s_%s_%d.mesh' % \
					(self._p._output, self._sulcus, i))
			break #FIXME : all 3 marginales give the same results

		e = 50.
		c = aims.SurfaceGenerator.cylinder([-e, 0, 0], [e, 0, 0],
						0.2, 0.2, 6, False, True)
		c += aims.SurfaceGenerator.cylinder([0, -e, 0], [0, e, 0],
						0.2, 0.2, 6, False, True)
		c += aims.SurfaceGenerator.cylinder([0, 0, -e], [0, 0, e],
						0.2, 0.2, 6, False, True)
		m = numpy.identity(4)
		m[:3, :3] = self._ld.M().T
		motion2 = aims.Motion(m.flatten())
		aims.SurfaceManip.meshTransform(c, motion * motion2)

		c.header()['material'] = go
		aims.Writer().write(c, '%s_axes_%s.mesh' % \
				(self._p._output, self._sulcus))


class GaussianMesher(LocalModelMesher):
	def __init__(self, *args, **kwargs):
		LocalModelMesher.__init__(self, *args, **kwargs)
		self._ratio = [0.5, 1, 2]
		self._alpha = [1, 0.6, 0.2]

	def mesh(self):
		gaussian = self._ld
		meshes = []
		if gaussian.type() == 'fake_gaussian': return
		#mean = gaussian.mean()
		#mean3df = aims.Point3df(numpy.asarray(mean).flatten())
		cov = gaussian.covariance()
		eigval, eigvect = numpy.linalg.eig(cov)
		d = numpy.diag(numpy.sqrt(eigval))
		for i in range(3):
			color2 = list(self._color) + [self._alpha[i]]
			cov = eigvect * (d * self._ratio[i]) * eigvect.I
			mesh = aims.AimsTimeSurface_3(self._unit_sphere)
			transformation = numpy.identity(4)
			transformation[:3, :3] = cov
			flattrans = numpy.asarray(transformation).flatten()
			motion = aims.Motion(flattrans)
			#motion.setTranslation(mean3df)
			aims.SurfaceManip.meshTransform(mesh, motion)
			go =  {'diffuse' : color2}
			mesh.header()['material'] = go
			meshes.append(mesh)

		return meshes


class FakeMesher(LocalModelMesher):
	def __init__(self, *args, **kwargs):
		LocalModelMesher.__init__(self, *args, **kwargs)

	def mesh(self):
		return []

################################################################################
mesher_map = {
	'orientation' : OrientationMesher,
	'bingham' : BinghamOrientationMesher,
	'full_gaussian' : GaussianMesher,
}

def mesherFactory(name, options):
	if name in ['kent', 'von_mises_fisher']: name = 'orientation'
	if name in ['orientation', 'bingham']:
		if options.inverse:
			s = -1
		else:	s = 1
		mesher_opt = float(options.scale), float(options.size), s
	else:	mesher_opt = []
	try:
		mesher = mesher_map[name]
	except KeyError:
		mesher = FakeMesher
	return mesher, mesher_opt

################################################################################
def parseOpts(argv):
	description = 'Display learned bayesian local models ' \
			'(one for each sulci).'
	parser = OptionParser(description)
	parser.add_option('--distrib-gaussians', dest='distrib_gaussians_name',
		metavar = 'FILE', action='store', default = None,
		help='distribution models')
	parser.add_option('--distrib-local',
		dest='distrib_local_name', metavar = 'FILE',
		action='store', default = None, help='distribution models')
	parser.add_option('-s', '--sulci', dest='sulci',
		metavar = 'LIST', action='store', default = None,
		help='tag only specified manually tagged sulci.')
	parser.add_option('--level', dest='level', metavar='TYPE',
		action='store', default = 'segments', type='choice',
		choices=('segments', 'relations'),
		help="level : 'segments' or 'relations'")
	parser.add_option('-o', '--output', dest='output',
		metavar = 'FILE', action='store', default = 'output.mesh',
		help='output mesh pattern FILE gives FILE_sulcus.mesh ' + \
		'for each sulcus')
	hie_filename = os.path.join(aims.carto.Paths.shfjShared(),
					'nomenclature', 'hierarchy',
					'sulcal_root_colors.hie')
	parser.add_option('--hierarchy', dest='hie', metavar = 'FILE',
		action='store', default = hie_filename,
		help='hierarchy (used to get color of labels) ' + \
		'(default : %default)')
	parser.add_option('--size', dest='size', metavar = 'FLOAT',
		action='store', default = 1., help='unit sphere size')
	parser.add_option('--scale', dest='scale', metavar = 'FLOAT',
		action='store', default = 0.2, help='scaling of likelihoods')
	parser.add_option('--inverse', dest='inverse', action='store_true',
		default = False,
		help='inverse orientations for orientations distribution')
	return parser, parser.parse_args(argv)

def main():
	parser, (options, args) = parseOpts(sys.argv)
	if None in [options.distrib_gaussians_name,
		options.distrib_local_name, options.output]:
		print "error : missing option(s)"
		parser.print_help()
		sys.exit(1)
	if options.sulci is None:
		selected_sulci = None
	else:	selected_sulci = options.sulci.split(',')
	gaussians_distrib = io.read_segments_distrib(\
		options.distrib_gaussians_name, selected_sulci)
	if options.level == 'segments':
		local_distrib = io.read_segments_distrib(\
			options.distrib_local_name, selected_sulci)
	elif options.level == 'relations':
		local_distrib = io.read_relations_distrib(\
			options.distrib_local_name, selected_sulci)
	hie = aims.Reader().read(options.hie)

	scale = float(options.scale)
	size = float(options.size)

	opt = [gaussians_distrib, local_distrib,
		hie, options, options.output, selected_sulci]
	if options.level == 'segments':
		displayer = SegmentsDisplay(*opt)
	elif options.level == 'relations':
		displayer = RelationsDisplay(*opt)
	displayer.display()

if __name__ == '__main__' : main()
