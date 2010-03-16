#!/usr/bin/env python

import os, sys
import numpy

from soma import aims
from sigraph import *
from sulci.common import io, add_translation_option_to_parser
from optparse import OptionParser

# Options parser
def parseOpts(argv):
	hierarchy = os.path.join(aims.carto.Paths.shfjShared(),
					'nomenclature', 'hierarchy',
					'sulcal_root_colors.hie')
	description = 'Convert Aims data fold graph to vrml1 format.'
	parser = OptionParser(description)
	add_translation_option_to_parser(parser)
	parser.add_option('-i', '--input', dest='inputname',
		metavar = 'FILE', action='store', default = None,
		help='data fold graph')
	parser.add_option('-o', '--output', dest='outputname',
		metavar = 'FILE', action='store', default = None,
		help='vrml file')
	parser.add_option('--label-mode', dest='label_mode',
		metavar = 'STR', type='choice', choices=('name', 'label'),
		action='store', default='name',
		help="'name': manual label, 'label': automatic label " + \
		"(default: %default)")
	parser.add_option('--hierarchy', dest='hierarchy',
		metavar = 'FILE', action='store', default = hierarchy,
		help='hierarchy (links between names and colors), ' + \
		'default : %default')

	return parser, parser.parse_args(argv)

################################################################################
# VRML sections

def writeExtendedMaterial(fd, vertex, name=None):
	if name is not None: fd.write('\t\tDEF %s\n' % name)
	fd.write('\t\tExtendedMaterial {\n')
	fd.write('\t\t\tfields [ SFLong index, ' + \
		'SFString label, SFString name ]\n')
	fd.write('\t\t\tindex %d\n' % vertex["index"])
	if vertex.has_key('label'):
		fd.write('\t\t\tlabel "%s"\n' % vertex['label'])
	if vertex.has_key('name'):
		fd.write('\t\t\tname "%s"\n' % vertex["name"])
	fd.write('\t\t}\n')

def writeMaterial(fd, properties={}, name=None):
	properties_default = { \
		'diffuseColor' :  [0.5, 0.5, 0.5],
		'specularColor' : [1, 1, 1],
		'shininess' : [0.5],
		'transparency' : [0.]}
	properties_default.update(properties)
	if name is not None: fd.write('\t\tDEF %s\n' % name)
	fd.write('\t\tMaterial {\n')
	for k, v in properties_default.items():
		fd.write('\t\t\t%s %s\n' % (k, ' '.join(str(x) for x in v)))
	fd.write('\t\t}\n')

def writeCoordinate3(fd, points):
	fd.write('\t\tCoordinate3 {\n')
	fd.write('\t\t\tpoint [\n')
	for p in points: fd.write('\t\t\t\t%6.6f %6.6f %6.6f,\n' % tuple(p))
	fd.write('\t\t\t]\n')
	fd.write('\t\t}\n')

def writeIndexedFaceSet(fd, faces):
	fd.write('\t\tIndexedFaceSet {\n')
	fd.write('\t\t\tcoordIndex [\n')
	for f in faces: fd.write('\t\t\t\t%6d, %6d, %6d, -1,\n' % tuple(f))
	fd.write('\t\t\t]\n')
	fd.write('\t\t}\n')


def writeGraph(fd, g, hie, label_mode):
	'''
    fd:  output
    g:   input fold graph
    hie: hierarchy
	'''
	fd.write("#VRML V1.0 ascii\n")
	fd.write("#Generated from the following command:\n")
	fd.write('# ' + ' '.join(sys.argv) + '\n')
	fd.write("\nSeparator {\n")
	for v in g.vertices():
		if v.getSyntax() != 'fold': continue
		sulcus = v[label_mode]
		m = v['aims_Tmtktri']
		color = hie.find_color(sulcus)
		fd.write("\tDEF Segment%d\n" % v['index'])
		fd.write("\tSeparator {\n")
		writeExtendedMaterial(fd, v, 'Meta')
		writeMaterial(fd, {'diffuseColor' : color})
		writeCoordinate3(fd, numpy.array(m.vertex()))
		writeIndexedFaceSet(fd, numpy.array(m.polygon()))
		fd.write("\t}\n")
	fd.write("}\n")


################################################################################
# main function
def main():
	# read options
	parser, (options, args) = parseOpts(sys.argv)
	if None in [options.inputname, options.outputname]:
		print "error: missing input or output."
		parser.print_help()
		sys.exit(1)

	# read model
	r = aims.Reader()
	g = io.load_graph(options.transfile,
		options.inputname, options.label_mode)
	hie = aims.Reader().read(options.hierarchy)

	fd = open(options.outputname, 'w')
	writeGraph(fd, g, hie, options.label_mode)
	fd.close()


if __name__ == '__main__' : main()
