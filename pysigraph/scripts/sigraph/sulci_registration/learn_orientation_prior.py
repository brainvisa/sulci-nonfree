#!/usr/bin/env python

import sys, os, pprint, re
import numpy
from optparse import OptionParser
from soma import aims, aimsalgo
from sulci.common import io, add_translation_option_to_parser
from sulci.registration import orientation
from sulci.models.distribution import VonMisesFisher, Kent
from sulci.models.distribution_aims import Bingham, MatrixBingham, \
						MatrixVonMisesFisher



def get_orientations_from_skeletons(sulci, graphs, skelnames,
				selected_sulci, options):
	reader = aims.Reader()

	orientations = dict((sulcus, {}) for sulcus in sulci)
	for i in range(len(graphs)):
		graph = graphs[i]
		subject = os.path.splitext(os.path.basename(\
			graph['aims_reader_filename']))[0]
		skel = reader.read(skelnames[i])
		gXname = '%s_gX.ima' % subject
		gYname = '%s_gY.ima' % subject
		gZname = '%s_gZ.ima' % subject
		if os.path.exists(gXname) and os.path.exists(gYname) and \
			os.path.exists(gZname):
			reader = aims.Reader()
			print "find grad X,Y,Z maps for subject '%s'" % subject
			gradsX = reader.read(gXname)
			gradsX = aims.AimsData_FLOAT(gradsX)
			gradsY = reader.read(gYname)
			gradsY = aims.AimsData_FLOAT(gradsY)
			gradsZ = reader.read(gZname)
			gradsZ = aims.AimsData_FLOAT(gradsZ)
		else:
			print "compute grad X,Y,Z maps for subject '%s'" % \
								subject
			fat = aims.FoldGraphAttributes(skel, graph)
			fat.prepareBrainDepthMap()
			gradsX = fat.getBrainDepthGradX()
			gradsY = fat.getBrainDepthGradY()
			gradsZ = fat.getBrainDepthGradZ()
			writer = aims.Writer()
			writer.write(gradsX, gXname)
			writer.write(gradsY, gYname)
			writer.write(gradsZ, gZname)
		motion = aims.GraphManip.talairach(graph)
		for sulcus in sulci: orientations[sulcus][graph] = [], []
		for v in graph.vertices():
			if v.getSyntax() != 'fold': continue
			sulcus = v['name']
			if (selected_sulci is not None) and \
				(sulcus not in selected_sulci): continue
			map = v['aims_ss'].get()
			X = orientations[sulcus][graph][0]
			W = orientations[sulcus][graph][1]
			for p in map[0].keys():
				gx = gradsX.value(p[0], p[1], p[2])
				gy = gradsY.value(p[0], p[1], p[2])
				gz = gradsZ.value(p[0], p[1], p[2])
				g = motion.transform_normal(\
					gx, gy, gz).arraydata().copy()
				w = numpy.linalg.norm(g)
				if numpy.isnan(w) or w == 0 or w > 5: continue
				g /= w
				X.append(g)
				W.append(w)
	return orientations


def get_orientations_from_graphs(sulci, graphs, skelnames,
				selected_sulci, options):
	orientations = dict((sulcus, {}) for sulcus in sulci)
	for graph in graphs:
		ors, ws = [], []
		for sulcus in sulci: orientations[sulcus][graph] = [], []
		for v in graph.vertices():
			if v.getSyntax() != 'fold': continue
			sulcus = v['name']
			if (selected_sulci is not None) and \
				(sulcus not in selected_sulci): continue
			if options.data_type == 'orientation':
				o, w = orientation.get_sulci_rotation_axe(v)
			elif options.data_type == 'refhull_normal':
				o, w = v['hull_normal'], v['hull_normal_weight']
			elif options.data_type == 'coordinate_system':
				h = v['hull_normal'].arraydata()
				n = v['refnormal'].arraydata()
				o = numpy.vstack([h, n]).ravel()
				w = v['refsize']
			elif options.data_type == 'refnormal':
				o = v['refnormal'].arraydata()
				w = v['refsize']
			if o is None: continue
			orientations[sulcus][graph][0].append(o)
			orientations[sulcus][graph][1].append(w)
	return orientations


