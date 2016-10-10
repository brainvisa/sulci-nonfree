#!/usr/bin/env python2
from __future__ import print_function
import os, sys
import numpy
from optparse import OptionParser
from soma import aims
import datamind.io
from sulci.common import io
from sulci.features.descriptors import GravityCenterSegmentDescriptor

################################################################################
def parseOpts(argv):
	hie_filename = os.path.join(aims.carto.Paths.shfjShared(),
					'nomenclature', 'hierarchy',
					'sulcal_root_colors.hie')
	description = 'Display learned bayesian local models ' \
			'(one for each sulci).'
	parser = OptionParser(description)
	parser.add_option('-d', '--distrib', dest='distribname',
		metavar='FILE', action='store', default=None,
		help='center gravity gaussian distribution models')
	parser.add_option('--hierarchy', dest='hie', metavar='FILE',
		action='store', default=hie_filename,
		help='hierarchy (used to get color of labels) ' + \
		'(default : %default)')
	return parser, parser.parse_args(argv)



def Axes():
	d1 = numpy.array([1, 0, 0])
	d2 = numpy.array([0, 1, 0])
	d3 = numpy.array([0, 0, 1])
	g = (0, 0, 0)
	r = 0.1
	n = 50
	m = aims.SurfaceGenerator.cylinder(g, d1, r, r, n, True, True)
	m += aims.SurfaceGenerator.cylinder(g, d2, r, r, n, True, True)
	m += aims.SurfaceGenerator.cylinder(g, d3, r, r, n, True, True)
	m += aims.SurfaceGenerator.cone(d1 * 1.4, d1, 2 * r, n, True, True)
	m += aims.SurfaceGenerator.cone(d2 * 1.4, d2, 2 * r, n, True, True)
	m += aims.SurfaceGenerator.cone(d3 * 1.4, d3, 2 * r, n, True, True)
	m += aims.SurfaceGenerator.sphere(g, r, 1024)
	return m


def main():
	parser, (options, args) = parseOpts(sys.argv)
	if None in [options.distribname]:
		print("missing gaussian distrib")
		parser.print_help()
		sys.exit(1)
	# read
	gaussians_distrib = io.read_segments_distrib(\
		options.distribname, None)
	gaussians_distrib = gaussians_distrib['vertices']
	hie = aims.Reader().read(options.hie)
	axes = Axes()
	scale = [10.] * 3
	for sulcus, gd in gaussians_distrib.items():
		m = aims.AimsSurfaceTriangle(axes)
		center = numpy.asarray(gd.mean()).ravel()
		motion_s = aims.Motion()
		motion_s.scale(scale, [1] * 3)
		motion_t = aims.Motion()
		motion_t.setTranslation(center)
		quat = aims.Quaternion()
		axis = numpy.random.uniform(0, 1, 3)
		angle = numpy.random.uniform(0, numpy.pi, 1)
		quat.fromAxis(axis, angle)
		motion_r = aims.Motion(quat)
		motion = motion_t * motion_r * motion_s
		aims.SurfaceManip.meshTransform(m, motion)
		color = hie.find_color(sulcus)
		go =  {'diffuse' : list(color) + [1.]}
		m.header()['material'] = go
		aims.Writer().write(m, '%s.mesh' % (sulcus))


if __name__ == '__main__' : main()
