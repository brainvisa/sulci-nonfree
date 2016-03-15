#!/usr/bin/env python2

import os, sys
from optparse import OptionParser
from soma import aims
from sulci.common import io

def parseOpts(argv):
	description = 'convert volume of labels with a sulci map to ' + \
		'a graph of ROI\n./convert_VolumeOfLabels_to_ROI.py -i ' + \
		'input_vol -m sulci_map -o roigraph.arg'

	parser = OptionParser(description)
	parser.add_option('-i', '--input', dest='input_volname',
		metavar = 'FILE', action='store', default = None,
		help='volume of labels')
	parser.add_option('-m', '--sulci-map', dest='input_sulci_map',
		metavar = 'FILE', action='store', default = None,
		help="sulci map : file format : sulci_map = " + \
		"{ 1 : 'S.C._right'}")
	parser.add_option('--tmp', dest='tmp_volname',
		metavar = 'FILE', action='store', default = 'tmp.ima',
		help='temporary file for volume convertion')
	parser.add_option('-o', '--output', dest='output_roigraph',
		metavar = 'FILE', action='store', default = None,
		help='output ROI graph')
	return parser, parser.parse_args(argv)

def main():
	parser, (options, args) = parseOpts(sys.argv)
	input_volname = options.input_volname
	input_sulci_map = options.input_sulci_map
	tmp_volname = options.tmp_volname
	output_roigraph = options.output_roigraph
	if None in [input_volname, input_sulci_map, output_roigraph]:
		print 'error : missing parameter'
		sys.exit(1)

	finder = aims.Finder()
	finder.check(input_volname)
	data_type = finder.header()['data_type']
	if data_type == 'U8':
		input_vol = aims.Reader().read(input_volname)
		# data U8 -> S16
		sX = input_vol.getSizeX() 
		sY = input_vol.getSizeY() 
		sZ = input_vol.getSizeZ() 
		sT = input_vol.getSizeT() 
		tmp_vol = aims.Volume_S16(sX, sY, sY, sT)
		conv = aims.Converter_Volume_U8_Volume_S16()
		conv.convert(input_vol, tmp_vol)
		aims.Writer().write(tmp_vol, tmp_volname)
		input_volname = tmp_volname
	elif data_type != 'S16':
		print "datatype '%s' not supported" % data_type
		sys.exit(1)

	# read S16 data
	r = aims.Reader({ 'Volume' : { 'S16' : 'Graph' }})
	graph = r.read(input_volname)
	graph['filename_base'] = '*'
	aims.GraphManip.volume2Buckets(graph)
	sulci_map = io.read_from_exec(input_sulci_map, 'sulci_map')
	for v in graph.vertices():
		ind = v['roi_label']
		v['name'] = sulci_map[ind]
	aims.write(graph, output_roigraph)


if __name__ == '__main__' : main()
