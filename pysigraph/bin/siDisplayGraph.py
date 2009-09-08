#!/usr/bin/env python
import os, sys, exceptions, numpy
import qt, glob, re
from optparse import OptionParser
import sigraph
from sulci.common import io, add_translation_option_to_parser
import anatomist.direct.api as anatomist
from soma import aims
try:
	import datamind.io.old_csvIO as csv_io
except ImportError:
	print "import failed: disable datamind"


################################################################################
class DirectedObjectsFactory(object):
	def _motion(self, center, src, dir, scale=[1, 1, 1]):
		motion_s = aims.Motion()
		motion_s.scale(scale, [1] * 3)
		motion_t = aims.Motion()
		motion_t.setTranslation(center)
		if numpy.any(dir):
			dir_n = dir / numpy.sqrt((dir ** 2).sum())
			src_n = src / numpy.sqrt((src ** 2).sum())
			axis = (dir_n + src_n) / 2.
			quat = aims.Quaternion()
			quat.fromAxis(axis, numpy.pi)
			motion_r = aims.Motion(quat)
			motion = motion_t * motion_r * motion_s
		else:	motion = motion_t * motion_s
		return motion

class TwoDemiSpheresFactory(DirectedObjectsFactory):
	def __init__(self):
		s = aims.SurfaceGenerator.sphere((0, 0, 0), 2, 96)
		ds1 = aims.AimsSurfaceTriangle()
		ds2 = aims.AimsSurfaceTriangle()
		plane1 = (1, 1, 1, 0)
		plane2 = (-1, -1, -1, 0)
		borderline1 = aims.AimsTimeSurface_2()
		borderline2 = aims.AimsTimeSurface_2()
		aims.SurfaceManip.cutMesh(s, plane1, ds1, borderline1)
		aims.SurfaceManip.cutMesh(s, plane2, ds2, borderline2)
		self._dspheres = ds1, ds2

	def get(self, center):
		ds1, ds2 = self._dspheres
		ds1 = aims.AimsSurfaceTriangle(ds1)
		ds2 = aims.AimsSurfaceTriangle(ds2)
		motion = self._motion(center, numpy.array([1., 0., 0.]), dir)
		aims.SurfaceManip.meshTransform(ds1, motion)
		aims.SurfaceManip.meshTransform(ds2, motion)
		return ds1, ds2


class TwoConesFactory(DirectedObjectsFactory):
	def __init__(self):
		d1, d2 = (-5, 0, 0), (5, 0, 0)
		g = (0, 0, 0)
		c1 = aims.SurfaceGenerator.cone(d1, g, 1.5, 6, False, True)
		c2 = aims.SurfaceGenerator.cone(d2, g, 1.5, 6, False, True)
		self._cones = c1, c2

	def get(self, center, dir):
		c1, c2 = self._cones
		c1 = aims.AimsSurfaceTriangle(c1)
		c2 = aims.AimsSurfaceTriangle(c2)
		motion = self._motion(center, numpy.array([1., 0., 0.]), dir)
		aims.SurfaceManip.meshTransform(c1, motion)
		aims.SurfaceManip.meshTransform(c2, motion)
		return c1, c2


class TwoArrowsFactory(DirectedObjectsFactory):
	def __init__(self):
		d1, d2 = numpy.array([-1, 0, 0]), numpy.array([1, 0, 0])
		g = (0, 0, 0)
		c1 = aims.SurfaceGenerator.cone(g, d2 * 5, 0.7, 6, True, True)
		c2 = aims.SurfaceGenerator.cone(g, d1 * 5, 0.7, 6, True, True)
		c3 = aims.SurfaceGenerator.cylinder(g, d1, 0.2, 0.2, 6,
							False, True)
		c4 = aims.SurfaceGenerator.cylinder(g, d2, 0.2, 0.2, 6,
							False, True)
		self._cones = c1, c2
		self._cylinders = c3, c4
		self._src = d2

	def get(self, center, dir, size):
		(c1, c2), (c3, c4) = self._cones, self._cylinders
		c1 = aims.AimsSurfaceTriangle(c1)
		c2 = aims.AimsSurfaceTriangle(c2)
		c3 = aims.AimsSurfaceTriangle(c3)
		c4 = aims.AimsSurfaceTriangle(c4)
		scale = [size / 2. - 5., 1., 1.]
		motion1 = self._motion(center, self._src, dir)
		motion2 = self._motion(center + dir, self._src, dir)
		motion3 = self._motion(center + dir / 2., self._src, dir, scale)
		motion4 = self._motion(center + dir / 2., self._src, dir, scale)
		aims.SurfaceManip.meshTransform(c1, motion1)
		aims.SurfaceManip.meshTransform(c2, motion2)
		aims.SurfaceManip.meshTransform(c3, motion3)
		aims.SurfaceManip.meshTransform(c4, motion4)
		return c1, c2, c3, c4

