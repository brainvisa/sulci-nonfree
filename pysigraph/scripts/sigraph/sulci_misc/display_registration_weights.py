#!/usr/bin/env python
from __future__ import print_function
import os, sys
from collections import defaultdict
import numpy
from optparse import OptionParser
from soma import aims
import datamind.io
from sulci.common import io, add_translation_option_to_parser
from sulci.features.descriptors import GravityCenterSegmentDescriptor

################################################################################
def parseOpts(argv):
	hie_filename = os.path.join(aims.carto.Paths.shfjShared(),
					'nomenclature', 'hierarchy',
					'sulcal_root_colors.hie')
	description = 'Display learned bayesian local models ' \
			'(one for each sulci).'
	parser = OptionParser(description)
	add_translation_option_to_parser(parser)
	parser.add_option('-d', '--distrib', dest='distribname',
		metavar='FILE', action='store', default=None,
		help='center gravity gaussian distribution models')
	parser.add_option('-g', '--graph', dest='graphname',
		action='store', default=None, help='graph subject')
	parser.add_option('-s', '--sulci', dest='sulci',
		metavar='LIST', action='store', default=None,
		help='display information only for specified sulci label')
	parser.add_option('-p', '--posterior', dest='posterior',
		metavar='FILE', action='store', default=None,
		help='posterior file')
	parser.add_option('--threshold', dest='threshold', type='float',
		metavar='FLOAT', action='store', default=0.01,
		help='keep only links over the specified threshold')
	parser.add_option('--hierarchy', dest='hie', metavar='FILE',
		action='store', default=hie_filename,
		help='hierarchy (used to get color of labels) ' + \
		'(default : %default)')
	parser.add_option('--mode', dest='mode', type='choice',
		choices=('posterior', 'labels', 'labels_mean'),
		metavar='MODE', action='store', default='labels',
		help="'posterior' (use --posterior), 'labels' or 'labels_mean'")
	return parser, parser.parse_args(argv)

def main():
	parser, (options, args) = parseOpts(sys.argv)
	if None in [options.distribname, options.graphname]:
		print("missing gaussian distrib or input sulci graph")
		parser.print_help()
		sys.exit(1)
	if options.mode == 'posterior':
		if options.posterior is None:
			print("missing posterior file")
			parser.print_help()
			sys.exit(1)
	if options.sulci is None:
		selected_sulci = None
	else:	selected_sulci = options.sulci.split(',')

	# read
	gaussians_distrib = io.read_segments_distrib(\
		options.distribname, selected_sulci)
	gaussians_distrib = gaussians_distrib['vertices']
	graph = io.load_graph(options.transfile, options.graphname)
	hie = aims.Reader().read(options.hie)
	if options.mode == 'posterior':
		main_posterior_mode(options, gaussians_distrib,
					graph, hie, selected_sulci)
	elif options.mode == 'labels':
		main_labels_mode(options, gaussians_distrib,
					graph, hie, selected_sulci)
	elif options.mode == 'labels_mean':
		main_labels_mean_mode(options, gaussians_distrib,
					graph, hie, selected_sulci)

def main_labels_mode(options, gaussians_distrib, graph, hie, selected_sulci):
	sulci = gaussians_distrib.keys()
	meshes = {}
	r = 1 # cylinder radius
	for v in graph.vertices():
		if v.getSyntax() != 'fold': continue
		g1 = v['refgravity_center'].arraydata()
		sulcus = v['name']
		index = v['index']
		if selected_sulci is not None and \
			sulcus not in selected_sulci: continue
		color = hie.find_color(sulcus)
		gd = gaussians_distrib[sulcus]
		g2 = numpy.asarray(gd.mean()).ravel()
		c = aims.SurfaceGenerator.cylinder(g1, g2, r, r, 6, True, True)
		go =  {'diffuse' : list(color) + [1.]}
		c.header()['material'] = go
		meshes[(sulcus, index)] = c

	for (sulcus, ind), m in meshes.items():
		aims.Writer().write(m, '%s_%d.mesh' % (sulcus, ind))

def main_labels_mean_mode(options, gaussians_distrib, graph, hie, selected_sulci):
	sulci = gaussians_distrib.keys()
	r = 1 # cylinder radius
	coords = defaultdict(list)
	sizes = defaultdict(list)
	for v in graph.vertices():
		if v.getSyntax() != 'fold': continue
		g = v['refgravity_center'].arraydata()
		sulcus = v['name']
		size = v['size']
		index = v['index']
		if selected_sulci is not None and \
			sulcus not in selected_sulci: continue
		coords[sulcus].append(g)
		sizes[sulcus].append(size)
	meshes = {}
	for sulcus, X in coords.items():
		color = hie.find_color(sulcus)
		W = numpy.array(sizes[sulcus])
		W /= W.sum()
		n = len(X)
		g1 = (W[None].T * numpy.vstack(X)).sum(axis=0)
		#g1 = (numpy.vstack(X)).mean(axis=0)
		gd = gaussians_distrib[sulcus]
		g2 = numpy.asarray(gd.mean()).ravel()
		c = aims.SurfaceGenerator.cylinder(g2, g1, r, r, 6, True, True)
		v = (g2 - g1)
		v = 4 * v / numpy.linalg.norm(v)
		c2 = aims.SurfaceGenerator.cone(g2+v, g2, r*2, 6, True, True)
		c += c2
		go =  {'diffuse' : list(color) + [1.]}
		c.header()['material'] = go
		meshes[sulcus] = c

	for sulcus, m in meshes.items():
		aims.Writer().write(m, '%s.mesh' % (sulcus))

def main_posterior_mode(options, gaussians_distrib, graph, hie, selected_sulci):
	r = datamind.io.ReaderCsv()
	X = r.read(options.posterior)
	colnames = [s.strip('proba_') for s in X.colnames()]
	sulci = gaussians_distrib.keys()
	meshes = {}
	r = 1 # cylinder radius
	for v in graph.vertices():
		if v.getSyntax() != 'fold': continue
		g1 = v['refgravity_center'].arraydata()
		index = v['index']
		P = X[numpy.argwhere(X[:, 'nodes'] == index)[0, 0]]
		for ind in numpy.argwhere(P >= options.threshold).ravel():
			if ind < 4: continue
			sulcus = colnames[ind]
			if selected_sulci is not None and \
				sulcus not in selected_sulci: continue
			val = P[ind] # posterior P(L_i|D_i)
			color = hie.find_color(sulcus)
			gd = gaussians_distrib[sulcus]
			g2 = numpy.asarray(gd.mean()).ravel()
			c = aims.SurfaceGenerator.cylinder(g1, g2, r, r, 6,
								True, True)
			go =  {'diffuse' : list(color) + [val]}
			c.header()['material'] = go
			meshes[(sulcus, index)] = c


if __name__ == '__main__' : main()
