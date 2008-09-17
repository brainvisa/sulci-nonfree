#!/usr/bin/env python
import os, sys, numpy
from optparse import OptionParser
from sulci.common import io
from sulci.models import check_same_distribution, distribution, \
				distribution_aims, distribution_fff
from sulci.registration import procrust
from soma import aims

unit_sphere = aims.SurfaceGenerator.sphere([0, 0, 0], 1, 4096)

class Display(object):
	def __init__(self, gd, od, hie, sulcus, output, scale, size, s):
		self._mean = numpy.asarray(gd.mean()).ravel()
		self._od = od
		self._hie = hie
		self._sulcus = sulcus
		self._output = output
		self._scale = scale
		self._size = size
		self._s = s


class OrientationDisplay(Display):
	def __init__(self, *args, **kwargs):
		Display.__init__(self, *args, **kwargs)

	def display(self):
		color = self._hie.find_color(self._sulcus)
		go =  {'diffuse' : color}
		mesh = aims.AimsTimeSurface_3(unit_sphere)
		mesh.header()['material'] = go
		for v in mesh.vertex():
			logli, li = self._od.likelihood(v * self._s)
			v += li * v / self._scale
			v *= self._size
		motion = aims.Motion()
		motion.setTranslation(self._mean)
		aims.SurfaceManip.meshTransform(mesh, motion)
		aims.Writer().write(mesh, '%s_%s.mesh' % \
				(self._output, self._sulcus))

class MatrixBinghamDisplay(Display):
	def __init__(self, *args, **kwargs):
		Display.__init__(self, *args, **kwargs)

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
		return mesh

	def display(self):
		motion = aims.Motion()
		motion.setToIdentity()
		motion.setTranslation(self._mean)
		color = self._hie.find_color(self._sulcus)
		go =  {'diffuse' : color}

		for i in range(3):
			mesh = self.create_marginal_mesh(i)
			mesh.header()['material'] = go
			aims.SurfaceManip.meshTransform(mesh, motion)
			aims.Writer().write(mesh, '%s_%s_%d.mesh' % \
					(self._output, self._sulcus, i))
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
				(self._output, self._sulcus))



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
		action='store', default = 5., help='unit sphere size')
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
	orientations_distrib = io.read_segments_distrib(\
		options.distrib_orientations_name, selected_sulci)
	hie = aims.Reader().read(options.hie)

	sulci = set()
	sulci.update(gaussians_distrib['vertices'].keys())
	sulci.update(orientations_distrib['vertices'].keys())

	if options.inverse:
		s = -1
	else:	s = 1
	scale = float(options.scale)
	size = float(options.size)

	for sulcus in sulci:
		if selected_sulci is not None and \
			sulcus not in selected_sulci: continue
		try: gd = gaussians_distrib['vertices'][sulcus]
		except KeyError: continue
		try: od = orientations_distrib['vertices'][sulcus]
		except KeyError: continue

		opt = [gd, od, hie, sulcus, options.output, scale, size, s]
		if od.name() == 'matrix_bingham':
			displayer = MatrixBinghamDisplay(*opt)
		else:	displayer = OrientationDisplay(*opt)
		displayer.display()

if __name__ == '__main__' : main()