class TwoConnectedSpheresFactory(DirectedObjectsFactory):
	def __init__(self):
		c1, c2 = numpy.array([0, 0, 0]), numpy.array([1, 0, 0])
		self._sphere = aims.SurfaceGenerator.sphere(c1, 2, 96)
		self._cylinder = aims.SurfaceGenerator.cylinder(c1, c2,
						0.1, 0.1, 6, False, True)
		self._src = c2

	def get(self, center, dir, size):
		s, c = self._sphere, self._cylinder
		s1 = aims.AimsSurfaceTriangle(s)
		s2 = aims.AimsSurfaceTriangle(s)
		c = aims.AimsSurfaceTriangle(c)
		scale = [size, 1., 1.]
		motion1 = self._motion(center, self._src, dir)
		motion2 = self._motion(center + dir, self._src, dir)
		motion3 = self._motion(center, self._src, dir, scale)
		aims.SurfaceManip.meshTransform(s1, motion1)
		aims.SurfaceManip.meshTransform(s2, motion2)
		aims.SurfaceManip.meshTransform(c, motion3)
		return s1, s2, c


################################################################################
class Display(object):
	def __init__(self, write=False):
		self._an = anatomist.Anatomist()
		self._awin = self._an.createWindow('3D')
		self._aobjects = []
		self._writer = aims.Writer()
		self._writemeshes = write

	def _write(self):
		if not self._writemeshes: return
		for i, ao in enumerate(self._aobjects):
			ao = ao.internalRep
			mesh = ao.surface().get()
			filename = '%s_%d.mesh' % (self._name, i)
			self._writer.write(mesh, filename)


class CsvDisplay(Display):
	'''Display information from sulcus csv.'''
	def __init__(self, write, dirname):
		Display.__init__(self, write)
		self._files = glob.glob(os.path.join(dirname, "siMorpho*.dat"))
		self._reader = csv_io.ReaderHeaderCsv()

	def display(self):
		for f in self._files:
			sulcus = re.sub('.dat', '', re.sub('.*siMorpho_', '',f))
			ind1, ind2 = sulcus.find('_'), sulcus.rfind('_')
			if ind1 != ind2: continue
			self._display_one_sulcus(f, sulcus)
		self._an.addObjects(self._aobjects, [self._awin])
		self._write()

	def _display_one_sulcus(self, csv, sulcus): pass


class GraphDisplay(Display):
	def __init__(self, write, graphs):
		Display.__init__(self, write)
		self._graphs = graphs

	def display(self):
		for g in self._graphs: self._display_one_graph(g)
		self._display_after_callback()
		self._an.addObjects(self._aobjects, [self._awin])
		self._write()

	def _display_after_callback(self): pass
	def _display_one_graph_callback(self, graph): pass


class FoldDisplay(GraphDisplay):
	def __init__(self, write, graphs):
		GraphDisplay.__init__(self, write, graphs)

	def _display_one_graph(self, graph):
		self._display_one_graph_callback(graph)
		for v in graph.vertices():
			if v.getSyntax() != 'fold': continue
			self._display_one_vertex(v)

	def _display_one_vertex(self, v):
		self._display_one_vertex_callback(v)
		for e in v.edges():
			v1, v2 = e.vertices()
			if v1.getSyntax() != 'fold' or v2.getSyntax() != 'fold':
				continue
			self._display_one_edge_callback(e, v1, v2)

	def _display_one_vertex_callback(self, v): pass
	def _display_one_edge_callback(self, e, v1, v2): pass


class JunctionDisplay(GraphDisplay):
	def __init__(self, write, graphs, hull):
		GraphDisplay.__init__(self, write, graphs)
		if hull:
			self._syntax = 'hull_junction'
		else:	self._syntax = 'junction'

	def _display_one_graph(self, graph):
		self._display_one_graph_callback(graph)
		for e in graph.edges():
			if e.getSyntax() != self._syntax: continue
			self._display_one_junction(e)
		
	def _display_one_junction(self, e): pass


