#!/usr/bin/env python

import sys, os
from optparse import OptionParser
from soma import aims, aimsalgo
from sulci.common import io, add_translation_option_to_parser
from sulci.models.distribution import VonMisesFisher
from sulci.registration import orientation

################################################################################
# main + options
def parseOpts(argv):
	description = 'display orientations of sulci'
	parser = OptionParser(description)
	add_translation_option_to_parser(parser)
	parser.add_option('-g', '--graph', dest='graphname', metavar='FILE',
		action='store', default = None, help='input sulci graph')
	parser.add_option('-s', '--sulcus', dest='sulcus',
		metavar = 'NAME', action='store', default = None,
		help='Compute spam only for one sulcus ' \
			'(default : compute all spams)')
	return parser, parser.parse_args(argv)


def main():
	parser, (options, args) = parseOpts(sys.argv)
	if None in [options.graphname]:
		print "error: missing option(s)"
		parser.print_help()
		sys.exit(1)

	hie_filename = os.path.join(aims.carto.Paths.shfjShared(),
					'nomenclature', 'hierarchy',
					'sulcal_root_colors.hie')
	hie = aims.Reader().read(hie_filename)
	graph = io.load_graph(options.transfile, options.graphname)
	h = {}
	for v in graph.vertices():
		if v.getSyntax() != 'fold': continue
		label = v['name']
		if options.sulcus and label != options.sulcus: continue
		edir = None
		for e in v.edges():
			if e.getSyntax() != 'hull_junction': continue
			try: edir = e['refdirection'].arraydata()
			except KeyError: pass
		g = v['refgravity_center'].arraydata()
		dir = v['refdirection'].arraydata()
		hull_normal = v['refhull_normal'].arraydata()
		depth_direction = v['refdepth_direction'].arraydata()
		normal = v['refnormal'].arraydata()
		o = orientation.get_sulci_rotation_axe(v)
		data = (g, edir, dir, o, hull_normal, depth_direction, normal)
		if h.has_key(label):
			h[label].append(data)
		else:	h[label] = [data]

	writer = aims.Writer()
	for sulcus, data in h.items():
		color = hie.find_color(sulcus)
		go =  {'diffuse' : color}

		dmesh = aims.AimsSurfaceTriangle()
		emesh = aims.AimsSurfaceTriangle()
		omesh = aims.AimsSurfaceTriangle()
		hmesh = aims.AimsSurfaceTriangle()
		ddmesh = aims.AimsSurfaceTriangle()
		nmesh = aims.AimsSurfaceTriangle()
		for one_data in data:
			(g, e, d, o, h, dd, n) = one_data
			dmesh += aims.SurfaceGenerator.arrow(
				g - d * 10, g, 1., 2., 10, 0.3)
			if e is not None:
				emesh += aims.SurfaceGenerator.arrow(
					g - e * 10, g, 1., 2., 10, 0.3)
			if o is not None:
				omesh += aims.SurfaceGenerator.arrow(
					g - o * 10, g, 1., 2., 10, 0.3)
			hmesh += aims.SurfaceGenerator.arrow(
				g - h * 10, g, 1., 2., 10, 0.3)
			ddmesh += aims.SurfaceGenerator.arrow(
				g - dd * 10, g, 1., 2., 10, 0.3)
			nmesh += aims.SurfaceGenerator.arrow(
				g - n * 10, g, 1., 2., 10, 0.3)
		dmesh.header()['material'] = go
		emesh.header()['material'] = go
		omesh.header()['material'] = go
		hmesh.header()['material'] = go
		ddmesh.header()['material'] = go
		nmesh.header()['material'] = go
		writer.write(dmesh, 'dir_%s.mesh' % sulcus)
		writer.write(emesh, 'edgedir_%s.mesh' % sulcus)
		writer.write(omesh, 'orientation_%s.mesh' % sulcus)
		writer.write(hmesh, 'hull_normal_%s.mesh' % sulcus)
		writer.write(ddmesh, 'depth_direction_%s.mesh' % sulcus)
		writer.write(nmesh, 'normal_%s.mesh' % sulcus)


if __name__ == '__main__' : main()
