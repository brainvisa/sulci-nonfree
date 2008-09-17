#!/usr/bin/env python
import os, sys, numpy, pprint, re, glob
from optparse import OptionParser
import matplotlib
matplotlib.use('QtAgg')
import pylab
import sigraph
from soma import aims
from sulci.common import io, add_translation_option_to_parser
from sulci.models import distribution, distribution_aims
from sulci.features.descriptors import descriptorFactory
from soma.gui.api import chooseMatplotlibBackend
chooseMatplotlibBackend()

def save_fig(relation, distr, X, filename):
	n = len(X) / 5
	if n < 5: n = 5
	if n > 100: n = 100
	xmin = X.min()
	xmax = X.max()
	if xmin == xmax: return
	f = pylab.gcf()
	if (X != 0).sum() != 0:
		pylab.hist(X, bins=n, normed=1)
	pylab.title("relation : " + str(relation))
	x = numpy.linspace(xmin, xmax, 1000)
	y = numpy.asarray(distr.likelihoods(x)[1]).ravel()
	pylab.plot(x, y, 'r-')
	pylab.savefig(filename, dpi=150)
	f.clear()

from sulci.features.descriptors import RelationDescriptor

class ConnexionStrengthRelationDescriptor(RelationDescriptor):
	def __init__(self):
		RelationDescriptor.__init__(self)
		self._name = 'connexion_strength'

	def data_edge(self, motion, edge):
		v1, v2 = edge.vertices()
		if edge.getSyntax() == 'junction':
			return edge['reflength']
		else:	return None

	def edges_from_graph(self, graph, selected_sulci=None):
		edges = {}
		# store in priority junction/plidepassage
		# and else cortical
		for e in graph.edges():
			v1, v2 = e.vertices()
			# skip hull junctions
			if v1.getSyntax() != 'fold' or \
				v2.getSyntax() != 'fold':
				continue
			name1, name2 = v1['name'], v2['name']
			if selected_sulci != None and \
				(name1 not in selected_sulci) and \
				(name2 not in selected_sulci):
				continue
			r1, r2 = v1['index'], v2['index']
			if r1 > r2:
				v1, v2 = v2, v1
				r1, r2 = r2, r1
			if e.getSyntax() == 'junction':
				edges[(r1, r2)] = e, v1, v2
		return edges

	def data_from_graphs(self, graphs, selected_sulci=None):
		data = {}
		for g in graphs:
			motion = aims.GraphManip.talairach(g)
			edges = self.edges_from_graph(g, selected_sulci)
			for (r1, r2), (e, v1, v2) in edges.items():
				name1, name2 = v1['name'], v2['name']
				d = numpy.array(self.data_edge(motion, e))
				if d is None: continue
				# order names
				if name1 > name2: name1, name2 = name2, name1
				key = (name1, name2)
				if data.has_key(key):
					data[key].append(d)
				else:	data[key] = [d]
		for relation, D in data.items():
			data[relation] = numpy.vstack(D)
		return data

################################################################################
def compute_relations(graphs, distribdir, selected_sulci, options):
	min_db_size = 100
	sulci_set = {}
	if options.data_type == 'connexion_strength': #FIXME
		descriptor = ConnexionStrengthRelationDescriptor()
	else: descriptor = descriptorFactory(options.data_type)
	data = descriptor.data_from_graphs(graphs, selected_sulci)

	# create output directory
	prefix = distribdir
	try:	os.mkdir(prefix)
	except OSError, e:
		print e
		sys.exit(1)

	Distrib = distribution.distributionFactory(options.model_type)

	# learn and write spams
	h = {'level' : 'relations', 'data_type' : options.data_type,
		'files' : {}}
	if options.savefig:
		s = [len(X) for relation, X in data.items()]
		print "number of relations= ", len(data)
		if len(data) and (numpy.min(s) != numpy.max(s)):
			f = pylab.gcf()
			pylab.hist(s, bins=150)
			filename = os.path.join(prefix, 'size_hist.png')
			pylab.savefig(filename, dpi=150)
			f.clear()
	Xd_intra = []
	Xd_inter = []
	for relation, X in data.items():
		sulcus1, sulcus2 = relation
		if len(X) < min_db_size:
			if sulcus1 == sulcus2:
				Xd_intra.append(X)
			else:	Xd_inter.append(X)
	data['default_intra'] = numpy.vstack(Xd_intra)
	data['default_inter'] = numpy.vstack(Xd_inter)

	for relation, X in data.items():
		if isinstance(relation, list):
			sulcus1, sulcus2 = relation
			if selected_sulci != None and \
				(sulcus1 not in selected_sulci) \
				and (sulcus2 not in selected_sulci): continue
		if len(X) < min_db_size: continue
		print "*** %s ***" % str(relation)
		print repr(X.T)
		d = Distrib()
		d.fit(X)
		filename = io.node2densityname(prefix,
				options.model_type, relation)
		if options.savefig: save_fig(relation, d, X, filename + '.png')
		d.write(filename)
		relfilename = re.sub('^%s%s' % (os.path.dirname(prefix), \
						os.path.sep), '', filename)
		h['files'][relation] = (options.model_type, relfilename)

	# write distribution summary file
	summary_file = distribdir + '.dat'
	io.write_pp('distributions', summary_file, h)