################################################################################
class Compute(object):
	def __init__(self, graphs, distribdir, sulci,
		orientations, options, selected_sulci):
		self._graphs = graphs
		self._distribdir = distribdir
		self._sulci = sulci
		self._orientations = orientations
		self._options = options
		self._selected_sulci = selected_sulci
		self._init()

	def _init(self):
		prefix = self._distribdir
		try:	os.mkdir(prefix)
		except OSError, e:
			print e
			sys.exit(1)

	def compute_orientations(self):
		# first mean orientation estimation to reorient direction of
		# orientations
		if self._options.data_type == 'orientation':
			mean_orientations = {}
			for sulcus in self._sulci:
				ors, ws = flatten(self._orientations[sulcus],
								self._graphs)
				if len(ws) == 0:
					mean_orientations[sulcus]= None
				else:
					o = numpy.dot(ws, ors) / numpy.sum(ws)
					mean_orientations[sulcus] = o

		selected_orientations = {}
		# one orientation per label or per node
		if self._options.splitmode == 'labels':
			for sulcus in self._sulci:
				ors2, ws2 = [], []
				for graph, (ors, ws) in  \
					self._orientations[sulcus].items():
					if graph not in self._graphs: continue
					if len(ws) == 0: continue
					ors = numpy.vstack(ors)
					if self._options.data_type == \
						'orientation':
						o = mean_orientations[sulcus]
						ors = orientation.reorient( \
								ors, o)
					o = numpy.dot(ws, ors) / numpy.sum(ws)
					w = numpy.sum(ws)
					ors2.append(o)
					ws2.append(w)
				selected_orientations[sulcus] = (ors2, ws2)
		elif self._options.splitmode in ['nodes', 'voxels']:
			for sulcus in self._sulci:
				try:	o_sulcus = self._orientations[sulcus]
				except KeyError: continue
				t = flatten(o_sulcus, self._graphs)
				selected_orientations[sulcus] = t
		return selected_orientations

	def fit(self):
		level = self.get_level()
		# estimates and writes models
		h = {'level' : level, 'data_type' : self._options.data_type,
			'files' : {}}
		dir = os.path.dirname(self._distribdir)
		for sulcus in self._sulci:
			X, W = self.get_learn_data(sulcus)
			if X is None: continue
			distr = self.get_distr()
			distr.fit(X, W)
			filename = io.node2densityname(self._distribdir,
				self._options.model_type, sulcus)
			distr.write(filename)
			relfilename = re.sub('^%s%s' % (dir, \
					os.path.sep), '', filename)
			h['files'][sulcus] = (self._options.model_type, \
							relfilename)
		summary_file = self._distribdir + '.dat'
		io.write_pp('distributions', summary_file, h)

		
class ComputeOrientations(Compute):
	def __init__(self, *args, **kwargs):
		Compute.__init__(self, *args, **kwargs)

	def compute(self):
		self._selected_orientations = self.compute_orientations()
		self.fit()

	def get_distr(self):
		if self._options.model_type == 'von_mises_fisher':
			Distr = VonMisesFisher
		elif self._options.model_type == 'kent':
			Distr = Kent
		elif self._options.model_type == 'bingham':
			Distr = Bingham
		return Distr()

	def get_level(self):
		if self._options.splitmode == 'nodes': level = 'segments'
		elif self._options.splitmode == 'voxels': level = 'voxels'
		elif self._options.splitmode == 'labels': level = 'sulci'
		return level

	def get_learn_data(self, sulcus):
		try:	ors, ws = self._selected_orientations[sulcus]
		except KeyError:
			return None, None
		if len(ors) == 0: return None, None
		ors = numpy.vstack(ors)
		if self._options.data_type == 'orientation':
			ors = orientation.reorient(ors, o)
		ws = numpy.array(ws)
		return ors, ws

