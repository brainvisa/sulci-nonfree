#!/usr/bin/env python2

import os, sys
import numpy
from soma import aims, aimsalgo
from soma.wip.aimsalgo import foldsgraphthickness as FG

def main():
	# filenames
	graphname = '/home/perrot/ccrt/base2008/graphs/Rsujet12_man.arg'
	skelname = '/home/perrot/ccrt/base2008/skeleton/Rskeleton_sujet12.ima'
	white_mesh_name = '/home/perrot/ccrt/base2008/mesh_white/sujet12_Rwhite.mesh'
	hemi_mesh_name = '/home/perrot/ccrt/base2008/mesh_hemi/sujet12_Rhemi.mesh'

	# read
	reader = aims.Reader()
	graph = reader.read(graphname)
	skel = reader.read(skelname)
	white_mesh = reader.read(white_mesh_name)
	hemi_mesh = reader.read(hemi_mesh_name)

	askel = skel.arraydata()
	outside_label = askel[0, 0, 0, 0]
	askel[askel != outside_label] = 100   # Grey Matter label
	askel[askel == outside_label] = 0     # outside label

	# process
	fd = FG.FoldsGraphThickness(graph, skel, white_mesh, hemi_mesh)
	# skip some parts
	try:	fd.preProcess()
	except Exception, e: pass
	ttex = fd.gm_wm_tex
	tex = ttex[0]
	ind_max = 1
	h = {}
	indices = []
	for i, x in enumerate(tex):
		v = tex[i]
		if h.has_key(v):
			ind = h[v]
		else:
			h[v] = ind = ind_max
			indices.append(v)
			ind_max += 1
		tex[i] = ind
	vol2 = aims.AimsData_S16(fd.voronoi_vol)
	avol = fd.voronoi_vol.arraydata()
	avol2 = vol2.volume().arraydata()
	for i, ind in enumerate(indices):
		avol2[avol == ind] = i
	print "indices = ", indices
	aims.Writer().write(ttex, 'plop.tex')
	aims.Writer().write(vol2, 'voronoi_vol.ima')

if __name__ == '__main__': main()
