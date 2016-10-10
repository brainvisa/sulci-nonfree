#!/usr/bin/env python2

from __future__ import print_function
import os, sys, numpy, pprint, re
from optparse import OptionParser
import sigraph
from soma import aims
from datamind.tools import *
from sulci.common import io, add_translation_option_to_parser
from sulci.models import check_same_distribution
from sulci.models.distribution import UniformFrequency
from sulci.features.descriptors import descriptorFactory

################################################################################
class Tagger(object):
	def __init__(self, sulcimodel, descriptor, graph_data,
			csvname, selected_sulci=None, node_index=None):
		self._sulcimodel = sulcimodel
		self._distrib = sulcimodel.segments_distrib()
		self._descriptor = descriptor
		self._states = sulcimodel.labels()
		self._states_n = len(self._states)
		self._graph_data = graph_data
		self._motion = aims.GraphManip.talairach(graph_data)
		self._selected_sulci = selected_sulci
		self._node_index = node_index
		self._fd = open(csvname, 'w')
		self._init()

	def _init(self):
		# output header
		s = "nodes\tlikelihood\tloglikelihood\tproba"
		for si in range(self._states_n):
			s += '\tproba_' + self._states[si]
		self._fd.write(s + '\n')

	def _not_selected(self, xi):
		node_index = xi['index']
		sulcus = xi['name']
		return ((self._selected_sulci is not None) and \
			(sulcus not in self._selected_sulci)) or \
			((self._node_index is not None and
			node_index != self._node_index))

	def _priors(self):
		labels_prior_distrib = self._sulcimodel.labels_prior()
		if labels_prior_distrib:
			priors = labels_prior_distrib['prior'].frequencies()
			prior_labels = labels_prior_distrib['labels']
			if self._selected_sulci is None:
				p = numpy.array(prior_labels)
				s = numpy.array(self._states)
				if self._states != prior_labels:
					if len(self._states) != \
						len(prior_labels):
						print("error : labels size " + \
							"differs between " + \
							"sulcimodel and prior.")
						sys.exit(1)
					print("warning : labels order differs"+\
						" between sulcimodel and " + \
						"prior : order fixed.")
					indices = [numpy.argwhere(p == x)[0,0] \
								for x in s]
					priors = numpy.asarray(priors)[0][indices]
			else:
				indices = []
				for sulcus in self._states:
					ind = numpy.argwhere(numpy.array(\
						[x == sulcus \
						for x in prior_labels]))[0,0]
					indices.append(ind)
				priors = numpy.asarray(priors)[0][indices]
		else:	priors = UniformFrequency(self._states_n).frequencies()
		return numpy.ravel(numpy.asarray(priors))

	def tag(self):
		i = 1
		size = len(self._graph_data.vertices())
		priors = self._priors()
		available_labels = {}
		bar = ProgressionBarPct(len(self._graph_data.vertices()),
						'#', color = 'green')
		for xi in self._graph_data.vertices():
			bar.display(i)
			if xi.getSyntax() != 'fold' : continue
			if self._not_selected(xi):
				i += 1
				continue
			logli, li = self._likelihoods(xi)
			liw = li * priors
			p = liw / liw.sum()
			si = numpy.argmax(p)
			xi['label'] = self._states[si]
			ind = int(xi['index'])
			s = "%d\t%f\t%f\t%f" % (ind, li[si], logli[si], p[si])
			for si in range(self._states_n): s += '\t%f' % p[si]
			self._fd.write(s + '\n')
			k = numpy.argwhere(p > 0.01).T.flatten()
			available_labels[ind] = [self._states[si] for si in k]
			i += 1
		return available_labels

	def _likelihoods(self, xi):
		li = numpy.zeros(self._states_n, dtype=numpy.longdouble)
		logli = numpy.zeros(self._states_n, dtype=numpy.longdouble)
		distribs = self._distrib['vertices']
		for i, label in enumerate(self._states):
			distrib = distribs[label]
			logli[i], li[i] = self._descriptor.likelihood(\
						distrib, self._motion, xi)
		return logli, li