class ComputeCoordinateSystem(Compute):
	def __init__(self, *args, **kwargs):
		Compute.__init__(self, *args, **kwargs)

	def compute(self):
		self._selected_orientations = self.compute_orientations()
		self.fit()

	def get_distr(self):
		if self._options.model_type == 'matrix_bingham':
			Distr = MatrixBingham
		elif self._options.model_type == 'matrix_von_mises_fisher':
			Dist = MatrixVonMisesFisher
		return Distr((2, 3))

	def get_level(self):
		if self._options.splitmode == 'nodes': level = 'segments'
		elif self._options.splitmode == 'labels': level = 'sulci'
		return level

	def get_learn_data(self, sulcus):
		try:	ors, ws = self._selected_orientations[sulcus]
		except KeyError:
			return None, None
		if len(ors) == 0: return None, None
		ors = numpy.vstack(ors)
		ws = numpy.array(ws)
		return ors, ws



################################################################################
def compute(graphs, distribdir, sulci, orientations,
					options, selected_sulci):
	opt = [graphs, distribdir, sulci, orientations, options, selected_sulci]
	if options.data_type in ['orientation', 'refhull_normal', 'refnormal']:
		c = ComputeOrientations(*opt)
	elif options.data_type in ['coordinate_system']:
		c = ComputeCoordinateSystem(*opt)
	c.compute()


################################################################################
def flatten(orientations_sulcus, graphs):
	ors2, ws2 = [], []
	for graph, (ors, ws) in orientations_sulcus.items():
		if graph not in graphs: continue
		ors2 += ors
		ws2 += ws
	return (ors2, ws2)

################################################################################
# main + options
def parseOpts(argv):
	description = 'compute label priors\n' \
		'./learn_priors.py [OPTIONS] --splitmode [labels,nodes] ' + \
		'graph1.arg graph2.arg ...\n' + \
		'./learn_priors.py [OPTIONS] --splitmode voxels ' + \
		'graph1.arg graph2.arg ... skeleton1.ima skeleton2.ima ...\n'
	parser = OptionParser(description)
	add_translation_option_to_parser(parser)
	parser.add_option('-m', '--graphmodel', dest='graphmodelname',
		metavar = 'FILE', action='store',
		default = 'bayesian_graphmodel.dat', help='bayesian model : '\
			'graphical model structure (default : %default)')
	parser.add_option('--splitmode', dest='splitmode',
		metavar='FILE', action='store', default = 'labels',
		help="splitmode : 'labels', 'nodes', 'voxels' (default : " + \
		"%default)")
	parser.add_option('--datatype', dest='data_type',
		metavar='FILE', action='store', default = 'orientation',
		help="datype: 'orientation' (for local rotation prior), " + \
		"'refhull_normal' (for local hull_normal learning) : " + \
		"it exists some differencies along F.C.L sulci (branches " + \
		"and broca area), 'refnormal' : (normals of sulci), " + \
		"'coordinate_system' (refhull_normal, refnormal, refdirection)")
	parser.add_option('-d', '--distribdir', dest='distribdir',
		metavar = 'FILE', action='store',
		default = 'bayesian_orientations_distribs',
		help='output distribution directory (default : %default).' \
			'A file named FILE.dat is created to store ' \
			'labels/databases links.')
	parser.add_option('-s', '--sulci', dest='sulci',
		metavar = 'LIST', action='store', default = None,
		help='tag only specified manually tagged sulci.')
	parser.add_option('--mode', dest='mode',
		metavar = 'FILE', action='store', default = 'normal',
		help="'normal' : compute spams on given graphs, 'loo' : " + \
		"leave one out on graphs : create several models " + \
		"(default : %default), all given reference FILE options " + \
		"must be located in './all/' relative directory and similar " +\
		"data can be found in './cv_*/' directories relative to " + \
		"leave one out graphs folds.")
	parser.add_option('--model-type', dest='model_type',
		metavar = 'TYPE', action='store', default = None,
		help="- model type to learn orientations (default " + \
		"von_mises_fisher) : [von_mises_fisher, kent, bingham], with "+\
		"von_mises_fisher (univariate gaussian on sphere), kent " +\
		"(bivariate gaussian on sphere), bingham (gaussian on axes " + \
		"data).\nmodel type to learn coordinate system (default " + \
		"matrix_bingham) : [matrix_bingham, matrix_von_mises_fisher] "+\
		"with matrix_binghan (gaussian on axes set with fixed angles "+\
		"between them), matrix_von_mises_fisher (gaussian on " + \
		"directions set or rotations with fixed angles with fixed " + \
		"angles between them).")

	return parser, parser.parse_args(argv)