class CorticalDisplay(GraphDisplay):
	def __init__(self, write, graphs):
		GraphDisplay.__init__(self, write, graphs)

	def _display_one_graph(self, graph):
		self._display_one_graph_callback(graph)
		for e in graph.edges():
			if e.getSyntax() != 'cortical': continue
			self._display_one_cortical(e)
		
	def _display_one_cortical(self, e): pass


class PureCorticalDisplay(GraphDisplay):
	def __init__(self, write, graphs):
		GraphDisplay.__init__(self, write, graphs)

	def _display_one_graph(self, graph):
		self._display_one_graph_callback(graph)
		for e in graph.edges():
			if e.getSyntax() != 'cortical': continue
			v1, v2 = e.vertices()
			pure_cortical = True
			for e1 in v1.edges():
				if e1.getSyntax() == 'cortical': continue
				ve1, ve2 = e1.vertices()
				if ve1 != v1: ve1, ve2 = v2, v1
				if ve1 != v1 or ve2 != v2: continue
				if e1.getSyntax() == 'junction':
					pure_cortical = False
					break
			if not pure_cortical: continue
			self._display_one_cortical(e)
		
	def _display_one_cortical(self, e): pass



################################################################################


class WireFrameDisplay(GraphDisplay):
	def __init__(self, write, graphs):
		GraphDisplay.__init__(self, write, graphs)
		self._name = 'wireframe'

	def _display_one_vertex_callback(self, v):
		g = numpy.asarray(v['refgravity_center'].list())
		s = aims.SurfaceGenerator.sphere(g, 2, 96)
		aas = self._an.toAObject(s)
		self._aobjects += [aas]

	def _display_one_edge_callback(self, e, v1, v2):
		g1 = numpy.asarray(v1['refgravity_center'].list())
		g2 = numpy.asarray(v2['refgravity_center'].list())
		c = aims.SurfaceGenerator.cylinder(g1, g2, 1, 1, 16,
							True, True)
		syntax = e.getSyntax()
		# add color
		material = anatomist.cpp.Material()
		color_map = {
			'plidepassage' : (1.0, 0., 0., 1.),
			'junction' : (0, 1., 0., 1.),
			'cortical' : (0, 0., 1., 1.)
		}
		go =  aims.Object({'diffuse' : (0, 0., 1., 1.)})
		c.header()['material'] = go
		material.set(go.get())
		ac = self._an.toAObject(c)
		ac.SetMaterial(material)
		self._aobjects += [ac]


class GravityCentersDisplay(FoldDisplay):
	def __init__(self, write, graphs):
		GraphDisplay.__init__(self, write, graphs)
		self._name = 'gravityclouds'
		hie_filename = os.path.join(aims.carto.Paths.shfjShared(),
					'nomenclature', 'hierarchy',
					'sulcal_root_colors.hie')
		transfile = os.path.join(aims.carto.Paths.shfjShared(),
				'nomenclature', 'translation',
				'sulci_model_noroots.trl')
		self._hie = aims.Reader().read(hie_filename)
		self._translator = sigraph.FoldLabelsTranslator(transfile)
		self._meshes = {}

	def _display_one_graph_callback(self, graph):
		self._translator.translate(graph)

	def _display_one_vertex_callback(self, v):
		# create nodes gravity spheres
		g = numpy.asarray(v['refgravity_center'].list())
		s = aims.SurfaceGenerator.sphere(g, 1, 96)
		name = v['name']
		if self._meshes.has_key(name):
			self._meshes[name] += s
		else:	self._meshes[name] = s

	def _display_after_callback(self):
		# add color
		for name, mesh in self._meshes.items():
			amesh = self._an.toAObject(mesh)
			material = anatomist.cpp.Material()
			color = self._hie.find_color(name)
			go =  aims.Object({'diffuse' : color})
			mesh.header()['material'] = go
			material.set(go.get())
			amesh.SetMaterial(material)
			self._aobjects += [amesh]