################################################################################
def parseOpts(argv):
	description = 'Compute relations models from a list of graphs.\n' \
	'learn_relations.py [OPTIONS] graph1.arg graph2.arg...\n'
	parser = OptionParser(description)
	add_translation_option_to_parser(parser)
	parser.add_option('-d', '--distribdir', dest='distribdir',
		metavar='FILE', action='store',
		default='bayesian_relation_distribs',
		help='output distribution directory (default : %default).' \
			'A file named FILE.dat is created to store ' \
			'labels/databases links.')
	parser.add_option('-s', '--sulci', dest='sulci',
		metavar='LIST', action='store', default=None,
		help='compute models only for specified sulci. ex: ' + \
		'"S.C._right,S.C._right;S.Pe.C._right,S.F.inf._right;' + \
		'S.F.sup._right,*" : it specifies 3 relations, 1st : intra ' + \
		'S.C._right, 2nd : between S.Pe.C._right and S.F.inf._right '+ \
		'and 3rd : between S.F.sup._right and any sulcus')
	parser.add_option('--data-type', dest='data_type',
		metavar='TYPE', action='store', default='min_distance',
		type='choice', choices=('min_distance', 'connexion_strength'),
		help="data type : kind of relation to be learned. " + \
		"'min_distance' : distance between the 2 " + \
		"nearest extremities (near from the minimal distance), " + \
		"'connection_strength' : 0 if disconnected segments, from " + \
		"0 to 1 for connected components (default : %default)")
	parser.add_option('--model-type', dest='model_type',
		metavar='TYPE', action='store', default='gamma', type='choice',
		choices=('gamma', 'beta', 'gamma_exponential_mixture'),
		help="distributions : gamma, beta, gamma_exponential_mixture")
	parser.add_option('--mode', dest='mode', metavar = 'FILE',
		action='store', default='normal', type='choice',
		choices=('normal', 'loo'),
		help="'normal' : compute spams on given graphs, 'loo' : " + \
		"leave one out on graphs : create several models " + \
		"(default : %default), all given reference FILE options " + \
		"must be located in './all/' relative directory and similar " +\
		"data can be found in './cv_*/' directories relative to " + \
		"leave one out graphs folds.")
	parser.add_option('--save-fig', dest='savefig',
		action='store_true', default=False,
		help="save histograms of learned values")

	return parser, parser.parse_args(argv)


def main():
	# options
	parser, (options, args) = parseOpts(sys.argv)
	graphnames = args[1:]
	if len(graphnames) == 0:
		print "error : at least one graph is needed"
		parser.print_help()
		sys.exit(1)

	# read data
	graphs = io.load_graphs(options.transfile, graphnames)
	if options.sulci is None:
		selected_sulci = None
	else:	selected_sulci = options.sulci.split(',')

	# computations
	if options.mode == 'normal' :
		compute_relations(graphs, options.distribdir,
					selected_sulci, options)
	elif options.mode == 'loo' :
		print "-- all --"
		distribdir = os.path.join('all', options.distribdir)
		if options.sigma_file is None:
			sigma_file = None
		else:	sigma_file = os.path.join('all', options.sigma_file)
		compute_relations(graphs, distribdir, selected_sulci, options)
		for i in range(len(graphs)):
			subgraphs = graphs[:i] + graphs[i+1:]
			directory = 'cv_%d' % i
			print '-- %s --' % directory
			distribdir = os.path.join(directory, options.distribdir)
			if options.sigma_file is None:
				sigma_file = None
			else:	sigma_file = os.path.join(directory,
					options.sigma_file)
			compute_relations(subgraphs, distribdir,
					selected_sulci, options)
	else:
		print "error : '%s' unknown mode" % options.mode



if __name__ == '__main__' : main()
