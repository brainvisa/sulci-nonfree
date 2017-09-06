#!/usr/bin/env python2
from __future__ import print_function
import os, sys, exceptions, numpy
from optparse import OptionParser
import sigraph
import anatomist.direct.api as anatomist
from soma import aims, aimsalgo
import fff.GMM
import datamind.io.old_csvIO as datamind_io
from datamind.tools import *
from sulci.common import io
qt4=True
import soma.qt_gui.qt_backend.QtGui as qt

a = anatomist.Anatomist()


def parseOpts(argv):
	description = 'Display learned bayesian local models ' \
			'(one for each sulci).'
	parser = OptionParser(description)
	parser.add_option('-b', '--database', dest='database',
		metavar = 'FILE', action='store',
		default = 'bayesian_voxels_databases.dat',
		help='databases summary file (default : %default).')
	parser.add_option('-s', '--sulcus', dest='sulcus',
		metavar = 'NAME', action='store', default = None,
		help='Compute mesh only for specified sulcus ' \
			'(default : compute all meshes)')
	return parser, parser.parse_args(argv)

def main():
	parser, (options, args) = parseOpts(sys.argv)
	if None in [options.database]:
		parser.print_help()
		sys.exit(1)

	database = io.read_databaselist(options.database)

	if database['data'] != 'voxels_coordinates':
		print('database data type must be : voxels_coordinates')
		sys.exit(1)

	hie_filename = os.path.join(aims.carto.Paths.shfjShared(),
					'nomenclature', 'hierarchy',
					'sulcal_root_colors.hie')
	hie = aims.Reader().read(hie_filename)
	unit_sphere = aims.SurfaceGenerator.cube([0, 0, 0], 0.1)
	writer = aims.Writer()

	aobjects = []
	for sulcus, minfname in database['files'].items():
		if options.sulcus != None and options.sulcus != sulcus:
			continue
		db, header = datamind_io.ReaderMinfCsv().read(minfname)
		color = hie.find_color(sulcus)
		all_mesh = aims.AimsTimeSurface_3_VOID()
		for x in db.getX():
			mesh = aims.AimsTimeSurface_3_VOID(unit_sphere)
			motion = aims.Motion()
			r = numpy.random.normal(0, 0.5)
			motion.setTranslation(x + r)
			aims.SurfaceManip.meshTransform(mesh, motion)
			all_mesh += mesh

		go =  aims.Object({'diffuse' : color})
		all_mesh.header()['material'] = go
		material = anatomist.cpp.Material()
		material.set(go.get())
		amesh = a.toAObject(all_mesh)
		amesh.SetMaterial(material)
		aobjects += [amesh]
		writer.write(all_mesh, 'voxels_database_%s.mesh' % sulcus)

	awin = a.createWindow('3D')
	a.addObjects(aobjects, [awin])
	if qt4:
		qt.qApp.exec_()
	else:
		qt.qApp.exec_loop()

	

if __name__ == '__main__' : main()