class DataCorticalDisplay(CorticalDisplay):
	def __init__(self, write, graphs):
		GraphDisplay.__init__(self, write, graphs)
		hie_filename = os.path.join(aims.carto.Paths.shfjShared(),
					'nomenclature', 'hierarchy',
					'sulcal_root_colors.hie')
		transfile = os.path.join(aims.carto.Paths.shfjShared(),
				'nomenclature', 'translation',
				'sulci_model_noroots.trl')
		self._hie = aims.Reader().read(hie_filename)
		self._translator = sigraph.FoldLabelsTranslator(transfile)
		self._meshes = {}
		self._data = {}
		self._data_min = numpy.inf
		self._data_max = -numpy.inf
		self._shapeFactory = TwoConnectedSpheresFactory()

	def _display_one_graph_callback(self, graph):
		self._translator.translate(graph)

	def _display_one_cortical(self, e):
		pfv1 = e.vertices()[0]
		pfv2 = e.vertices()[1]
		name1 = pfv1['name']
		name2 = pfv2['name']
		# create nodes gravity spheres
		g1 = numpy.asarray(pfv1['refgravity_center'].list())
		g2 = numpy.asarray(pfv2['refgravity_center'].list())
		dir = g2 - g1
		size = numpy.sqrt((dir ** 2).sum())
		s1, s2, c = self._shapeFactory.get(g1, dir, size)
		if self._meshes.has_key(name1):
			self._meshes[name1] += s1
		else:	self._meshes[name1] = s1
		if self._meshes.has_key(name2):
			self._meshes[name2] += s2
		else:	self._meshes[name2] = s2
		info = {'g1' : g1, 'g2' : g2, 'dist' : size, 'dir' : dir}
		d = self._compute_cortical_data(e, info)
		self._data[c] = d
		if d < self._data_min: self._data_min = d
		if d > self._data_max: self._data_max = d

	def _display_after_callback(self):
		# add color
		#for name, mesh in self._meshes.items():
		#	amesh = self._an.toAObject(mesh)
		#	material = anatomist.cpp.Material()
		#	color = self._hie.find_color(name)
		#	go =  aims.Object({'diffuse' : color})
		#	mesh.header()['material'] = go
		#	material.set(go.get())
		#	amesh.SetMaterial(material)
		#	self._aobjects += [amesh]
		an_shared_path = str(self._an.anatomistSharedPath())
		imgfilename = os.path.join(an_shared_path,'rgb', 'Blue-Red.ima')
		r = aims.Reader()
		img = r.read(imgfilename)
		img_size = img.getSizeX()
		h = {}
		for mesh, data in self._data.items():
			material = anatomist.cpp.Material()
			d = (data - self._data_min) / \
				(self._data_max - self._data_min)
			ds = int(d * (img_size - 1))
			if h.has_key(ds):
				h[ds] += mesh
			else:	h[ds] = mesh
		for d, mesh in h.items():
			color = img.value(d)
			color = [color.red() / 255., color.green() / 255.,
					color.blue() / 255., 1]
			go =  aims.Object({'diffuse' : color})
			mesh.header()['material'] = go
			material.set(go.get())
			amesh = self._an.toAObject(mesh)
			amesh.SetMaterial(material)
			self._aobjects += [amesh]

class CenterDistCorticalDisplay(DataCorticalDisplay):
	def __init__(self, write, graphs):
		DataCorticalDisplay.__init__(self, write, graphs)
		self._name = 'centerdist_cortical'

	def _compute_cortical_data(self, e, info):
		return info['dist']

class DiffGravityCentersDisplay(CorticalDisplay):
	def __init__(self, write, graphs):
		GraphDisplay.__init__(self, write, graphs)
		self._name = 'gravityclouds'
		hie_filename = os.path.join(aims.carto.Paths.shfjShared(),
					'nomenclature', 'hierarchy',
					'sulcal_root_colors.hie')
		transfile = os.path.join(aims.carto.Paths.shfjShared(),
				'nomenclature', 'translation',
				'sulci_model_noroots.trl')
		self._hie = aims.Reader().read(hie_filename)
		self._translator = sigraph.FoldLabelsTranslator(transfile)
		self._meshes = {}
		self._shapeFactory = TwoArrowsFactory()

	def _display_one_graph_callback(self, graph):
		self._translator.translate(graph)

	def _display_one_cortical(self, e):
		pfv1 = e.vertices()[0]
		pfv2 = e.vertices()[1]
		name1 = pfv1['name']
		name2 = pfv2['name']
		# create nodes gravity spheres
		g1 =  numpy.asarray(pfv1['refgravity_center'].list())
		g2 =  numpy.asarray(pfv2['refgravity_center'].list())
		dir = g2 - g1
		size = numpy.sqrt((dir ** 2).sum())
		c1, c2, c3, c4 = self._shapeFactory.get(g1, dir, size)
		c1 += c3
		c2 += c4
		if self._meshes.has_key(name1):
			self._meshes[name1] += c1
		else:	self._meshes[name1] = c1
		if self._meshes.has_key(name2):
			self._meshes[name2] += c2
		else:	self._meshes[name2] = c2

	def _display_after_callback(self):
		# add color
		for name, mesh in self._meshes.items():
			amesh = self._an.toAObject(mesh)
			material = anatomist.cpp.Material()
			color = self._hie.find_color(name)
			go =  aims.Object({'diffuse' : color})
			mesh.header()['material'] = go
			material.set(go.get())
			amesh.SetMaterial(material)
			self._aobjects += [amesh]


