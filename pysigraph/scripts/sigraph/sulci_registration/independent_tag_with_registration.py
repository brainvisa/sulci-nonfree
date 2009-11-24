#!/usr/bin/env python

import os, sys, numpy, pprint
from optparse import OptionParser
import sigraph
from soma import aims
from datamind.tools import *
from sulci.common import io, add_translation_option_to_parser
from sulci.models import distribution, distribution_aims
from sulci.models.distribution import UniformFrequency
from sulci.features.descriptors import descriptorFactory
from sulci.registration import MixtureGlobalRegistration, \
				MixtureLocalRegistration
from sulci.registration.common import save_transformation
from sulci.registration.transformation import SulcusWiseRigidTransformations
import sulci.registration.spam

class Tagger(object):
	def __init__(self, sulcimodel, gaussians_distrib, descriptor,
		graph_data, input_motion, available_labels, csvname,
		selected_sulci, node_index, no_tal, is_affine, verbose=0):
		self._sulcimodel = sulcimodel
		self._distrib = sulcimodel.segments_distrib()
		self._descriptor = descriptor
		self._states = sulcimodel.labels()
		self._states_n = len(self._states)
		self._states_map = dict((label, i) \
			for i, label in enumerate(self._states))
		self._graph_data = graph_data
		if no_tal:
			self._motion = aims.Motion()
			self._motion.setToIdentity()
		else:	self._motion = aims.GraphManip.talairach(graph_data)
		self._is_affine = is_affine
		if input_motion: self._motion = input_motion * self._motion
		self._selected_sulci = selected_sulci
		self._node_index = node_index
		self._verbose = verbose
		self._fd = open(csvname, 'w')
		s = "nodes\tlikelihood\tloglikelihood\tproba"
		for si in range(self._states_n):
			s += '\tproba_' + self._states[si]
		self._fd.write(s + '\n')

		# priors
		priors = self._get_priors()

		# data
		self._X, self._groups = self._create_data_matrix()

		# mixture model
		self._gravity_centers = []
		translation_prior, direction_prior, angle_prior = [], [], []
		models = []
		rp = sulcimodel._local_rotations_prior
		if rp[0]: t_rp = rp[0]['vertices']
		else: t_rp = None
		if rp[1]: d_rp = rp[1]['vertices']
		else: d_rp = None
		if rp[2]: a_rp = rp[2]['vertices']
		else: a_rp = None
		for i, label in enumerate(self._states):
			distrib = self._distrib['vertices'][label]
			models.append(distrib)
			if gaussians_distrib:
				gd = gaussians_distrib['vertices'][label]
				g = numpy.asarray(gd.mean()).T
				self._gravity_centers.append(g)
			if t_rp: translation_prior.append(t_rp[label])
			if d_rp: direction_prior.append(d_rp[label])
			if a_rp: angle_prior.append(a_rp[label])
		self._local_rotations_prior = translation_prior, \
					direction_prior, angle_prior
		Mixture = self._get_mixture_model()
		self._mixture = Mixture(models, priors)

		# available labels
		if available_labels:
			self._available_labels = \
			self.get_avalaible_labels_matrix(available_labels)
		else:	self._available_labels = None

	def _not_selected(self, xi):
		node_index = xi['index']
		sulcus = xi['name']
		return ((self._selected_sulci is not None) and \
			(sulcus not in self._selected_sulci)) or \
			((self._node_index is not None and
			node_index != self._node_index))

	def _get_priors(self):
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
						print "error : labels size " + \
							"differs between " + \
							"sulcimodel and prior."
						sys.exit(1)
					print "warning : labels order differs"+\
						" between sulcimodel and " + \
						"prior : order fixed."
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

	def get_avalaible_labels_matrix(self, available_labels):
		L = []
		for xi in self._graph_data.vertices():
			if xi.getSyntax() != 'fold' : continue
			if self._not_selected(xi): continue
			node_index = xi['index']
			Lxi = numpy.zeros(self._states_n)
			for label in available_labels[node_index]:
				id = self._states_map[label]
				Lxi[id] = 1
			L.append(Lxi)
		L = numpy.vstack(L)
		return L

	def tag(self, mode='global', eps=1, maxiter=numpy.inf,
			user_func=(lambda self,x : None),
			user_data=None):
		if mode == 'global' :
			opt = [self._X.T, self._mixture, self._groups,
				self._available_labels,
				self._is_affine, self._verbose]
			reg = MixtureGlobalRegistration(*opt)
		elif mode == 'local' :
			rotations_prior = self._local_rotations_prior
			opt = [self._X.T, self._gravity_centers, self._mixture,
				rotations_prior[0], rotations_prior[1],
				rotations_prior[2], self._groups,
				self._available_labels, self._verbose]
			reg = MixtureLocalRegistration(*opt)
		trans, posteriors, loglikelihoods, likelihoods = \
			reg.optimize(eps=eps, maxiter=maxiter,
			user_func=user_func, user_data=user_data,
			mode=self._opt_mode, affine=self._is_affine)
		available_labels = {}

		# set label to maximum a posteriori
		i = 0
		for xi in self._graph_data.vertices():
			if xi.getSyntax() != 'fold' : continue
			node_index = xi['index']
			if self._not_selected(xi): continue
			posteriors_xi = posteriors[:, i]
			loglikelihoods_xi = loglikelihoods[:, i]
			likelihoods_xi = likelihoods[:, i]
			si = numpy.argmax(posteriors_xi)
			p = posteriors_xi[si]
			logli = loglikelihoods_xi[si]
			li = likelihoods_xi[si]
			xi['label'] = self._states[si]
			ind = int(node_index)
			s = "%d\t%f\t%f\t%f" % (ind, li, logli, p)
			for si in range(self._states_n):
				s += '\t%f' % posteriors_xi[si]
			self._fd.write(s + '\n')
			filter = numpy.asarray(posteriors_xi > 0.01).flatten()
			k = numpy.argwhere(filter).T.flatten()
			available_labels[ind] = \
				[self._states[si] for si in k]
			i += 1

		return trans, available_labels

	def _create_data_matrix(self):
		X = []
		groups = []
		id = 0
		for xi in self._graph_data.vertices():
			if xi.getSyntax() != 'fold' : continue
			if self._not_selected(xi): continue
			Xs = self._descriptor.data(self._motion, xi)
			if Xs is not None:
				X.append(Xs)
				groups += [id] * len(Xs)
			id += 1
		return numpy.vstack(X), numpy.array(groups)


		