def main():
	parser, (options, args) = parseOpts(sys.argv)

	inputs = args[1:]
	if len(inputs) == 0:
		print "error: missing input graph(s)/skeleton(s)"
		parser.print_help()
		sys.exit(1)
	if not (options.splitmode in ['labels', 'nodes', 'voxels']):
		print "error: unknown split mode '%s'" % options.splitmode
		parser.print_help()
		sys.exit(1)
	if options.data_type not in ['orientation', 'refhull_normal',
		'refnormal', 'coordinate_system']:
		print "error: unknown data mode '%s'" % options.data_type
		parser.print_help()
		sys.exit(1)
	if options.splitmode in ['labels', 'nodes']:
		graphnames, skelnames = inputs, None
	elif options.splitmode == 'voxels':
		if options.data_type == 'coordinate_system':
			print "error: unavailable splitmode 'voxels' for " + \
				"datatype 'coordinate_system'"
			parser.print_help()
			sys.exit(1)
		s = len(inputs)
		graphnames, skelnames = inputs[:s / 2], inputs[s / 2:]
	if options.data_type in ['orientation', 'refhull_normal', 'refnormal']:
		authorized_distr = ['von_mises_fisher', 'kent', 'bingham']
		if options.model_type is None:
			options.model_type = 'von_mises_fisher'
	elif options.data_type == 'coordinate_system':
		authorized_distr = ['matrix_bingham', 'matrix_von_mises_fisher']
		if options.model_type is None:
			options.model_type = 'matrix_bingham'
	if options.model_type not in authorized_distr:
		print "error: '%s' unknown model type or unauthorized " + \
			"type for this datatype : '%s'" % (options.model_type,
			options.data_type)
		parser.print_help()
		sys.exit(1)

	if options.sulci is None:
		selected_sulci = None
	else:	selected_sulci = options.sulci.split(',')

	# reading
	graphs = io.load_graphs(options.transfile, graphnames)
	graphmodel = io.read_graphmodel(options.graphmodelname)
	sulci = graphmodel['vertices'].keys()

	opt =  (sulci, graphs, skelnames, selected_sulci, options)
	if skelnames:
		orientations = get_orientations_from_skeletons(*opt)
	else:	orientations = get_orientations_from_graphs(*opt)

	if options.mode == 'normal' :
		compute(graphs, options.distribdir,
			sulci, orientations, options, selected_sulci)
	elif options.mode == 'loo' :
		print "-- all --"
		distribdir = os.path.join('all', options.distribdir)
		compute(graphs, distribdir,
				sulci, orientations, options, selected_sulci)
		for i in range(len(graphs)):
			subgraphs = graphs[:i] + graphs[i+1:]
			dir = 'cv_%d' % i
			print '-- %s --' % dir
			distribdir = os.path.join(dir, options.distribdir)
			compute(subgraphs, distribdir,
				sulci, orientations, options, selected_sulci)
	else:
		print "error : '%s' unknown mode" % options.mode

if __name__ == '__main__' : main()
