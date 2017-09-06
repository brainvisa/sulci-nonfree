#!/usr/bin/env python2

from __future__ import print_function
import os, sys, numpy, pylab
from optparse import OptionParser
import sigraph
from soma import aims
from datamind.tools import *
from sulci.common import io, add_translation_option_to_parser
from sulci.models import distribution

eps = 10e-200

################################################################################
# Vertex models
class VertexModel(object):
	def __init__(self, graph_data, bayesian_model, priors):
		self._graph_data = graph_data
		self._bayesian_model = bayesian_model
		self._states = bayesian_model['graph_model']['vertices'].keys()
		self._states_n = len(self._states)
		if priors:
			pn = bayesian_model['priors_nodes_hash']
			self._prior_ind = pn[priors]
		else:	self._prior_ind = -1
		print("compute node priors...")
		self._priors = self._compute_priors()

	def _compute_priors(self):
		priors = numpy.asmatrix(numpy.zeros(self._states_n,
						numpy.longdouble)).T
		if self._prior_ind == -1: # no prior
			return priors + 1.
		gv = self._bayesian_model['vertices']
		for si in range(len(self._states)):
			label = self._states[si]
			data = gv[label]
			priors[si] = data['priors'][self._prior_ind]
		return priors

	def get_priors(self):
		return self._priors

	def _get_vertex_distrib(self, label):
		gv = self._bayesian_model['vertices']
		return gv[label]['density']

	def _transmat(self, xi):
		print("error : abstract method")

	def states(self): return self._states

	def transmat(self, xi):
		tr = self._transmat(xi)
		t = numpy.multiply(tr, self._priors)
		return t


class GravityCenterVertexModel(VertexModel):
	def __init__(self, graph_data, bayesian_model, priors):
		VertexModel.__init__(self, graph_data, bayesian_model, priors)
	
	def _transmat(self, xi):
		n = self._states_n
		phi_i = numpy.asmatrix(numpy.zeros(n, numpy.longdouble)).T
		# gravity center :
		g = numpy.asarray(xi['refgravity_center'].list())
		for si in range(n):
			label = self._states[si]
			distrib = self._get_vertex_distrib(label)
			logli, li = distrib.likelihood(g)
			phi_i[si] = li
		return phi_i


class SpamVertexModel(VertexModel):
	def __init__(self, graph_data, bayesian_model, priors):
		VertexModel.__init__(self, graph_data, bayesian_model, priors)
	
	def _transmat(self, xi):
		n = self._states_n
		phi_i = numpy.asmatrix(numpy.zeros(n, dtype=numpy.longdouble)).T
		# gravity center :
		g = numpy.asarray(xi['refgravity_center'].list())

		for si in range(n):
			label = self._states[si]
			distrib = self._get_vertex_distrib(label)
			logli, li = distrib.prodlikelihoods_vertex(\
						self._graph_data, xi)
			phi_i[si] = li
		return phi_i


class UniformVertexModel(VertexModel):
	def __init__(self, graph_data, bayesian_model, priors):
		VertexModel.__init__(self, graph_data, bayesian_model, priors)

	def _transmat(self, xi):
		phi_i = numpy.asmatrix(numpy.ones(self._states_n, 'double')).T
		return phi_i


################################################################################
# Edge models

