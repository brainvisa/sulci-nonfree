#!/usr/bin/env python2

from __future__ import absolute_import
from soma import aims
import sigraph
import sys, os, optparse
import sip

parser = optparse.OptionParser( description='Replace the CliqueDescriptor ' \
  'in a model graph to another one' )
parser.add_option( '-i', '--input', dest='mgraph', help='input model graph' )
parser.add_option( '-o', '--output', dest='outmgraph',
  help='output model graph' )
parser.add_option( '-n', '--nodemodel', dest='nodemodel',
  help='model element to set into the model graph nodes' )
parser.add_option( '-r', '--relmodel', dest='relmodel',
  help='model element to set into the model graph nodes' )

options, args = parser.parse_args( sys.argv )
mgfile = options.mgraph
omgfile = options.outmgraph
nmodfile = options.nodemodel
rmodfile = options.relmodel

mgraph = aims.read( mgfile )
if nmodfile:
  nmodel = sigraph.MReader( nmodfile ).readModel()
if rmodfile:
  rmodel = sigraph.MReader( rmodfile ).readModel()

if nmodfile:
  for vertex in mgraph.vertices():
    if 'model' in vertex:
      oldm = vertex[ 'model' ]
      otm = oldm.topModel()
      if otm is not None:
        m = nmodel.clone()
        sip.transferto( m, None )
        vertex[ 'model' ] = m
        sip.transferback( otm )
        sl = m.significantLabels()
        for l in list( sl ):
          sl.remove( l )
        for l in otm.significantLabels():
          sl.add( l )
        m.setVoidLabel( 'unknown' )
        label = vertex[ 'label' ]
        m.setBaseName( label )
  del vertex, m, otm, oldm

if rmodfile:
  voidl = 'unknown'
  if 'void_label' in mgraph:
    voidl = mgraph[ 'void_label' ]

  for edge in mgraph.edges():
    if 'model' in edge:
      oldm = edge[ 'model' ]
      otm = oldm.topModel()
      if otm is not None:
        m = rmodel.clone()
        sip.transferto( m, None )
        edge[ 'model' ] = m
        sip.transferback( otm )
        sl = m.significantLabels()
        for l in list( sl ):
          sl.remove( l )
        for l in otm.significantLabels():
          sl.add( l )
        m.setVoidLabel( 'unknown' )
        label1 = edge[ 'label1' ]
        label2 = edge[ 'label2' ]
        m.setBaseName( label1 + '-' + label2 )
        m.setVoidLabel( voidl )

aims.write( mgraph, omgfile )