################################################################################
def parseOpts(argv):
	description = 'Tag sulci graph based on a bayesian model.'
	parser = OptionParser(description)
	add_translation_option_to_parser(parser)
	parser.add_option('-i', '--ingraph', dest='input_graphname',
		metavar = 'FILE', action='store', default = None,
		help='data graph')
	parser.add_option('-o', '--outgraph', dest='output_graphname',
		metavar = 'FILE', action='store',
		default = 'posterior_independent.arg',
		help='output tagged graph (default : %default)')
	parser.add_option('-d', '--distrib', dest='distribname',
		metavar = 'FILE', action='store', default = None,
		help='distribution models')
	parser.add_option('-c', '--csv', dest='csvname',
		metavar = 'FILE', action='store',
		default = 'posterior_independent.csv',
		help='csv storing posterior probabilities (default : %default)')
	parser.add_option('-s', '--sulci', dest='sulci',
		metavar = 'LIST', action='store', default = None,
		help='tag only specified manually tagged sulci.')
	parser.add_option('-n', '--node', dest='node_index',
		metavar = 'INDEX', action='store', default = None,
		help='tag only one specified node')
	parser.add_option('-p', '--prior', dest='priorname',
		metavar = 'FILE', action='store', default = None,
		help='prior file (default : no prior)')
	parser.add_option('-l', '--labels', dest='labelsfile',
		metavar = 'FILE', action='store',
		default = 'posterior_independent_labels.dat',
		help='file storing available labels for each sulci node')
	return parser, parser.parse_args(argv)


def main():
	parser, (options, args) = parseOpts(sys.argv)
	if None in [options.input_graphname, options.distribname]:
		parser.print_help()
		sys.exit(1)

	if options.node_index:
		node_index = int(options.node_index)
	else:	node_index = None

	print("read...")
	if options.sulci is None:
		selected_sulci = None
	else:	selected_sulci = options.sulci.split(',')
	sulcimodel = io.read_full_model(None,
		segmentsdistribname=options.distribname,
		labelspriorname=options.priorname,
		selected_sulci=selected_sulci)
	graph = io.load_graph(options.transfile, options.input_graphname)

	# format options
	segments_distrib = sulcimodel.segments_distrib()
	data_type = segments_distrib['data_type']
	check, models_types = check_same_distribution(\
				segments_distrib['vertices'])
	if not check:
		print("error : only one model_type is supported. Found : " +\
			str(list(models_types)))
		sys.exit(1)
	models_type = list(models_types)[0]
	print("models type = ", models_type)
	descriptor = descriptorFactory(data_type)

	# descriptor depth map
	if models_type == 'depth_weighted_spam':
		reader = aims.Reader()
		filename = graph['aims_reader_filename']
		subject = os.path.splitext(os.path.basename(filename))[0]
		side = subject[0]
		subject = re.sub('_.*', '', subject)[1:]
		depthmapname = os.path.join('..', '%s_depthmap.ima' % subject)
		if not os.path.exists(depthmapname):
			print("error : can't find depthmap file '%s'" % \
								depthmapname)
			sys.exit(1)
		print("find depthmap for subject '%s'" % subject)
		depthmap = reader.read(depthmapname)
		depthmap = aims.AimsData_FLOAT(depthmap)
		descriptor.setDepthMap(depthmap)

	# sulci tag
	tagger = Tagger(sulcimodel, descriptor, graph,
			options.csvname, selected_sulci, node_index)
	print("tag...")
	available_labels = tagger.tag()
	io.write_pp('labels', options.labelsfile, available_labels)

	graph['filename_base'] = '*'
	w = sigraph.FoldWriter(options.output_graphname)
	w.write(graph)

if __name__ == '__main__' : main()