class EdgeModel(object):
	def __init__(self, bayesian_model, node_model, priors, lambda_):
		self._bayesian_model = bayesian_model
		self._node_model = node_model
		self._lambda = lambda_
		self._states = node_model.states()
		self._states_n = len(self._states)
		if priors:
			pn = bayesian_model['priors_relations_hash']
			self._prior_ind = pn[priors]
		else:	self._prior_ind = -1
		print("compute relation priors...")
		self._priors = self._compute_priors()

	def get_priors(self):
		return self._priors

	def _compute_priors(self):
		n = self._states_n
		priors = numpy.asmatrix(numpy.zeros((n, n), numpy.longdouble))
		if self._prior_ind == -1: # no prior
			return priors + 1.
		ge = self._bayesian_model['edges']
		gv = self._bayesian_model['vertices']
		for si in range(n):
			label_i = self._states[si]
			for sj in range(n):
				label_j = self._states[si]
				if label_i > label_j:
					labels = label_j, label_i
				else:	labels = label_i, label_j
				try:
					ps = ge[labels]['priors']
					p = ps[self._prior_ind]
				except KeyError:
					p = 10.e-100
				priors[si, sj] = p
		ind = [i for i, s in enumerate(self._states) \
					if s == 'unknown'][0]
		#priors = numpy.asmatrix(numpy.asarray(priors) ** 0.1)
		#priors = numpy.asmatrix(numpy.identity(n))
		#d = numpy.diag(priors)
		#priors /= d
		#priors = numpy.asmatrix(numpy.diag(numpy.diag(priors)))
		#pd = numpy.diag(priors).mean()
		#p0 = priors.mean(axis=0)
		#p1 = priors.mean(axis=1)
		#priors[ind, :] = priors[:, ind] = priors.min()
		#priors[ind, :] = p0
		#priors[:, ind] = p1
		#priors[ind,ind] = pd
		#priors[ind, ind] = p
		#priors += numpy.identity(n) * pd
		#priors
		#S = priors.sum(axis=0)
		#priors /= S
		#priors = numpy.multiply(priors, self._node_model.get_priors())
		#priors = numpy.asarray(priors).astype('float')
		#pylab.imshow(priors)
		#pylab.colorbar()
		#pylab.show()
		#sys.exit(1)
		return priors

	def _find_xij(self, xi, xj):
		ri = xi['index']
		rj = xj['index']
		edges = {}
		for e in xi.edges():
			v1, v2 = e.vertices()
			r1, r2 = v1['index'], v2['index']
			if (ri == r1 and rj == r2) or (ri == r2 and rj == r1):
				edges[e.getSyntax()] = e
		return edges

	def _get_edge_model(self, labels):
		'''label order is not considered'''
		if labels[0] > labels[1]: labels = labels[1], labels[0]
		ge = self._bayesian_model['edges']
		try:
			distrib = ge[labels]['density']
		except KeyError:
			if labels == ('S.C._right', 'S.C._right'):
				print(ge[labels]['density'])
			distrib = distribution.FakeGaussian(eps)
			ge[labels] = {'density': distrib }
		return distrib

	def _transmat(self, xi, xj):
		print("error : abstract method")

	def transmat(self, xi, xj):
		t = numpy.multiply(self._transmat(xi, xj), self._priors)
		t /= t.sum()
		n = self._states_n
		m = 1. / (n ** 2)
		t = (1 - self._lambda) * m + self._lambda * t
		return t

class GravityCentersDeltaEdgeModel(EdgeModel):
	def __init__(self, bayesian_model, node_model, priors, lambda_):
		EdgeModel.__init__(self, bayesian_model, node_model,
							priors, lambda_)

	def _transmat(self, xi, xj):
		g1 = numpy.asarray(xi['refgravity_center'].list())
		g2 = numpy.asarray(xj['refgravity_center'].list())
		dg = g1 - g2
		n = self._states_n
		psi_ij = numpy.zeros((n, n), numpy.longdouble)
		psi_ij = numpy.asmatrix(psi_ij)
		for si in range(n):
			label_i = self._states[si]
			for sj in range(n):
				label_j = self._states[sj]
				labels = (label_i, label_j)
				# orientation
				if label_i > label_j: s = -1
				else:	s = 1
				distrib = self._get_edge_model(labels)
				logli, li = distrib.likelihood(s * dg)
				psi_ij[si, sj] = li
		return psi_ij


class GravityCentersDistanceEdgeModel(EdgeModel):
	def __init__(self, bayesian_model, node_model, priors, lambda_):
		EdgeModel.__init__(self, bayesian_model, node_model,
							priors, lambda_)

	def _transmat(self, xi, xj):
		#FIXME : unfinished
		print(type(xi['refgravity_center']))
		print(dir(xi['refgravity_center']))
		g1 = numpy.asarray(xi['refgravity_center'].list())
		g2 = numpy.asarray(xj['refgravity_center'].list())
		dist = numpy.sqrt(((g1 - g2) ** 2).sum())
		n = self._states_n
		psi_ij = numpy.zeros((n, n), numpy.longdouble)
		psi_ij = numpy.asmatrix(psi_ij)

		for si in range(n):
			label_i = self._states[si]
			for sj in range(n):
				label_j = self._states[sj]
				labels = (label_i, label_j)
				distrib = self._get_edge_model(labels)
				logli, li = distrib.likelihood(dist)
				psi_ij[si, sj] = li
		return psi_ij


