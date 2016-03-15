#!/usr/bin/env python2
import re, os, sys, numpy
import sigraph
from sulci.common import io, add_translation_option_to_parser
from optparse import OptionParser
from soma import aims

################################################################################
TOP_STATE = 0
GRAPH_STATE = 1
NODE_STATE = 2
UNKNOWN_STATE = 255

def read_graph(graphname, ft, selected_sulci):
	tag_map = {
		'GRAPH' : GRAPH_STATE,
		'NODE' : NODE_STATE,
	}
	states = [TOP_STATE]
	fd = open(graphname)
	sulcus = 'none'
	ind = -1
	voxels_n = {}
	vox_n = None
	for line in fd.readlines():
		# BEGIN
		m = re.match('\*BEGIN (\w*) (.*)\n', line)
		if m:
			category, dummy = m.groups()
			states.append(tag_map.get(category, UNKNOWN_STATE))
			continue

		# END
		m = re.match('\*END', line)
		if m:
			if states[-1] == NODE_STATE:
				if vox_n is not None and \
					((selected_sulci is None) or \
					(sulcus in selected_sulci)):
					if voxels_n.has_key(sulcus):
						voxels_n[sulcus].append(vox_n)
					else:	voxels_n[sulcus] = [vox_n]
				sulcus = 'none'
				vox_n = None
			states.pop()
			continue

		# STATES
		if states[-1] == GRAPH_STATE:
			continue
		elif states[-1] == NODE_STATE:
			m = re.match('point_number[ \t]*(\d*)', line)
			if m:
				vox_n = int(m.groups()[0])
				continue
			m = re.match('name[ \t]*([\w.-]*)', line)
			if m:
				sulcus = ft.lookupLabel(m.groups()[0])
				continue
	fd.close()
	return voxels_n

################################################################################
def parseOpts(argv):
	description = 'Compute total number of voxels of given sulci over ' + \
		'all sujbects\n' + \
		'./compute_sulci_voxels_number.py -s SULCI graph1.arg graph2.arg...'
	parser = OptionParser(description)
	add_translation_option_to_parser(parser)
	parser.add_option('-s', '--sulci', dest='selected_sulci',
		metavar = 'LIST', action='store', default = None,
		help='tag only specified manually tagged sulci.')

	return parser, parser.parse_args(argv)


def main():
	parser, (options, args) = parseOpts(sys.argv)
	graphnames = args[1:]
	if len(graphnames) == 0:
		print 'error: missing graph (at least one)'
		parser.print_help()
		sys.exit(1)
	ft = sigraph.FoldLabelsTranslator(options.transfile)

	resume = {}
	for i, graphname in enumerate(graphnames):
		sys.stdout.write('\rgraphs : %d/%d' % (i + 1, len(graphnames)))
		sys.stdout.flush()
		voxels_n = read_graph(graphname, ft, options.selected_sulci)
		for sulcus, numbers in voxels_n.items():
			n = numpy.sum(numbers)
			if resume.has_key(sulcus):
				resume[sulcus] += n
			else:	resume[sulcus] = n
	print
	print "voxels_number = ", resume


if __name__ == '__main__': main()