class IntersectionDisplay(JunctionDisplay):
	def __init__(self, write, graphs, hull):
		JunctionDisplay.__init__(self, write, graphs, hull)
		self._name = 'intersection'
		hie_filename = os.path.join(aims.carto.Paths.shfjShared(),
					'nomenclature', 'hierarchy',
					'sulcal_root_colors.hie')
		transfile = os.path.join(aims.carto.Paths.shfjShared(),
				'nomenclature', 'translation',
				'sulci_model_noroots.trl')
		self._hie = aims.Reader().read(hie_filename)
		self._translator = sigraph.FoldLabelsTranslator(transfile)
		self._meshes = {}
		self._shapeFactory = TwoConesFactory()
		

	def _display_one_graph_callback(self, graph):
		self._translator.translate(graph)

	def _display_one_junction(self, e):
		pfv1 = e.vertices()[0]
		pfv2 = e.vertices()[1]
		name1 = pfv1['name']
		name2 = pfv2['name']
		if name1 == name2: return
		if self._syntax == 'hull_junction':
			if name1 == 'unknown': name1 = name2
			if name2 == 'unknown': name2 = name1
		# create nodes gravity spheres
		e1 = numpy.asarray(e['refextremity1'].list())
		e2 = numpy.asarray(e['refextremity2'].list())
		g = (e1 + e2) / 2.
		try:
			g1 =  numpy.asarray(pfv1['refgravity_center'].list())
			g2 =  numpy.asarray(pfv2['refgravity_center'].list())
		except KeyError:
			g1, g2 = e1, e2
		dir = g2 - g1
		c1, c2 = self._shapeFactory.get(g, dir)
		if self._meshes.has_key(name1):
			self._meshes[name1] += c1
		else:	self._meshes[name1] = c1
		if self._meshes.has_key(name2):
			self._meshes[name2] += c2
		else:	self._meshes[name2] = c2

	def _display_after_callback(self):
		# add color
		for name, mesh in self._meshes.items():
			amesh = self._an.toAObject(mesh)
			material = anatomist.cpp.Material()
			color = self._hie.find_color(name)
			go =  aims.Object({'diffuse' : color})
			mesh.header()['material'] = go
			material.set(go.get())
			amesh.SetMaterial(material)
			self._aobjects += [amesh]


class GravityPureCorticalDisplay(PureCorticalDisplay):
	def __init__(self, write, graphs):
		PureCorticalDisplay.__init__(self, write, graphs)
		self._name = 'gravitypurecortical'
		hie_filename = os.path.join(aims.carto.Paths.shfjShared(),
					'nomenclature', 'hierarchy',
					'sulcal_root_colors.hie')
		transfile = os.path.join(aims.carto.Paths.shfjShared(),
				'nomenclature', 'translation',
				'sulci_model_noroots.trl')
		self._hie = aims.Reader().read(hie_filename)
		self._translator = sigraph.FoldLabelsTranslator(transfile)
		self._meshes = {}
		self._shapeFactory = TwoConesFactory()

	def _display_one_graph_callback(self, graph):
		self._translator.translate(graph)

	def _display_one_cortical(self, e):
		pfv1 = e.vertices()[0]
		pfv2 = e.vertices()[1]
		name1 = pfv1['name']
		name2 = pfv2['name']
		if name1 == name2: return
		# create nodes gravity spheres
		n1 =  numpy.asarray(e['refSS1nearest'].list())
		n2 =  numpy.asarray(e['refSS2nearest'].list())
		g1 =  numpy.asarray(pfv1['refgravity_center'].list())
		g2 =  numpy.asarray(pfv2['refgravity_center'].list())
		g = (n1 + n2) / 2.
		dir = g2 - g1
		c1, c2 = self._shapeFactory.get(g, dir)
		if self._meshes.has_key(name1):
			self._meshes[name1] += c1
		else:	self._meshes[name1] = c1
		if self._meshes.has_key(name2):
			self._meshes[name2] += c2
		else:	self._meshes[name2] = c2

	def _display_after_callback(self):
		# add color
		for name, mesh in self._meshes.items():
			amesh = self._an.toAObject(mesh)
			material = anatomist.cpp.Material()
			color = self._hie.find_color(name)
			go =  aims.Object({'diffuse' : color})
			mesh.header()['material'] = go
			material.set(go.get())
			amesh.SetMaterial(material)
			self._aobjects += [amesh]


