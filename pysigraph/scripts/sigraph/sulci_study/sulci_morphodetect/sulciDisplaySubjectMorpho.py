#!/usr/bin/env python2

from __future__ import print_function
import anatomist.direct.api as ana
from soma import aims
import os, sys,numpy

def loadMorpho( wdir ):
  morpho = {}
  files = os.listdir( wdir )
  files.sort()
  for fn in files:
    if fn.endswith( '.dat' ):
      fn = os.path.join( wdir, fn )
      f = open( fn )
      hdr = f.readline().strip().split()
      l1 = f.readline().split()
      f.close()
      ncols = len( hdr )
      label = l1[1]
      side = l1[2]
      label += '_' + side
      n = numpy.loadtxt( fn, skiprows=1, usecols=range(3,ncols) ).reshape( ( -1, ncols-3 ) )
      subjects = numpy.loadtxt( fn, skiprows=1, usecols=[0], dtype=str ).reshape( ( n.shape[0], ) )
      morpho[ label ] = [ hdr[3:], n, subjects ]
  return morpho

def loadstat( file ):
  f = open( file )
  hdr = f.readline().strip().split()
  f.close()
  ncols = len( hdr )
  mean = numpy.loadtxt( file, skiprows=1, usecols=range(2,ncols) )
  meant = {}
  f = open( file )
  f.readline()
  i = 0
  for l in f.xreadlines():
    l = l.strip().split()
    label = l[0]
    side = l[1]
    label += '_' + side
    meant[ label ] = mean[i, :]
    i += 1
  return meant

def mapProperty( graph, statprop, prop, subject, labelprop, morpho, means, std ):
  vert = graph.vertices()
  for v in vert:
    if v.has_key( labelprop ):
      label = v[ labelprop ]
      measures = morpho.get( label )
      if measures:
        col = measures[0].index( prop )
        line = numpy.where( measures[2] == subject )[0][0]
        meas = measures[1][ line ][col]
        s = stds[label][col]
        if s == 0:
          meas = 0
        else:
          meas = (meas - means[label][col] ) / s
        v[ statprop ] = meas

if __name__ == '__main__':
  if sys.argv[1] in ( '-h', '--help' ):
    print('usage:')
    print(sys.argv[0], 'morphomean.dat morphostd.dat subject.arg morphodir property subjectname')
    print()
    sys.exit(0)

  morpho = loadMorpho( sys.argv[4] )
  means = loadstat( sys.argv[1] )
  stds = loadstat( sys.argv[2] )
  graph = aims.read( sys.argv[3] )
  prop = sys.argv[5]
  subject = sys.argv[6]
  mapProperty( graph, 'stat', prop, subject, 'label', morpho,
    means, stds )

a = ana.Anatomist()
ag = ana.cpp.AObjectConverter.anatomist( graph )
a.execute( 'GraphDisplayProperties', objects=[ag], display_mode='PropertyMap',
  display_property='stat' )
w = a.createWindow( '3D' )
a.execute( 'SetObjectPalette', objects=[ag], absoluteMode=1, min=-3., max=3. )
w.addObjects( [ag], add_graph_nodes=True )

from soma.qt_gui.qt_backend import QtGui
if QtGui.qApp.startingUp():
  QtGui.qApp.exec_()