class MinDistanceEdgeModel(EdgeModel):
	def __init__(self, bayesian_model, node_model, priors, lambda_):
		EdgeModel.__init__(self, bayesian_model, node_model,
							priors, lambda_)

	def _transmat(self, xi, xj):
		edges = self._find_xij(xi, xj)
		if 'cortical' in edges.keys():
			xij = edges['cortical']
			pi = xij['refSS1nearest'].arraydata()
			pj = xij['refSS2nearest'].arraydata()
		else:
			pi = numpy.array([0, 0, 0])
			pj = numpy.array([0, 0, 0])
		dist = numpy.sqrt(((pi - pj) ** 2).sum())

		n = self._states_n
		psi_ij = numpy.zeros((n, n), numpy.longdouble)
		psi_ij = numpy.asmatrix(psi_ij)
		for si in range(n):
			label_i = self._states[si]
			for sj in range(n):
				label_j = self._states[sj]
				labels = (label_i, label_j)
				distrib = self._get_edge_model(labels)
				logli, li = distrib.likelihood(dist)
				psi_ij[si, sj] = li
		return psi_ij


class UniformEdgeModel(EdgeModel):
	def __init__(self, bayesian_model, node_model, priors, lambda_):
		EdgeModel.__init__(self, bayesian_model, node_model,
							priors, lambda_)

	def _transmat(self, xi, xj):
		n = self._states_n
		psi_ij = numpy.asmatrix(numpy.ones((n, n), 'double'))
		return psi_ij


class StickEdgeModel(EdgeModel):
	def __init__(self, bayesian_model, node_model, priors,
						lambda_, eps=0.1):
		EdgeModel.__init__(self, bayesian_model, node_model,
							priors, lambda_)
		self._eps = eps

	def _transmat(self, xi, xj):
		n = self._states_n
		psi_ij = numpy.asmatrix(numpy.identity(n, 'double') + self._eps)
		return psi_ij

class StickDistanceEdgeModel(EdgeModel):
	'''
    The nearer the 2 segments of the relation are, the more similar they are.
    They labels have better chance to be the sames.
	'''
	def __init__(self, bayesian_model, node_model, priors,
						lambda_, eps=1):
		EdgeModel.__init__(self, bayesian_model, node_model,
						priors, lambda_)
		self._eps = eps

	def _transmat(self, xi, xj):
		edges = self._find_xij(xi, xj)
		if 'cortical' in edges.keys():
			xij = edges['cortical']
			pi = xij['refSS1nearest'].arraydata()
			pj = xij['refSS2nearest'].arraydata()
		else:
			pi = numpy.array([0, 0, 0])
			pj = numpy.array([0, 0, 0])
		dist = numpy.sqrt(((pi - pj) ** 2).sum())

		s = 1
		d = numpy.exp(-dist ** 2 / (2 * (s ** 2)))
		n = self._states_n
		m = d * numpy.identity(n, 'double') + self._eps
		psi_ij = numpy.asmatrix(m)
		return psi_ij

class NoParallelEdgeModel(EdgeModel):
	def __init__(self, bayesian_model, node_model, priors,
					lambda_, eps=0.5):
		EdgeModel.__init__(self, bayesian_model, node_model,
					priors, lambda_)
		self._eps = eps

	def _transmat(self, xi, xj):
		edges = selff._find_xij(xi, xj)


################################################################################
# Loopy Belief Propagation

