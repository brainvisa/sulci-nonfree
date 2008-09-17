#!/usr/bin/env python

import os, sys, numpy, pprint, re
from optparse import OptionParser
import sigraph
from soma import aims
from datamind.tools import *
from sulci.common import io, add_translation_option_to_parser
from sulci.models import distribution


################################################################################
class Prior(object):
	def __init__(self, graphmodel, graphs, distr,
			compute_likelihoods=False):
		self._graphmodel = graphmodel
		self._sulci = graphmodel['vertices'].keys()
		self._sulci_map = dict((s, i) \
			for (i, s) in enumerate(self._sulci))
		self._graphs = graphs
		self._distr = distr
		self._compute_likelihoods = compute_likelihoods

	def _extract_local_data(self, vertex):
		print "undefined virtual method"
		sys.exit(1)

	def _post_process_extracted_graph_data(self, h):
		print "undefined virtual method"
		sys.exit(1)

	def _extract_data(self):
		X = []
		for g in self._graphs:
			x = numpy.zeros(len(self._sulci))
			n = 0
			for v in g.vertices():
				if v.getSyntax() != 'fold' : continue
				name = v['name']
				val = self._extract_local_data(v)
				x[self._sulci_map[name]] += val
				n += val
			self._post_process_extracted_graph_data(x, n)
			X.append(x)
		X = numpy.vstack(X)
		return X

	def _compute_likelihoods_func(self, X):
		for x in X:
			logli, li = self._distr.likelihood(x)
			print "proba density (log) = %f (%f) " % (li, logli)

	def eval(self):
		X = self._extract_data()
		self._distr.fit(X)
		if self._compute_likelihoods:
			self._compute_likelihoods_func(X)

	def sulci(self):
		return self._sulci

	def modeltype(self):
		return self._distr.name()

class DirichletPrior(Prior):
	def __init__(self, *args, **kwargs):
		Prior.__init__(self, *args, **kwargs)

	def _post_process_extracted_graph_data(self, x, n):
		pass


class FrequencyPrior(Prior):
	def __init__(self, *args, **kwargs):
		Prior.__init__(self, *args, **kwargs)

	def _post_process_extracted_graph_data(self, x, n):
		pass


################################################################################
class LabelFrequencyPrior(FrequencyPrior):
	def __init__(self, *args, **kwargs):
		FrequencyPrior.__init__(self, *args, **kwargs)

	def _extract_local_data(self, vertex):
		return 1.
	

class LabelDirichletPrior(DirichletPrior):
	def __init__(self, *args, **kwargs):
		DirichletPrior.__init__(self, *args, **kwargs)

	def _extract_local_data(self, vertex):
		return 1.

################################################################################
class SizeFrequencyPrior(FrequencyPrior):
	def __init__(self, *args, **kwargs):
		FrequencyPrior.__init__(self, *args, **kwargs)

	def _extract_local_data(self, vertex):
		return float(vertex['refsize'])

class SizeDirichletPrior(DirichletPrior):
	def __init__(self, *args, **kwargs):
		DirichletPrior.__init__(self, *args, **kwargs)

	def _extract_local_data(self, vertex):
		return float(vertex['refsize'])


################################################################################
prior_map = { \
	'label_frequency' : (LabelFrequencyPrior, distribution.Frequency, []),
	'label_dirichlet' : (LabelDirichletPrior,
		distribution.ShiftedDirichlet, [1.]),
	'label_generalized_dirichlet' : (LabelDirichletPrior,
		distribution.ShiftedGeneralizedDirichlet, [1.]),
	'size_frequency' : (SizeFrequencyPrior, distribution.Frequency, []),
	'size_dirichlet' : (SizeDirichletPrior,
		distribution.ShiftedDirichlet, [1.]),
	'size_generalized_dirichlet' : (SizeDirichletPrior,
		distribution.ShiftedGeneralizedDirichlet, [1.]),
}

def priorFactory(type):
	return prior_map[type]


