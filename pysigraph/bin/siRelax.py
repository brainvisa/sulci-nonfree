#!/usr/bin/env python2

from __future__ import print_function
import sys
import numpy
import sigraph
from soma import aims


class WeightsModifier(object):
	def __init__(self, mgraph, cgraph):
		self._mgraph = mgraph
		self._cgraph = cgraph

	def init(self): pass

	def _normalized_vertices(self, weightsum):
		for v in self._mgraph.vertices():
			topmodel = v['model'].topModel()
			if topmodel is not None:
				w = topmodel.weight() / weightsum
				topmodel.setWeight(w)

	def _normalized_edges(self, weightsum):
		for e in self._mgraph.edges():
			topmodel = e['model'].topModel()
			if topmodel is not None:
				w = topmodel.weight() / weightsum
				topmodel.setWeight(w)

	def _sigmoid(self, vertex, T):
		w = self._weights[vertex]
		Wmin = min(1, w)
		Wmax = max(1, w)
		y = (Wmax - Wmin) / (1. + numpy.exp((820. - T) / 80.)) + Wmin
		return y

	def _normalized_all(self, weightsum):
		self._normalized_vertices(weightsum)
		self._normalized_edges(weightsum)

	def isTimeChanging(self):
		return True


class NoneWeightsModifier(WeightsModifier):
	def __init__(self, mgraph, cgraph):
		WeightsModifier.__init__(self, mgraph, cgraph)

	def modify(self): pass
	
        def isTimeChanging(self):
                return False

class VerticesWeightsModifier(WeightsModifier):
	def __init__(self, mgraph, cgraph):
		WeightsModifier.__init__(self, mgraph, cgraph)

	def modify(self):
		self.init()
		weightsum = 0.
		for v in self._mgraph.vertices():
			topmodel = v['model'].topModel()
			if topmodel is not None:
				w = self._compute_weight(v)
				topmodel.setWeight(w)
				weightsum += w
		vertices_n = float(len(self._mgraph.vertices()))
		self._normalized_vertices(weightsum / vertices_n)


class GraphWeightsModifier(WeightsModifier):
	def __init__(self, mgraph, cgraph):
		WeightsModifier.__init__(self, mgraph, cgraph)

	def modify(self):
		self.init()
		weightsum = 0.
		graphobjects = self._mgraph.vertices().list() + \
				self._mgraph.edges().list()
		for go in graphobjects:
			topadap = go['model']
			topmodel = topadap.topModel()
			if topmodel is not None:
				w = self._compute_weight(go)
				topmodel.setWeight(w)
				weightsum += w
		vertices_n = float(len(self._mgraph.vertices()))
		edges_n = float(len(self._mgraph.edges()))
		self._normalized_all(weightsum / (edges_n + vertices_n))


class NeighboursWeightsModifier(VerticesWeightsModifier):
	def __init__(self, mgraph, cgraph):
		VerticesWeightsModifier.__init__(self, mgraph, cgraph)
		self._weights = {}
		for v in self._mgraph.vertices():
			self._weights[v] = len(v.edges())

	def _compute_weight(self, vertex):
		return self._weights[vertex]

	@classmethod
	def isTimeChanging(self):
		return False

	@classmethod
	def name(self):
		return "Neighbours-T-Weights"


class NeighboursWeightsTemperatureModifier(NeighboursWeightsModifier):
	def __init__(self, mgraph, cgraph, anneal):
		NeighboursWeightsModifier.__init__(self, mgraph, cgraph)
		self._anneal = anneal

	@classmethod
	def isTimeChanging(self):
		return True

	def _compute_weight(self, vertex):
		T = self._anneal.temp()
		return self._sigmoid(vertex, T)


class VolumeWeightsModifier(GraphWeightsModifier):
	def __init__(self, mgraph, cgraph):
		GraphWeightsModifier.__init__(self, mgraph, cgraph)
		self._weights = {}
		graphobjects = self._mgraph.vertices().list() + \
				self._mgraph.edges().list()
		for go in graphobjects: self._init_weight(go)

	def _init_weight(self, graphobject):
		model = graphobject['model']
		if not model.isAdaptive(): return
		al = model.model()
		descr = al.getAdapDescr()
		el = al.workEl()
		if isinstance(graphobject, aims.Vertex):
			val = el.getMean(18) # volume
		elif isinstance(graphobject, aims.Edge):
			val = el.getMean(20) # junction_size
		self._weights[graphobject] = val

	def _compute_weight(self, graphobject):
		return self._weights[graphobject]

	@classmethod
	def isTimeChanging(self):
		return False

	@classmethod
	def name(self):
		return "Volume-Weights"


