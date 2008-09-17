import os, numpy
from soma import aims

def add_translation_option_to_parser(parser):
	transfile = os.path.join(aims.carto.Paths.shfjShared(), 'nomenclature',
		'translation', 'sulci_model_noroots.trl')

	parser.add_option('-t', '--translation', dest='transfile',
		metavar = 'FILE', action='store', default = transfile,
		help='translation file (default : %default)')


def vertex2voxels(motion, vertex, data_type):
	if data_type == 'simple_surface': map = vertex['aims_ss'].get()
	elif data_type == 'bottom': map = vertex['aims_bottom'].get()
	s = numpy.array([map.sizeX(), map.sizeY(), map.sizeZ()])
	vox = [motion.transform(aims.Point3df(p * s)) for p in map[0].keys()]
	return vox
