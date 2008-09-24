#!/usr/bin/env python
import os, sys, numpy
from optparse import OptionParser
from sulci.common import io
from sulci.models import check_same_distribution, distribution, \
				distribution_aims, distribution_fff
from sulci.registration import procrust
from soma import aims

unit_sphere = aims.SurfaceGenerator.sphere([0, 0, 0], 1, 4096)

################################################################################
class GlobalModelDisplay(object):
	def __init__(self, gaussians_distrib, orientations_distrib,
		hie, s,	scale, size, output, selected_sulci=None):
		self._gaussians_distrib = gaussians_distrib
		self._orientations_distrib = orientations_distrib
		self._hie = hie
		self._s = s
		self._scale = scale
		self._size = size
		self._output = output
		self._selected_sulci = selected_sulci
		self._sulci = set()
		self._sulci.update(gaussians_distrib['vertices'].keys())

	def display_one(self, sulcus, gd, od):
		mesher = OrientationMesher(od)
		if od.name() == 'bingham':
			mesh = mesher.mesh(self._scale, self._size/5., self._s)
		else:	mesh = mesher.mesh(self._scale, self._size, self._s)
		
		# add color
		color = self._hie.find_color(sulcus)
		go =  {'diffuse' : color}
		mesh.header()['material'] = go

		# translate
		pos = numpy.asarray(gd.mean()).ravel()
		motion = aims.Motion()
		motion.setTranslation(pos)
		aims.SurfaceManip.meshTransform(mesh, motion)

		# write
		aims.Writer().write(mesh, '%s_%s.mesh' % (self._output, sulcus))


class SegmentsDisplay(GlobalModelDisplay):
	def __init__(self, *args, **kwargs):
		GlobalModelDisplay.__init__(self, *args, **kwargs)

	def display(self):
		for sulcus in self._sulci:
			if self._selected_sulci is not None and \
				sulcus not in self._selected_sulci: continue
			try: gd = self._gaussians_distrib['vertices'][sulcus]
			except KeyError: continue
			try: od = self._orientations_distrib['vertices'][sulcus]
			except KeyError: continue
			self.display_one(sulcus, gd, od)


class RelationsDisplay(GlobalModelDisplay):
	def __init__(self, *args, **kwargs):
		GlobalModelDisplay.__init__(self, *args, **kwargs)

	def display(self):
		for relation in self._orientations_distrib['edges'].keys():
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
			try: od = self._orientations_distrib['edges'][relation]
			except KeyError: continue
			if label1 != label2:
				self.display_one(relation, gd1, gd2, od)
			else:	GlobalModelDisplay.display_one(self, label1,
								gd1, od)

	def display_one(self, relation, gd1, gd2, od):
		mesher = OrientationMesher(od)
		mesh = mesher.mesh(self._scale, self._size, self._s)

		# split the mesh in 2 parts along a plane directed by
		# the mean direction
		dir = numpy.hstack((od.GetMeanDirection(), 0))
		plane1 = tuple(dir)
		plane2 = tuple(-dir)
		mesh1 = aims.AimsSurfaceTriangle()
		mesh2 = aims.AimsSurfaceTriangle()
		borderline1 = aims.AimsTimeSurface_2()
		borderline2 = aims.AimsTimeSurface_2()
		aims.SurfaceManip.cutMesh(mesh, plane1, mesh1, borderline1)
		aims.SurfaceManip.cutMesh(mesh, plane2, mesh2, borderline2)

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
		aims.Writer().write(mesh1, '%s_%s,%s_1.mesh' % \
				(self._output, label1, label2))
		aims.Writer().write(mesh2, '%s_%s,%s_2.mesh' % \
				(self._output, label1, label2))


################################################################################
class LocalModelMesher(object):
	def __init__(self, od):
		self._od = od

class OrientationMesher(LocalModelMesher):
	def __init__(self, *args, **kwargs):
		LocalModelMesher.__init__(self, *args, **kwargs)

	def mesh(self, scale, size, s):
		mesh = aims.AimsTimeSurface_3(unit_sphere)
		for v in mesh.vertex():
			logli, li = self._od.likelihood(v * s)
			v += li * v / scale
			v *= size
		mesh.updateNormals()
		return mesh

# FIXME : old test, not already integrated in new API
class MatrixBinghamMesher(LocalModelMesher):
	def __init__(self, *args, **kwargs):
		LocalModelMesher.__init__(self, *args, **kwargs)

	def create_marginal_mesh(self, i):
		n = 100
		mesh = aims.AimsTimeSurface_3(unit_sphere)
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
				logli, li = self._od.likelihood(X)
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
		m[:3, :3] = self._od.M().T
		motion2 = aims.Motion(m.flatten())
		aims.SurfaceManip.meshTransform(c, motion * motion2)

		c.header()['material'] = go
		aims.Writer().write(c, '%s_axes_%s.mesh' % \
				(self._p._output, self._sulcus))



################################################################################
def parseOpts(argv):
	description = 'Display learned bayesian local models ' \
			'(one for each sulci).'
	parser = OptionParser(description)
	parser.add_option('--distrib-gaussians', dest='distrib_gaussians_name',
		metavar = 'FILE', action='store', default = None,
		help='distribution models')
	parser.add_option('--distrib-orientations',
		dest='distrib_orientations_name', metavar = 'FILE',
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
		default = False, help='inverse orientations')
	return parser, parser.parse_args(argv)

def main():
	parser, (options, args) = parseOpts(sys.argv)
	if None in [options.distrib_gaussians_name,
		options.distrib_orientations_name, options.output]:
		print "error : missing option(s)"
		parser.print_help()
		sys.exit(1)
	if options.sulci is None:
		selected_sulci = None
	else:	selected_sulci = options.sulci.split(',')
	gaussians_distrib = io.read_segments_distrib(\
		options.distrib_gaussians_name, selected_sulci)
	if options.level == 'segments':
		orientations_distrib = io.read_segments_distrib(\
			options.distrib_orientations_name, selected_sulci)
	elif options.level == 'relations':
		orientations_distrib = io.read_relations_distrib(\
			options.distrib_orientations_name, selected_sulci)
	hie = aims.Reader().read(options.hie)

	if options.inverse:
		s = -1
	else:	s = 1
	scale = float(options.scale)
	size = float(options.size)

	opt = [gaussians_distrib, orientations_distrib,
		hie, s, scale, size, options.output, selected_sulci]
	if options.level == 'segments':
		displayer = SegmentsDisplay(*opt)
	elif options.level == 'relations':
		displayer = RelationsDisplay(*opt)
	displayer.display()

if __name__ == '__main__' : main()
