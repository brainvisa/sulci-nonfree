#!/usr/bin/env python
from __future__ import print_function
import os, sys
from optparse import OptionParser
import sigraph
from soma import aims
from sulci.common import io, add_translation_option_to_parser

################################################################################
def parseOpts(argv):
	description = 'display registred graphs' + \
	'display_registred_graphs.py [OPTIONS] graph1.arg graph2.arg...\n'
	parser = OptionParser(description)
	add_translation_option_to_parser(parser)
	parser.add_option('-i', '--ingraph', dest='input_graphname',
		metavar = 'FILE', action='store', default = None,
		help='data graph')
	parser.add_option('-o', '--outgraph', dest='output_graphname',
		metavar = 'FILE', action='store',
		default = 'posterior_independent.arg',
		help='output tagged graph (default : %default)')
	parser.add_option('-s', '--sulci', dest='sulci',
		metavar = 'LIST', action='store', default = None,
		help='move only specified sulci')
	parser.add_option('--global-motion', dest='global_motion_name',
		metavar = 'FILE', action='store', default = None,
		help='global registration motion')
	parser.add_option('--local-motions', dest='local_motions_name',
		metavar = 'FILE', action='store', default = None,
		help='sulcuswise registration motions')
	parser.add_option('--label-type', dest='label_type',
		metavar = 'TYPE', action='store', default = 'name',
		help='name (real labels) or label (automatic labelling) ' + \
		'(default : name)')
	parser.add_option('--no-talairach', dest='no_talairach',
		action='store_true', default = False,
		help='if specified does not apply global transformation to ' + \
		'Talairach space (by default the transformation is read ' + \
		'from the graph itself')

	return parser, parser.parse_args(argv)


def main():
	parser, (options, args) = parseOpts(sys.argv)
	if None in [options.input_graphname, options.output_graphname]:
		print("error: forgotten option")
		parser.print_help()
		sys.exit(1)

	if not (options.label_type in ['name', 'label']):
		print("unknown label type '%s'" % options.label_type)
		parser.print_help()
		sys.exit(1)

	if options.sulci is None:
		selected_sulci = None
	else:	selected_sulci = options.sulci.split(',')

	# reading...
	graph = io.load_graph(options.transfile, options.input_graphname)
	if options.no_talairach:
		motion_tal = aims.Motion()
		motion_tal.setToIdentity()
	else:	motion_tal = aims.GraphManip.talairach(graph)
	if options.global_motion_name:
		motion_global = aims.Reader().read(options.global_motion_name)
	else:	motion_global = None
	if options.local_motions_name:
		motions_local = io.read_from_exec(options.local_motions_name,
							'transformations')
		prefix = os.path.dirname(options.local_motions_name)
		for sulcus, filename in motions_local.items():
			f = os.path.join(prefix, filename)
			motions_local[sulcus] = aims.Reader().read(f)
	else:	motions_local = None

	meshes = {}
	for v in graph.vertices():
		if v.getSyntax() != 'fold' : continue
		sulcus = v[options.label_type]
		if selected_sulci is not None and \
			sulcus not in selected_sulci: continue
		if motion_global:
			motion_total = motion_global * motion_tal
		else:	motion_total = motion_tal
		if motions_local:
			try:
				motion_local = motions_local[sulcus]
			except KeyError: pass
			finally:
				motion_total = motion_local * motion_total
		else:	motion_total = motion_total
		mesh = v['aims_Tmtktri'].get()
		aims.SurfaceManip.meshTransform(mesh, motion_total)
	graph['filename_base'] = '*'

	w = sigraph.FoldWriter(options.output_graphname)
	w.write(graph)

if __name__ == '__main__' : main()