class ExtremityCloudsDisplay(CsvDisplay):
	'''
    Based on siMorpho output without subject names.

    number : 1 or 2.
	'''
	def __init__(self, write, dirname, number):
		CsvDisplay.__init__(self, write, dirname)
		self._name = 'extremity%d' % number
		hie_filename = os.path.join(aims.carto.Paths.shfjShared(),
					'nomenclature', 'hierarchy',
					'sulcal_root_colors.hie')
		self._hie = aims.Reader().read(hie_filename)
		self._meshes = {}
		self._number = number

	def _display_one_sulcus(self, csv, sulcus):
		header_minf = { 'X' : range(29), 'Y' : [], 'INF': [] }
		db, header = self._reader.read(csv, header_minf)
		labels = header['labels']
		h = {}
		for i, l in enumerate(labels): h[l] = i
		if self._number == 1:
			ind = [h['extremity1x'], h['extremity1y'], \
						h['extremity1z']]
		elif self._number == 2:
			ind = [h['extremity2x'], h['extremity2y'], \
						h['extremity2z']]
		X = db.getX()
		data = X[:, ind]
		for i, e in enumerate(data):
			if not numpy.any(e): continue
			s = aims.SurfaceGenerator.sphere(e, 2, 96)
			amesh = self._an.toAObject(s)
			material = anatomist.cpp.Material()
			color = self._hie.find_color(sulcus)
			go =  aims.Object({'diffuse' : color})
			s.header()['material'] = go
			material.set(go.get())
			amesh.SetMaterial(material)
			self._aobjects += [amesh]


			
def parseOpts(argv):
	description = 'Display graphs with usefull representations.\n' \
		'Usage : siDisplayGraph.py [Options] graph1.arg graph2.arg ...'
	parser = OptionParser(description)
	add_translation_option_to_parser(parser)
	parser.add_option('-m', '--mode', dest='mode',
		metavar = 'FILE', action='store', default = None,
		help='wireframe, gravity_centers, extremity1, extremity2, ' \
			'hull_intersection, pure_cortical, ' \
			'diff_gravity_centers, centerdist_cortical.')
	parser.add_option('-d', '--dir', dest='dirname',
		metavar = 'DIR', action='store', default = None,
		help='directory with siMorpho output files ' \
				'(used by extremities mode).')
	parser.add_option('-w', '--write', dest='write',
		action='store_true', default = False,
		help='Write meshes')

	return parser, parser.parse_args(argv)


def main():
	parser, (options, args) = parseOpts(sys.argv)
	if None in [options.mode] or len(args) == 1:
		parser.print_help()
		sys.exit(1)

	graphnames = args[1:]
	graphs = io.load_graphs(options.transfile, graphnames)
	if options.mode == 'wireframe':
		disp = WireFrameDisplay(options.write, graphs)
	elif options.mode == 'gravity_centers':
		disp = GravityCentersDisplay(options.write, graphs)
	elif options.mode == 'extremity1':
		disp = ExtremityCloudsDisplay(options.write, options.dirname, 1)
	elif options.mode == 'extremity2':
		disp = ExtremityCloudsDisplay(options.write, options.dirname, 2)
	elif options.mode == 'intersection':
		disp = IntersectionDisplay(options.write, graphs, False)
	elif options.mode == 'hull_intersection':
		disp = IntersectionDisplay(options.write, graphs, True)
	elif options.mode == 'pure_cortical':
		disp = GravityPureCorticalDisplay(options.write, graphs)
	elif options.mode == 'diff_gravity_centers':
		disp = DiffGravityCentersDisplay(options.write, graphs)
	elif options.mode == 'centerdist_cortical':
		disp = CenterDistCorticalDisplay(options.write, graphs)
	disp.display()
	qt.qApp.exec_loop()

if __name__ == '__main__' : main()