class LoopyBeliefPropagation(object):
	def __init__(self, node_model, edge_model, graph_data):
		self._node_model = node_model
		self._edge_model = edge_model
		self._graph_data = graph_data
		self._states = node_model._states
		self._states_n = len(self._states)
		print(" * states number = ", self._states_n)
		self._messages = {}
		self._messages2 = {}
		self._transmats = {}
		self._phi_i = {}
		self._believes = {}
		self._old_believes = {}
		z = numpy.asmatrix(numpy.ones(self._states_n)).T
		for xi in graph_data.vertices():
			if xi.getSyntax() != 'fold': continue
			ri = xi['index']
			self._old_believes[ri] = z.copy()
		print("init transition matrices...")
		self._init_messages()

	def _create_full_transmat(self, phi_i, psi_ij):
		# T = psi_ij^t * diag(phi_i)
		return numpy.multiply(psi_ij.T, phi_i).T

	def _init_messages(self):
		# init data : messages
		z = numpy.zeros((self._states_n, 1), numpy.longdouble) + \
						1. / self._states_n
		bar = ProgressionBarPct(len(self._graph_data.vertices()),
							'#', color = 'cyan')
		for i, xi in enumerate(self._graph_data.vertices()):
			bar.display(i)
			if xi.getSyntax() != 'fold': continue
			ri = xi['index']
			phi_i = self._node_model.transmat(xi)
			self._phi_i[ri] = phi_i
			for xj in xi.neighbours():
				if xj.getSyntax() != 'fold': continue
				z = numpy.asmatrix(z)

				rj = xj['index']
				self._messages[ri, rj] = z.copy()
				self._messages2[ri, rj] = z.copy()
				psi_ij = self._edge_model.transmat(xi, xj)
				tij = self._create_full_transmat(phi_i, psi_ij)
				self._transmats[ri, rj] = tij
						

	def _send_message(self, xi, xj, ri, rj):
		prod_mki = numpy.zeros_like(self._messages[ri, rj]) + 1.
		prod_mki = numpy.asmatrix(prod_mki)
		for xk in xi.neighbours():
			if xk.getSyntax() != 'fold': continue
			rk = xk['index']
			if rk == rj : continue
			mki = self._messages[rk, ri]
			prod_mki = numpy.multiply(prod_mki, mki)
		s = prod_mki.sum(axis=0)
		if s: prod_mki /= s
		m = self._transmats[ri, rj] * prod_mki
		s = m.sum()
		if s: m /= s
		else: m[:] = (1. / self._states_n)
		self._messages2[ri, rj] = m

	def _compute_belief(self):
		g = self._graph_data

		for xi in g.vertices():
			if xi.getSyntax() != 'fold' : continue
			prod_mki = numpy.zeros((self._states_n, 1),
						numpy.longdouble) + 1.
			prod_mki = numpy.asmatrix(prod_mki)
			ri = xi['index']
			for xk in xi.neighbours():
				if xk.getSyntax() != 'fold': continue
				rk = xk['index']
				mki = self._messages[rk, ri]
				prod_mki = numpy.multiply(prod_mki, mki)

			s = prod_mki.sum(axis=0)
			if s: prod_mki /= s
			phi_i = self._phi_i[ri]
			b = numpy.multiply(phi_i, prod_mki)
			s = b.sum()
			if s: b /= s
			else: b[:] = (1. / self._states_n)
			#if ri == 27:
			#	ind1 = [i for i, s in enumerate(self._states) \
			#		if s == 'F.I.P.r.int.2_right'][0]
			#	ind2 = [i for i, s in enumerate(self._states) \
			#		if s == 'F.P.O._right'][0]
			#	print("------------------")
			#	print("b = ", b.T, b[ind1], b[ind2])
			#	for xk in xi.neighbours():
			#		if xk.getSyntax() != 'fold': continue
			#		rk = xk['index']
			#		mki = self._messages[rk, ri]
			#		print("rk, mki = ", rk, mki.T, mki[ind1], mki[ind2])
			#	print("prod_mki = ", prod_mki.T, prod_mki[ind1], prod_mki[ind2])
			#	print('phi_i = ', phi_i.T, phi_i[ind1], phi_i[ind2])

			self._believes[ri] = b

	def run(self, csvname, output_graphname, save_all_steps):
		csvsplit = os.path.splitext(csvname)
		graphsplit = os.path.splitext(output_graphname)
		t = 0
		g = self._graph_data
		e0 = 10e100

		# count nodes number
		nodes_n = 0
		for xi in g.vertices():
			if xi.getSyntax() != 'fold': continue
			nodes_n += 1

		while 1:
			t += 1
			print(" ===== %s ===== " % t)
			for xi in g.vertices():
				if xi.getSyntax() != 'fold': continue
				for xj in xi.neighbours():
					if xj.getSyntax() != 'fold': continue
					ri = xi['index']
					rj = xj['index']
					self._send_message(xi, xj, ri, rj)
			self._compute_belief()
			if save_all_steps:
				self.write_csv('%s_%d%s' % (csvsplit[0],
							t, csvsplit[1]))
				w = sigraph.FoldWriter('%s_%d%s' % \
					(graphsplit[0], t, graphsplit[1]))
				w.write(self._graph_data)
			if t > 1:
				e1 = 0.
				for ri in self._believes.keys():
					b1 = self._old_believes[ri]
					b2 = self._believes[ri]
					e1 += numpy.abs(b1 - b2).sum()
				print("e = ", e1, \
					(e1 / (self._states_n * nodes_n)))
				if t > 100 or e1 < 0.01: break
				#if e1 < 0.1 or (e0 - e1 < 0) or t > 100 : break
				e0 = e1

			# switch buffers
			self._messages2, self._messages = \
					self._messages, self._messages2
			self._old_believes, self._believes = \
				self._believes, self._old_believes

	def write_csv(self, csvname):
		g = self._graph_data

		# write csv
		fd = open(csvname, 'w')
		n = self._states_n
		s = 'nodes'
		for si in range(n): s += '\tphi_' + self._states[si]
		for si in range(n): s += '\tbelief_' + self._states[si]
		s += '\targmax_ind\targmax_val'
		s += '\n'
		fd.write(s)
		for xi in g.vertices():
			if xi.getSyntax() != 'fold' : continue
			ri = xi['index']
			phi_i = self._phi_i[ri]
			believes = self._believes[ri]
			arg = numpy.argmax(believes)
			s = str(ri)
			for phi_ij in phi_i: s += '\t%e' % phi_ij
			for belief in believes: s += '\t%e' % belief
			s += '\t%f\t%e' % (arg, believes[arg])
			s += '\n'
			fd.write(s)
			xi['label'] = self._states[arg]
		fd.close()	
	

