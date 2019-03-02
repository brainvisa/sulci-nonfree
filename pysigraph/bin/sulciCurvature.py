#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function
import os, sys, re
from soma import aims, aimsalgo
import numpy, optparse, tempfile, soma.subprocess
import sigraph
import time, tempfile, errno


# --- config variables
bottommindist = 2. # mm
hullmindist = 1.
juncmindist = 1.
inhibitmindist = 2.
meshatt = 'aims_Tmtktri'
bottomatt = 'aims_bottom'
hjatt = 'aims_junction'
ssatt = 'aims_ss'
ppatt = 'aims_plidepassage'
otheratt = 'aims_other'
nullvalue = 0.

# --- args
parser = optparse.OptionParser(
  description='calculate sulcuswise curvature maps and averages by sulcus' )
parser.add_option( '-i', '--input', dest='graph', help='labelled sulci graph' )
parser.add_option( '-o', '--output', dest='csvfile', help='output CSV curvature stats file' )
parser.add_option( '-m', '--meshdir', dest='meshdir',
  help='output directory for meshes and curvature textures ' \
  '(default: don\'t write them)' )
parser.add_option( '-c', '--curv', dest='curv', default='fem',
  help='curvature calculation method: fem, barycenter, boix, boixgaussian' )
parser.add_option( '-l', '--labelatt', dest='label',
  help='label attribute (label or name), default: guessed if specified in ' \
  'graph, or take label' )
parser.add_option( '-n', '--nodewise', dest='nodewise', action='store_true',
  help='perform graph nodewise calculations, and possibly output per-node ' \
  'meshes and curvatures if -m option is also set' )
parser.add_option( '-s', '--subject', dest='subject',
  help='subject name to write in CSV output. If not specified it will not ' \
  'be written' )
parser.add_option( '-f', '--filter', dest='labelsfilter', default='.*',
  help='labels filter pattern (regular expression). Normally labels are not ' \
  'filtered, all sulci are processed.' )
parser.add_option( '-t', '--translation', dest='translation',
  help='labels translation: may be a .trl translation map, a selection ' \
  '(.sel), or a nomenclature (.hie)' )
parser.add_option( '--modeltrans', dest='modeltranslation',
  help='model graph used for labels translation, used in combination of the ' \
  'translation file, especially if the latter is a nomenclature. Wen using a' \
  'selection, the model graph sould not be specified, and when using a .trl ' \
  'translation, it is generally not so useful.' )
parser.add_option( '-a', '--append', dest='append', action='store_true',
  help='append output to a possibly existing CSV file. The output file will ' \
  'be locked using a <output>.lock directory to avoid concurrent access by ' \
  'multiple instances of sulciCurvature, and the CSV header will be ' \
  'written only if the CSV file does not exist or is empty.' )


(options, args) = parser.parse_args()

graphfile = options.graph
csvfile = options.csvfile
curvmethod = options.curv
labelatt = options.label
meshdir = options.meshdir
nodewise = options.nodewise
subject = options.subject
labelsfilter = options.labelsfilter
translation = options.translation
modeltranslation = options.modeltranslation
appendmode = options.append

if not graphfile:
  parser.parse_args( [ '-h' ] )


def makearrayfrombucket( bucket ):
  pts = bucket[0].keys()
  vs = numpy.array( [ bucket.sizeX(), bucket.sizeY(), bucket.sizeZ() ] )
  arr = numpy.zeros( ( len( pts ), 3 ) )
  i = 0
  for p in pts:
    arr[ i ] = p
    i += 1
  arr *= vs
  return arr

def distfrompoint( coords, point ):
  crel = coords - point
  d = numpy.sum( crel * crel, axis=1 )
  return d