class VolumeWeightsTemperatureModifier(VolumeWeightsModifier):
	def __init__(self, mgraph, cgraph, anneal):
		VolumeWeightsModifier.__init__(self, mgraph, cgraph)
		self._anneal = anneal

	def _compute_weight(self, graphobject):
		w = VolumeWeightsModifier._compute_weight(self, graphobject)
		self._sigmoid(w)

	@classmethod
	def isTimeChanging(self):
		return True

	@classmethod
	def name(self):
		return "Volume-T-Weights"


class SizeWeightsModifier(GraphWeightsModifier):
	def __init__(self, mgraph, cgraph):
		GraphWeightsModifier.__init__(self, mgraph, cgraph)
		self._weights = {}
		graphobjects = self._mgraph.vertices().list() + \
				self._mgraph.edges().list()
		for go in graphobjects: self._init_weight(go)

	def _init_weight(self, graphobject):
		model = graphobject['model']
		if not model.isAdaptive(): return
		al = model.model()
		descr = al.getAdapDescr()
		el = al.workEl()
		if isinstance(graphobject, aims.Vertex):
		# junction_hull_size (voxel or real size)
			val = el.getMean(26)
		elif isinstance(graphobject, aims.Edge):
			val = el.getMean(20) # junction_size
		self._weights[graphobject] = val

	def _compute_weight(self, graphobject):
		return self._weights[graphobject]

	@classmethod
	def isTimeChanging(self):
		return False

	@classmethod
	def name(self):
		return "Size-Weights"


class SizeWeightsTemperatureModifier(SizeWeightsModifier):
	def __init__(self, mgraph, cgraph, anneal):
		SizeWeightsModifier.__init__(self, mgraph, cgraph)
		self._anneal = anneal

	def _compute_weight(self, graphobject):
		w = SizeWeightsModifier._compute_weight(self, graphobject)
		self._sigmoid(w)

	@classmethod
	def isTimeChanging(self):
		return True

	@classmethod
	def name(self):
		return "Size-T-Weights"


class RatioWeightsModifier(GraphWeightsModifier):
	'''weights : (1-lambda) for sulci, (lambda) for relations'''
	def __init__(self, mgraph, cgraph):
		GraphWeightsModifier.__init__(self, mgraph, cgraph)
		self._lambda = 0.5

	def init(self):
		def get_relations(cliques):
			'''get fakeRel, relations (edges)'''
			return [c for c in cliques \
				if (c.getSyntax() == 'fakeRel' \
				or c['model_type'] == 'random_edge')]
		def get_sulci(cliques):
			'''get sulci (vertices)'''
			return [c for c in cliques \
				if (c['model_type'] == 'random_vertex')]
		def get_nothing(cliques):
			return []

		cls = self._cgraph.cliques()

		# choose cliques to delete
		if self._lambda == 0: get_todel = get_relations
		elif self._lambda == 1: get_todel = get_sulci
		else: get_todel = get_nothing
		todel = get_todel(cls)
		for c in todel: del cls[c]
		for v in self._cgraph.vertices():
			cls = v["cliques"]
			todel = get_todel(cls)
			for c in todel: del cls[c]

	def setLambda(self, x):
		self._lambda = x

	def _compute_weight(self, graphobject):
		if isinstance(graphobject, aims.Vertex):
			return 1. - self._lambda
		elif isinstance(graphobject, aims.Edge):
			return self._lambda

	@classmethod
	def isTimeChanging(self):
		return False

	@classmethod
	def name(self):
		return "Ratio-Weights"