class GaussianTagger(Tagger):
	def __init__(self, *args, **kwargs):
		Tagger.__init__(self, *args, **kwargs)
		self._opt_mode = 'riemannian'

	def _get_mixture_model(self):
		return distribution.GaussianMixtureModel

	def vtk_tag(self, mode, eps=1., maxiter=numpy.inf):
		import sulci.registration.vtk_helpers as V
		n = 100
		means = self._mixture.get_means()
		covariances = self._mixture.get_covariances()
		Y = V.gen_data_gmm(means, covariances, n)
		X = self._X.T
		pcX = V.PointsCloud(X)
		pcX.set_color([1, 0, 0])
		pcX.set_size(3)
		pcY = V.PointsCloud(Y)
		pcY.set_color([0, 0, 1])
		pcY.set_size(3)
		weights = [[1. / self._states_n] \
			* n * self._states_n for i in range(self._states_n)]
		weights = numpy.asmatrix(numpy.vstack(weights))
		fgl = V.FuzzyGaussianLinks(X, means, weights)
		fgl.set_color([0.9, 0.9, 0.9])
		vec = V.Vector([0, 0, 0], [10, 10, 10])
		vec.set_color([0, 1, 0])
		vec.set_size(3)
		plotter = V.VtkPlot(600, 600)
		plotter.set_bgcolor([0.7, 0.8, 0.9])
		plotter.plot([fgl, pcX, pcY, vec])
		plotter.render()

		def update_vtk(procrust, (pcX, fgl, vec, plotter)):
			X = procrust._X
			t = procrust._t
			R = procrust._R
			w = procrust._w
			centers = procrust._centers
			weights = procrust._weights
			X2 = (R * X + t)
			pcX.set_X(X2)
			fgl.set(X2, centers, weights)
			vec.set((R * X.mean(axis=1) + t), \
				w * 10/ numpy.linalg.norm(w))
			plotter.render()
		
		trans, available_labels = self.tag(mode, eps, maxiter,
				update_vtk, (pcX, fgl, vec, plotter))
			
		plotter.show()
		return trans, available_labels