################################################################################
# main + options

def parseOpts(argv):
	description = 'Tag sulci graph based on a bayesian network model ' \
			'thanks to belief propagation.'
	parser = OptionParser(description)
	add_translation_option_to_parser(parser)
	parser.add_option('-i', '--ingraph', dest='input_graphname',
		metavar = 'FILE', action='store', default = None,
		help='data graph')
	parser.add_option('-o', '--outgraph', dest='output_graphname',
		metavar = 'FILE', action='store',
		default = 'posterior_propagation.arg',
		help='output tagged graph (default : %default)')
	parser.add_option('-m', '--graphmodel', dest='graphmodelname',
		metavar = 'FILE', action='store',
		default = 'bayesian_graphmodel.dat', help='bayesian model : ' \
			'graphical model structure (default : %default)')
	parser.add_option('-d', '--distrib_nodes', dest='distribnodesname',
		metavar = 'FILE', action='store', default = None,
		help='distribution models')
	parser.add_option('--distrib_relations', dest='distribrelname',
		metavar = 'FILE', action='store', default = None,
		help='distribution models')
	parser.add_option('-c', '--csv', dest='csvname',
		metavar = 'FILE', action='store',
		default = 'posterior_propagation.csv',
		help='csv storing posterior probabilities (default : %default)')
	parser.add_option('-p', '--node-prior', dest='node_prior',
		metavar = 'NAME', action='store', default = None,
		help='use priors for nodes (default : no prior), see ' \
		'distrib FILE content to know available priors.')
	parser.add_option('--relation-prior', dest='relation_prior',
		metavar = 'NAME', action='store', default = None,
		help='use priors for relations (default : no prior), see ' \
		'distrib FILE content to know available priors.')
	parser.add_option('--node-type-model', dest='node_model_type',
		metavar = 'TYPE', action='store', default = None,
		help='default : use model given by --distrib_nodes else '\
		'use uniform distribution\n' \
		'uniform : uniform node distribution')
	parser.add_option('--rel-type-model', dest='rel_model_type',
		metavar = 'TYPE', action='store', default = None,
		help='default : use model given by --distrib_relations else '\
		'model given by --distrib_nodes else uniform model\n' \
		'uniform : uniform node distribution\n' \
		'stick : related nodes have more chance ' \
		'to have the same label\n' \
		'stick_distance : same as stick but use ' \
		'distance between nodes\n')
	parser.add_option('--save-all-steps', dest='save_all_steps',
		action='store_true', default = False,
		help='save automatic sulci labels at each step of ' \
		'the propagation process')
	parser.add_option('-l', '--lambda', dest='lambda_',
		action='store', default = 1.,
		help='lambda parameter : (1-lambda) * Cte + lambda * rel_model')

	return parser, parser.parse_args(argv)