class ThresholdWeightsModifier(GraphWeightsModifier):
	'''if reliance weight < lambda : weight = 0, else nothing.'''
	def __init__(self, mgraph, cgraph):
		GraphWeightsModifier.__init__(self, mgraph, cgraph)
		self._lambda = 0.5

	def init(self):
		cls = self._cgraph.cliques()
		finder = sigraph.FoldFinder(self._mgraph)
		todel = []
		for c in cls:
			o = finder.selectModel(c)
			go = o["model"].graphObject()
			if go is None: continue
			model = go['model']
			if not model.isAdaptive(): continue
			al = model.model()
			el = al.workEl()
			w = el.relianceWeight()
			if w < self._lambda: todel += [c]
		todel_edges_n = 0
		todel_vertices_n = 0
		for c in todel:
			model_type = c['model_type']
			if model_type == 'random_edge':
				todel_edges_n += 1 
			elif model_type == 'random_vertex':
				todel_vertices_n += 1 
			del cls[c]
		for v in self._cgraph.vertices():
			cls = v["cliques"]
			for c in cls:
				if c in todel: del cls[c]
		print("number of truncated models (vertices, edges) : " \
			"%s (%s, %s)" % (len(todel), todel_vertices_n, \
					todel_edges_n))

	def setLambda(self, x):
		self._lambda = x

	def _compute_weight(self, graphobject):
		model = graphobject['model']
		if not model.isAdaptive(): return
		al = model.model()
		el = al.workEl()
		w = el.relianceWeight()
		if w < self._lambda:
			return 0.
		else:	return 1.

	@classmethod
	def isTimeChanging(self):
		return False

	@classmethod
	def name(self):
		return "Threshold-Weights"



class WeightedAnnealExtension(sigraph.AnnealExtension):
	def __init__(self, anneal, Modifier):
		sigraph.AnnealExtension.__init__(self, anneal)
		self._weightModifier = Modifier(anneal.rGraph(), \
					anneal.cGraph(), anneal)

	def specialStep(self, passnum=0):
		self._weightModifier.modify()

	def ntrans(self):
		return 0

	def maxTrans(self):
		return 0

	def name(self):
		return self._weightModifier.name()


def weightsModifierFactory(name):
	h = {
		None : NoneWeightsModifier,
		'volume' : VolumeWeightsModifier,
		'volumeT' : VolumeWeightsTemperatureModifier,
		'ratio' : RatioWeightsModifier,
		'threshold' : ThresholdWeightsModifier,
		'size' : SizeWeightsModifier,
		'sizeT' : SizeWeightsTemperatureModifier,
		'neighbours' : NeighboursWeightsModifier,
		'neighboursT' : NeighboursWeightsTemperatureModifier,
	}
	return h[name]

from optparse import OptionParser

def parseOpts(argv):
	description = 'Annealing to label sulci.'
	parser = OptionParser(description)
	parser.add_option('-c', '--config', dest='config',
		metavar='FILE', action='store', default=None,
		help='siRelax config file')
	parser.add_option('-w', '--weight', dest='modifier',
		metavar='MODE', action='store', default=None,
		help='mode to weight models (sulci and relations) : volume, ' \
			'size, neighbours, volumeT, sizeT, neighboursT, ' \
			'ratio (0 : only sulci, 1 : only relations, 0.5 : ' \
			' classical model), treshold (w < l => w = 0)')
	parser.add_option('-l', '--lambda', dest='_lambda',
		metavar='FILE', action='store', default=None,
		help='lambda parameter for ratio weights modifier : (1-lambda)'
			' for sulci, (lambda) for relations. For threshold '
			'modifier : truncate threshold.')
	return parser, parser.parse_args(argv)


def print_potentials(fg):
	cls = fg.cliques()
	for c in cls:
		try:
			p = c["potential"]
		except: p = numpy.nan
		if c.getSyntax() != 'clique':
			print("fakeRel\t%f" % p)
		else:
			try:
				print("%s-%s\t%f" % (c['label1'], \
						c['label2'], p))
			except: print("%s\t%f" % (c['label'], p))

def main():
	# read options
	parser, (options, args) = parseOpts(sys.argv)
	if options.config is None:
		parser.print_help()
		sys.exit(1)

	rg = sigraph.FRGraph()
	fg = sigraph.FGraph()

	cfg = sigraph.AnnealConfigurator()
	cfg.loadConfig(options.config)
	cfg.loadGraphs(rg, fg)
	an = sigraph.Anneal(fg, rg)
	Modifier = weightsModifierFactory(options.modifier)
	modifier = Modifier(an.rGraph(), an.cGraph())
	cfg.initAnneal(an, cfg.plotFile)

	#sigraph.AnnealExtension(an)

	# set weights
	# define extension if needed
	if modifier.isTimeChanging():
		ae = WeightedAnnealExtension(an, Modifier)
		an.addExtension(ae, 1)
	else:
		if options.modifier in ['ratio', 'threshold']:
			modifier.setLambda(float(options._lambda))
		modifier.modify()

	# fit
	an.reset()
	while not an.isFinished():
		an.nIter()
		an.fitStep()
	print_potentials(fg)

	# write graph output with new labels
	w = sigraph.FoldWriter(cfg.output)
	w.write(fg)

if __name__ == '__main__' : main()