class SpamTagger(Tagger):
	def __init__(self, *args, **kwargs):
		Tagger.__init__(self, *args, **kwargs)
		self._opt_mode = 'powell'

	def _get_mixture_model(self):
		return distribution_aims.SpamMixtureModel


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
	parser.add_option('--input-motion', dest='input_motion',
		metavar = 'FILE', action='store', default = None,
		help='motion file (.trm) from Talairach to the ' + \
		'space of segments model')
	parser.add_option('--motion', action='store', dest='motion',
                metavar = 'FILE', default = None,
                help='output .trm motion file to register data on model')
	parser.add_option('-d', '--distrib', dest='distribname',
		metavar = 'FILE', action='store', default = None,
		help='distribution models')
	parser.add_option('--distrib-gaussians', dest='distrib_gaussians_name',
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
	parser.add_option('--input-labels', dest='input_labelsfile',
		metavar = 'FILE', action='store', default = None,
		help='file storing available labels for each sulci node')
	parser.add_option('-l', '--output-labels', dest='output_labelsfile',
		metavar = 'FILE', action='store',
		default = 'posterior_independent_labels.dat',
		help='file storing available labels for each sulci node')
	parser.add_option('--vtk', dest='vtk', action='store_true',
		default = False, help='display with vtk')
	parser.add_option('-v', '--verbose', dest='verbose',
		metavar = 'FILE', action='store', default = '0',
		help='verbosity level : 0 no verbosity, 1, 2... more verbosity')
	parser.add_option('-e', '--eps', dest='eps', metavar = 'FLOAT',
		action='store', default = 1., type='float',
		help='precision to stop EM')
	parser.add_option('--maxiter', dest='maxiter', metavar = 'INT',
		action='store', default = numpy.inf, type='int',
		help="max iterations number of optimization process")
	parser.add_option('--mode', dest='mode', metavar = 'MODE',
		action='store', default = 'global', help="registration " + \
		"mode : 'global' (one rotation+translation)  or 'local' " + \
		"(one rotation+translation by sulcus)")
	parser.add_option('--no-talairach', dest='no_tal',
		action='store_true', default = False,
		help="if not specified the internal transformation from " + \
		"subject to Talairach is used before any other given " + \
		"transformation.")
	parser.add_option('--affine', dest='is_affine',
		action='store_true', default = False,
		help="if not specified: rigid transformation." + \
		" if specified: affine transformation")
	parser.add_option('--translation-prior', dest='translation_prior',
		metavar = 'FILE', action='store', default=None,
		help="translation prior (see learn_transformation_prior.py)")
	parser.add_option('--direction-prior', dest='direction_prior',
		metavar = 'FILE', action='store', default=None,
		help="direction prior (see learn_transformation_prior.py)")
	parser.add_option('--angle-prior', dest='angle_prior',
		metavar = 'FILE', action='store', default=None,
		help="angle prior (see learn_transformation_prior.py)")

	return parser, parser.parse_args(argv)


def print_available_labels(filename, h):
	fd = open(filename, 'w')
	fd.write('labels = \\\n')
	p = pprint.PrettyPrinter(indent=4, width=1000, stream=fd)
	p.pprint(h)
	fd.close()


def main():
	parser, (options, args) = parseOpts(sys.argv)
	if None in [options.input_graphname, options.distribname]:
		print "missing option(s)"
		parser.print_help()
		sys.exit(1)

	if options.mode == 'local' and options.distrib_gaussians_name is None:
		print "error : missing gaussian distrib"
		sys.exit(1)

	if options.node_index:
		node_index = int(options.node_index)
	else:	node_index = None

	print "read..."
	if options.input_labelsfile:
		availablelabels = io.read_availablelabels(
				options.input_labelsfile)
	else:	availablelabels = None
	if options.sulci is None:
		selected_sulci = None
	else:	selected_sulci = options.sulci.split(',')
	sulcimodel = io.read_full_model(None,
		segmentsdistribname=options.distribname,
		labelspriorname=options.priorname,
		selected_sulci=selected_sulci)
	graph = io.load_graph(options.transfile, options.input_graphname)

	if options.distrib_gaussians_name:
		gaussians_distrib = io.read_segments_distrib(\
			options.distrib_gaussians_name, selected_sulci)
	else:	gaussians_distrib = None

	if options.translation_prior:
		translation_prior = io.read_segments_distrib(\
			options.translation_prior, selected_sulci)
	else:	translation_prior = None
	if options.direction_prior:
		direction_prior = io.read_segments_distrib(\
			options.direction_prior, selected_sulci)
	else:	direction_prior = None
	if options.angle_prior:
		angle_prior = io.read_segments_distrib(\
			options.angle_prior, selected_sulci)
	else:	angle_prior = None
	# hack...
	sulcimodel._local_rotations_prior = (translation_prior, \
				direction_prior, angle_prior)

	# format options
	data_type = sulcimodel.segments_distrib()['data_type']
	descriptor = descriptorFactory(data_type)
	distribs = sulcimodel.segments_distrib()['vertices']
	s = set()
	for sulcus, distrib in distribs.items():
		if isinstance(distrib, distribution.Gaussian):
			s.add('gaussian')
		elif isinstance(distrib, distribution_aims.Spam):
			s.add('spam')
	if len(s) > 1:
		print "error: mix of different models is not handle"
		print "%s found" % str(list(s))
		sys.exit(1)
	model_type = list(s)[0]
	if not (model_type in ['gaussian', 'spam']):
		print "error : unhandle model type '%s'" % model_type
		sys.exit(1)

	if options.input_motion:
		input_motion = aims.Reader().read(options.input_motion)
	else:	input_motion = None
	tagger_opt = [sulcimodel, gaussians_distrib, descriptor, graph,
		input_motion, availablelabels, options.csvname, selected_sulci, 
		node_index, options.no_tal, options.is_affine,
		int(options.verbose)]
	if model_type == 'gaussian' : d = GaussianTagger(*tagger_opt)
	elif model_type == 'spam' : d = SpamTagger(*tagger_opt)
	print "tag..."
	mode = options.mode

	# tag/registration + write outputs
	if options.vtk:
		trans, available_labels = d.vtk_tag(mode, options.eps,
							options.maxiter)
	else:	trans, available_labels = d.tag(mode, options.eps,
							options.maxiter)
	if options.motion:
		if mode == 'global': trans.write(options.motion)
		elif mode == 'local':
			transformations = trans.get_transformations()
			trans = SulcusWiseRigidTransformations()
			for i, sulcus in enumerate(sulcimodel.labels()):
				t = transformations[i]
				trans.add_transformation(sulcus, t)
			dir = options.motion
			filename = options.motion + '.dat'
			print dir, filename
			try:
				trans.write(dir, filename)
			except OSError, e:
				print e
				sys.exit(1)

	print_available_labels(options.output_labelsfile, available_labels)
	graph['filename_base'] = '*'
	w = sigraph.FoldWriter(options.output_graphname)
	w.write(graph)

if __name__ == '__main__' : main()