def cleanmeshcurv( mesh, boundaries, inhibitbounds=[] ):
  tf = tempfile.mkstemp( 'mesh.gii', 'sulcus' )
  os.close( tf[0] )
  tmpmesh = tf[1]
  tf = tempfile.mkstemp( 'curv.gii', 'sulcus' )
  os.close( tf[0] )
  tmpcurv = tf[1]
  aims.write( mesh, tmpmesh )
  tf = tempfile.mkstemp( 'stdout', 'aims' )
  soma.subprocess.call( [ 'AimsMeshCurvature', '-i', tmpmesh, '-o', tmpcurv, '-m',
    curvmethod ], stdout=tf[0] )
  os.close( tf[0] )
  os.unlink( tf[1] )
  tex = aims.read( tmpcurv )
  os.unlink( tmpmesh )
  os.unlink( tmpmesh + '.minf' )
  os.unlink( tmpcurv )
  os.unlink( tmpcurv + '.minf' )
  texarr = tex[0].arraydata()
  vindex = 0
  average = 0.
  nav = 0
  for p in mesh.vertex():
    # test for "hard" boundaries buckets
    reject = False
    for b, mindist in boundaries[0]:
      d = distfrompoint( b, p )
      if len( numpy.where( d < mindist )[0] ) != 0:
        texarr[ vindex ] = nullvalue
        reject = True
        break
    if not reject and len( boundaries[2] ) != 0:
      inhibit = False
      # test if inhibitor buckets around can prevent further cleaning
      for b, mindist in boundaries[1]:
        d = distfrompoint( b, p )
        if len( numpy.where( d < mindist )[0] ) != 0:
          inhibit = True
          break
      if not inhibit:
        # test remaining inhibitable buckets
        for b, mindist in boundaries[2]:
          d = distfrompoint( b, p )
          if len( numpy.where( d < mindist )[0] ) != 0:
            texarr[ vindex ] = nullvalue
            reject = True
            break
    if not reject:
      average += abs( texarr[ vindex ] )
      nav += 1
    vindex += 1
  return average, nav, tex

# --- main

graph = aims.read( graphfile )

if labelatt is None:
  if graph.has_key( 'label_property' ):
    labelatt = graph[ 'label_property' ]
  else:
    labelatt = 'label'

if translation is not None:
  if modeltranslation is not None:
    model = aims.read( modeltranslation )
    ft = sigraph.FoldLabelsTranslator( model, translation )
  else:
    ft = sigraph.FoldLabelsTranslator( translation )
  ft.translate( graph, labelatt, labelatt )

node = 0
globaverage = {}
labelsfilter = re.compile( labelsfilter )
for v in graph.vertices():
  if v.has_key( labelatt ):
    label = v[ labelatt ]
    if v.has_key( meshatt ) and labelsfilter.match( label ):
      mesh = v[ meshatt ]
      boundaries = [ ( makearrayfrombucket( v[ bottomatt ] ), bottommindist ) ]
      bck = v[ bottomatt ]
      pernodebounds = []
      inhibitbounds = []
      inhibitablebounds = []
      gav = globaverage.get( label, None )
      if gav is None:
        gav = { 'average' : 0., 'nav' : 0, 'boundaries' : [ [], [], [] ],
          'ss' : [] }
        globaverage[ label ] = gav
      gav[ 'ss' ] += [ v[ ssatt ], v[ otheratt ] ]
      # find hull_junction
      for e in v.edges():
        if e.getSyntax() == 'hull_junction':
          boundaries.append( ( makearrayfrombucket( e[ hjatt ] ),
            hullmindist ) )
          bck.merge( e[ hjatt ] )
        elif e.getSyntax() in ( 'junction', 'plidepassage' ):
          if e.getSyntax() == 'junction':
            bk = e[ hjatt ]
          else:
            bk = e[ ppatt ]
          bbk = makearrayfrombucket( bk )
          if e.vertices()[0][labelatt] == e.vertices()[1][labelatt]:
            # don't remove points near intra-sulcus junctions
            pernodebounds.append( ( bbk, juncmindist ) )
            # and prevent other junctions around to remove them
            inhibitbounds.append( ( bbk, inhibitmindist ) )
          else:
            inhibitablebounds.append( ( bbk, juncmindist ) )
          bck.merge( bk )
        #elif e.getSyntax() == 'plidepassage':
          #if e.has_key( ppatt ):
            #gav[ 'ss' ].append( e[ ppatt ] )
          #else: print('pli de passage without bucket')
      gav[ 'ss' ].append( bck )
      gav[ 'boundaries' ][0] += boundaries
      gav[ 'boundaries' ][1] += inhibitbounds
      gav[ 'boundaries' ][2] += inhibitablebounds
      if nodewise:
        average, nav, tex = cleanmeshcurv( mesh,
          ( boundaries + pernodebounds, [], [] ) )
        if nav != 0:
          gav[ 'average' ] += average
          gav[ 'nav' ] += nav
          average /= nav
	if meshdir:
          mfile = os.path.join( meshdir,
	    'node_' + label + '_' + str( node ) + '.gii' )
          tfile = os.path.join( meshdir,
	    'node_' + label + '_curv_' + str( node ) + '.gii' )
          mesh.header().update( { 'texture_filenames' : \
	    [ os.path.basename( tfile ) ] } )
          aims.write( mesh, mfile )
          aims.write( tex, tfile )
          aims.write( bck, os.path.join( meshdir,
	    'node_' + label + '_bck_' + str( node ) + '.bck' ) )
      node += 1