def main():
	# option parsing
	parser, (options, args) = parseOpts(sys.argv)
	error_parsing = False
	if (None in [options.input_graphname]):
		print("give an input graph")
		error_parsing = True
	if (options.distribnodesname == None):
		if (options.node_model_type == None):
			print("give a node distrib model : --distrib_nodes " \
				"or --node-type-model")
			error_parsing = True
		if (options.node_prior):
			print("node prior need a node distrib model")
			error_parsing = True
	if (options.distribrelname == None and \
		options.distribnodesname == None and \
		options.relation_prior):
		print("relation prior need a relation distrib " \
				"model or a node distrib model")
		error_parsing = True

	if error_parsing:
		parser.print_help()
		sys.exit(1)

	# read graph_model and distrib models
	print("read...")
	bayesian_model = io.read_bayesian_model(options.graphmodelname,
			options.distribnodesname, options.distribrelname)
	node_distrib = bayesian_model['node_distrib']
	rel_distrib = bayesian_model['rel_distrib']

	graph = io.load_graph(options.transfile, options.input_graphname)

	if node_distrib: node_model_type = node_distrib['model']
	else:	node_model_type = 'undefine'
	if rel_distrib: rel_model_type = rel_distrib['model']
	elif len(bayesian_model['edges']) == 0:
		rel_model_type = 'undefine'
	else:
		node_model_type = node_distrib['model'][0]
		rel_model_type = node_distrib['model'][1]

	if options.node_model_type != None:
		node_model_type = options.node_model_type
	elif node_model_type == 'undefine':
		node_model_type = 'uniform'
	if options.rel_model_type != None:
		rel_model_type = options.rel_model_type
	elif rel_model_type == 'undefine':
		rel_model_type = 'uniform'
	if isinstance(node_model_type, list) or \
		isinstance(node_model_type, tuple):
		node_model_type = node_model_type[0]

	print("node model = ", node_model_type)
	print("rel model = ", rel_model_type)

	# choose node models
	node_opt = [graph, bayesian_model, options.node_prior]
	if node_model_type == 'uniform':
		node_model = UniformVertexModel(*node_opt)
	elif node_model_type == 'gravity_centers' :
		node_model = GravityCenterVertexModel(*node_opt)
	elif node_model_type == 'spam' :
		node_model = SpamVertexModel(*node_opt)

	# choose relation models
	rel_opt = [bayesian_model, node_model, options.relation_prior, \
			float(options.lambda_)]
	if rel_model_type == 'uniform':
		edge_model = UniformEdgeModel(*rel_opt)
	elif rel_model_type == 'stick':
		edge_model = StickEdgeModel(*rel_opt)
	elif rel_model_type == 'stick_distance':
		edge_model = StickDistanceEdgeModel(*rel_opt)
	elif rel_model_type == 'delta_gravity_centers':
		edge_model = GravityCentersDeltaEdgeModel(*rel_opt)
	elif rel_model_type == 'min_distance':
		edge_model = MinDistanceEdgeModel(*rel_opt)

	# propagation tag
	print("loopy belief propagation...")
	lbp = LoopyBeliefPropagation(node_model, edge_model, graph)
	lbp.run(options.csvname, options.output_graphname,
				options.save_all_steps)
	lbp.write_csv(options.csvname)

	# write posteriors
	w = sigraph.FoldWriter(options.output_graphname)
	w.write(graph)

if __name__ == '__main__' : main()