################################################################################
def compute_prior(graphmodel, graphs, MyPrior, distr, options, output):
	# create output directory
	prefix = output
	try:	os.mkdir(prefix)
	except OSError, e:
		print e
		sys.exit(1)

	prior = MyPrior(graphmodel, graphs, distr,
			options.compute_likelihoods)
	prior.eval()

	# write distribution summary file
	prior_file = output + '.dat'
	model_file = os.path.join(prefix, 'global_prior.dat')
	distr.write(model_file)

	relmodel_file = re.sub('^%s%s' % (os.path.dirname(prefix),
					os.path.sep), '', model_file)
	h = {
		'level' : 'labels',
		'data_type' : options.prior_type,
		'labels' : list(prior.sulci()),
		'model_file' : (prior.modeltype(), relmodel_file),
	}
	io.write_pp('distributions', prior_file, h)


################################################################################
# main + options

def parseOpts(argv):
	description = 'compute label priors\n' \
		'./learn_priors.py [OPTIONS] graph1.arg graph2.arg...'
	parser = OptionParser(description)
	add_translation_option_to_parser(parser)
	parser.add_option('-m', '--graphmodel', dest='graphmodelname',
		metavar = 'FILE', action='store',
		default = 'bayesian_graphmodel.dat', help='bayesian model : ' \
			'graphical model structure (default : %default)')
	parser.add_option('--type', dest='prior_type',
		metavar = 'FILE', action='store', default = 'label_frequency',
		help='type : one among %s ,' % str(prior_map.keys()) + \
		'- label : at node level\n- size : at resized voxel level,\n' +\
		'- frequency : P(L) = prod P(L_i) = prod(f_i) with ' +\
		'f_i = #(L_i) / (sum #(L_i))\n' + \
		'- dirichlet : P(L) ~ Dirichlet(alpha)\n' + \
		'- generalized dirichlet : P(L) ~ ' + \
		'GeneralizedDirchlet(alpha, beta)')
	parser.add_option('-o', '--output', dest='output',
		metavar = 'FILE', action='store', default = 'bayesian_prior',
		help='output learned prior filename')
	parser.add_option('-l', '--compute-likelihoods',
		dest='compute_likelihoods',action='store_true', default = False,
		help='print likelihoods of input graphs with computed prior')
	parser.add_option('--mode', dest='mode',
		metavar = 'FILE', action='store', default = 'normal',
		help="'normal' : compute spams on given graphs, 'loo' : " + \
		"leave one out on graphs : create several models " + \
		"(default : %default), all given reference FILE options " + \
		"must be located in './all/' relative directory and similar " +\
		"data can be found in './cv_*/' directories relative to " + \
		"leave one out graphs folds.")

	return parser, parser.parse_args(argv)


def main():
	# option parsing
	parser, (options, args) = parseOpts(sys.argv)
	error_parsing = False

	graphnames = args[1:]
	if len(graphnames) == 0:
		print "give at least one graph"
		error_parsing = True
	if not (options.prior_type in prior_map.keys()):
		print "unknown prior type '%s'" % options.prior_type
	if error_parsing:
		parser.print_help()
		sys.exit(1)

	# reading...
	graphs = io.load_graphs(options.transfile, graphnames)
	graphmodel = io.read_graphmodel(options.graphmodelname)
	MyPrior, Distr, distr_opt = priorFactory(options.prior_type)
	distr = Distr(*distr_opt)


	if options.mode == 'normal' :
		compute_prior(graphmodel, graphs, MyPrior, distr,
				options, options.output)
	elif options.mode == 'loo' :
		print "-- all --"
		distribdir = os.path.join('all', options.output)
		compute_prior(graphmodel, graphs, MyPrior, distr,
				options, distribdir)
		for i in range(len(graphs)):
			subgraphs = graphs[:i] + graphs[i+1:]
			dir = 'cv_%d' % i
			print '-- %s --' % dir
			distribdir = os.path.join(dir, options.output)
			compute_prior(graphmodel, subgraphs, MyPrior, distr,
				options, distribdir)
	else:
		print "error : '%s' unknown mode" % options.mode



if __name__ == '__main__' : main()