if csvfile:
  if appendmode:
    globalcsvfile = csvfile
    x = tempfile.mkstemp()
    os.close( x[0] )
    csvfile = x[1]
  csv = open( csvfile, 'w' )
  if subject:
    csv.write( 'subject ' )
  csv.write( 'label side mean_curvature mean_curvature_points' )
  if nodewise:
    csv.write( ' mean_curvature_nodewise mean_curvature_points_nodewise' )
  csv.write( '\n' )
for label, globval in globaverage.iteritems():
  bcks = globval[ 'ss' ]
  bck = bcks[0]
  for b in bcks[1:]:
    bck.merge( b )
  vol = aims.Converter_BucketMap_VOID_AimsData_S16()( bck )
  tf = tempfile.mkstemp( 'vol.nii', 'sulcus' )
  os.close( tf[0] )
  tmpvolfile = tf[1]
  aims.write( vol, tmpvolfile )
  tf = tempfile.mkstemp( 'stdout', 'aims' )
  soma.subprocess.call( [ 'AimsConnectComp', '-i', tmpvolfile, '-o', tmpvolfile,
    '-c', '18', '-n', '1', '-b' ], stdout=tf[0] )
  os.close( tf[0] )
  os.unlink( tf[1] )
  vol = aims.read( tmpvolfile, 1 )
  os.unlink( tmpvolfile )
  os.unlink( tmpvolfile + '.minf' )
  aims.AimsData_S16( vol ).fillBorder( -1 )
  # vol2 = aims.Volume_S16( vol.dimX()+2, vol.dimY()+2, vol.dimZ()+2 )
  # vol2.fill( -1 )
  # h = {}
  # h.update( vol.header() )
  # del h[ 'sizeX' ]
  # del h[ 'sizeY' ]
  # del h[ 'sizeZ' ]
  # del h[ 'sizeT' ]
  # vol2.header().update( h )
  # vol2.header()[ '_borderWidth' ] = 1
  # numpy.array( vol2, copy=False )[ 1:-1, 1:-1, 1:-1,: ] \
  #   = numpy.array( vol.volume(), copy=False )
  m = aimsalgo.Mesher()
  m.setDecimation( 100, 5, 3, 180 )
  m.setMinFacetNumber( 50 )
  m.setVerbose( False )
  mesh = aims.AimsSurfaceTriangle()
  m.getBrain( vol, mesh )
  average, nav, tex = cleanmeshcurv( mesh, globval[ 'boundaries' ] )
  if meshdir:
    mfile = os.path.join( meshdir, 'sulcus_' + label + '.gii' )
    tfile = os.path.join( meshdir, 'sulcus_' + label + '_curv.gii' )
    mesh.header().update( { 'texture_filenames' : [ os.path.basename( tfile ) ] } )
    aims.write( mesh, mfile )
    aims.write( tex, tfile )
  if nav != 0:
    average /= nav
  print(label, 'global average:', average)
  if nodewise:
    if globval[ 'nav' ] != 0:
      globval[ 'average' ] /= globval[ 'nav' ]
    print(label, 'nodewise average:', globval[ 'average' ])
  if csvfile:
    if label.endswith( '_left' ):
      side = 'left'
      ulabel = label[ : len(label)-5 ]
    elif label.endswith( '_right' ):
      side = 'right'
      ulabel = label[ : len(label)-6 ]
    else:
      side = 'both'
      ulabel = label
    if subject:
      csv.write( subject + ' ' )
    csv.write( ulabel + ' ' + side + ' ' + str( average ) + ' ' + str( nav ) )
    if nodewise:
      csv.write( ' ' + str( globval[ 'average' ] ) + ' ' + str( globval[ 'nav' ] ) )
    csv.write( '\n' )

if csvfile and appendmode:
  # lock the output CSV
  success = False
  lockdir = os.path.join( globalcsvfile + '.lock' )
  while not success:
    try:
      os.mkdir( lockdir )
      success = True
    except OSError as e:
      if e.errno != errno.EEXIST:
        raise
      time.sleep( 0.5 ) # wait a little bit

  csv = open( globalcsvfile, 'a' )
  csvin = open( csvfile )
  if csv.tell() != 0:
    # copy header only if at start of file
    l = csvin.readline()
    l[:-1] # force using iterator
  csv.write( csvin.read() )
  csv.close()
  csvin.close()
  os.unlink( csvfile )
  # unlock the global CSV
  os.rmdir( lockdir )

