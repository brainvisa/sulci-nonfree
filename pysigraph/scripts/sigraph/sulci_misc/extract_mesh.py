#!/usr/bin/env python
from __future__ import print_function
from __future__ import absolute_import
import re, os, sys
from optparse import OptionParser
from soma import aims

################################################################################
TOP_STATE = 0
GRAPH_STATE = 1
NODE_STATE = 2
UNKNOWN_STATE = 255

def read_graph(graphname, sulci):
	tag_map = {
		'GRAPH' : GRAPH_STATE,
		'NODE' : NODE_STATE,
	}
	states = [TOP_STATE]
	fd = open(graphname)
	sulcus = 'none'
	ind = -1
	indices = []
	for line in fd.readlines():
		m = re.match('\*BEGIN (\w*) (.*)\n', line)
		if m:
			category, dummy = m.groups()
			states.append(tag_map.get(category, UNKNOWN_STATE))
			continue
		m = re.match('\*END', line)
		if m:
			if states[-1] == NODE_STATE:
				if sulcus in sulci: indices.append(int(ind))
				sulcus = 'none'
			states.pop()
			continue
		if states[-1] == GRAPH_STATE:
			m = re.match('filename_base[ \t]*([\*\w\.]*)', line)
			if m:
				data_dir = m.groups()[0]
				continue
			m = re.match('fold.global.tri[ \t]*([\w]*) (\w*) (\w*)',
									line)
			if m:
				dummy1, tmtktri, dummy2 = m.groups()
				continue
		elif states[-1] == NODE_STATE:
			m = re.match('Tmtktri_label[ \t]*(\w*)', line)
			if m:
				ind = m.groups()[0]
				continue
			m = re.match('name[ \t]*([\w.]*)', line)
			if m:
				sulcus = m.groups()[0]
				continue
	fd.close()
	if data_dir == '*':
		data_dir = re.sub('\.arg$', '.data',
			os.path.basename(graphname))
	meshname = os.path.join(data_dir, tmtktri + '.mesh')
	return meshname, indices

def extract_sulci_from_graph(graphname, sulci):
	writer = aims.Writer()
	subject = re.sub('\.arg$', '', os.path.basename(graphname))
	meshname, indices = read_graph(graphname, sulci)
	meshname = os.path.join(os.path.dirname(graphname), meshname)
	mesh = aims.Reader().read(meshname)
	for ind in indices:
		new_mesh = aims.AimsSurfaceTriangle()
		v = mesh.vertex(ind)
		new_mesh.vertex().assign(v)
		n = mesh.normal(ind)
		new_mesh.normal().assign(n)
		p = mesh.polygon(ind)
		new_mesh.polygon().assign(p)
		filename = '%s_%d.mesh' % (subject, ind)
		writer.write(new_mesh, filename)


################################################################################
def parseOpts(argv):
	description = 'Extract sulci mesh from several graphs\n' + \
		'./extract_mesh.py -s SULCI graph1.arg graph2.arg...'
	parser = OptionParser(description)
	parser.add_option('-s', '--sulci', dest='sulci',
		metavar = 'LIST', action='store', default = None,
		help='tag only specified manually tagged sulci.')

	return parser, parser.parse_args(argv)


def main():
	parser, (options, args) = parseOpts(sys.argv)
	graphnames = args[1:]
	if None in [options.sulci]:
		print('error: missing options')
		parser.print_help()
		sys.exit(1)

	for i, graphname in enumerate(graphnames):
		sys.stdout.write('\rgraphs : %d/%d' % (i + 1, len(graphnames)))
		sys.stdout.flush()
		extract_sulci_from_graph(graphname, options.sulci)
	print()


if __name__ == '__main__': main()
