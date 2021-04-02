#!/usr/bin/env python
import os, sys, numpy, pprint
from optparse import OptionParser
import sigraph
from soma import aims, aimsalgo
import datamind.io.old_csvIO as datamind_io
from sulci.common import io, add_translation_option_to_parser
from sulci.models import distribution

def parseOpts(argv):
	description = 'Compute Spam from a list of graphs.\n' \
	'learn_spams_distributions.py [OPTIONS] graph1.arg graph2.arg...'
	parser = OptionParser(description)
	add_translation_option_to_parser(parser)
	return parser, parser.parse_args(argv)


def compile_sulci(g):
	h = {}
	for v in g.vertices():
		if v.getSyntax() != 'fold': continue
		sulcus = v['name']
		if h.has_key(sulcus):
			h[sulcus].append(v)
		else:	h[sulcus] = [v]
	return h


def compute_size_weighted(h, feature):
	features_map = {}
	for sulcus, vertices in h.items():
		f = numpy.zeros(3)
		size_total = 0.
		for v in vertices:
			size = v['size']
			size_total += size
			f += v[feature].arraydata() * size
		features_map[sulcus] = f / size_total
	return features_map


def compute_gravity_centers(h):
	return compute_size_weighted(h, 'refgravity_center')


def compute_normals(h):
	return compute_size_weighted(h, 'refnormal')

def compute_direction(h): pass
	# dans les relations

def compute_size(h):
	sizes = {}
	for sulcus, vertices in h.items():
		s = 0.
		for v in vertices: s += v['size']
		sizes[sulcus] = s
	return sizes


def tag_neighbours(v, idmap, id, syntax_filters):
	r = v['index']
	if idmap.has_key(r): return
	idmap[r] = id
	idmap[r] = id
	for e in v.edges():
		if not e.getSyntax() in syntax_filters: continue
		v1, v2 = e.vertices()
		r1 = v1['index']
		r2 = v2['index']
		if r == r2:
			v1, v2 = v2, v1
			r1, r2 = r2, r1
		if v1['name'] != v2['name']: continue
		tag_neighbours(v2, idmap, id, syntax_filters)


def compute_components_number(h):
	n = {}
	for sulcus, vertices in h.items():
		idmap = {}
		for id, v in enumerate(vertices):
			tag_neighbours(v, idmap, id, \
				['junction', 'plidepassage'])
		if hasattr( numpy, 'unique1d' ):
			n[sulcus] = len(numpy.unique1d(idmap.values()))
		else:
			n[sulcus] = len(numpy.unique(idmap.values()))
	return n


def compute_sulci_components_number(h):
	return compute_components_number(h, ['junction', 'plidepassage'])


def compute_graph_components_number(h):
	return compute_components_number(h, \
		['junction', 'plidepassage', 'cortical'])


def compute_extremities(h):
	extremities = {}
	for sulcus, vertices in h.items():
		dots = []
		for v in vertices:
			for e in v.edges():
				if not e.getSyntax() in ['junction', \
					'plidepassage', 'cortical']:
					continue
				try:
					e1 = e['refextremity1']
					e2 = e['refextremity1']
				except KeyError:
					#FIXME : calculer autrement
					continue
				dots += [e1, e2]
		n = len(dots)
		if n == 0: continue
		dots = numpy.array(dots)
		dist = numpy.array([sum((dots - p)**2, 1) for p in dots])
		ind = numpy.argmax(dist.ravel())
		ind = ind / n, ind % n
		extremities[sulcus] = dots[ind[0]], dots[ind[1]]
	#FIXME : pb d'orientation des extermites
	return extremities
				


def main():
	parser, (options, args) = parseOpts(sys.argv)
	graphnames = args[1:]
	if len(graphnames) == 0:
		parser.print_help()
		sys.exit(1)
	
	graphs = io.load_graphs(options.transfile, graphnames)

	for g in graphs:
		h = compile_sulci(g)
		#compute_gravity_centers(h)
		#compute_size(h)
		#compute_extermites(h)
		#compute_graph_components_number(h)
		#compute_normals(h)
		compute_extremities(h)



if __name__ == '__main__' : main()
