#!/usr/bin/env python

import os, sys
from soma import aims, aimsalgo
import numpy
import optparse
import sigraph
import tempfile, subprocess

parser = optparse.OptionParser( description='calculates a distance map from ' \
  'a lesion in the brain, and mean distance from the lesion to all sulci. ' \
  'If the distance map is already calcualted, it can be specified as input ' \
  'using the --indistance parameter, instead of the lesionfile and brainfile.' )
parser.add_option( '-l', '--lesion', dest='lesionfile',
  help='input lesion mask' )
parser.add_option( '-L', '--lgreywhite', dest='lgw',
  help='left grey/white mask' )
parser.add_option( '-R', '--rgreywhite', dest='rgw',
  help='right grey/white mask' )
parser.add_option( '--lsk', dest='lsk',
  help='left sulci skeleton (optional) used to avoid crossing sulci' )
parser.add_option( '--rsk', dest='rsk',
  help='right sulci skeleton (optional) used to avoid crossing sulci' )
parser.add_option( '-o', '--output', dest='outputcsv',
  help='output CSV file' )
parser.add_option( '-g', '--graph', dest='graphs', action='append',
  default=[],
  help='sulci graph files, several can be specified (generally left and ' \
  'right hemispheres)' )
parser.add_option( '--indistance', dest='indistfile',
  help='input distance map image, recalculated if not specified' )
parser.add_option( '-d', '--distance', dest='distfile',
  help='output distance map image, not written if not specified' )
parser.add_option( '--labelatt', dest='label',
  help='label attribute (label or name), default: guessed if specified in ' \
  'graphs, or take label' )
parser.add_option( '-s', '--subject', dest='subject',
  help='subject name to write in CSV output. If not specified it will not be ' \
  'written' )
parser.add_option( '-t', '--translation', dest='translation',
  help='labels translation: may be a .trl translation map, a selection ' \
  '(.sel), or a nomenclature (.hie)' )
parser.add_option( '--modeltrans', dest='modeltranslation',
  help='model graph used for labels translation, used in combination of the ' \
  'translation file, especially if the latter is a nomenclature. Wen using a' \
  'selection, the model graph sould not be specified, and when using a .trl ' \
  'translation, it is generally not so useful.' )

options, args = parser.parse_args()

lgwfile = options.lgw
rgwfile = options.rgw
lskfil = options.lsk
rskfil = options.rsk
lesionfile = options.lesionfile
outputcsv = options.outputcsv
graphfiles = options.graphs
indistfile = options.indistfile
distfile = options.distfile
labelatt = options.label
subject = options.subject
translation = options.translation
modeltranslation = options.modeltranslation

if ( not lgwfile or not rgwfile or not lesionfile ) and not indistfile:
  parser.parse_args( [ '-h' ] )

graphs = []
if indistfile:
  dist = aims.read( indistfile )
else:
  lgw = aims.read( lgwfile, 1 )
  rgw = aims.read( rgwfile, 1 )
  lesion = aims.read( lesionfile )

  for graphfile in graphfiles:
    graphs.append( aims.read( graphfile ) )

  wh = aims.AimsData_S16( lgw + rgw, 1 )
  dh = dict( lgw.header() )
  for x in [ 'volume_dimension', 'sizeX', 'sizeY', 'sizeZ', 'sizeT' ]:
    if dh.has_key( x ):
      del dh[x]
  wh.header().update( dh )
  wharr = numpy.array( wh.volume(), copy=False )
  wharr[ wharr == 100 ] = 0 # erase grey matter
  # close WM one voxel to link both hemispheres
  wh = aimsalgo.AimsMorphoClosing( wh, wh.header()['voxel_size'][0] )
  wharr = numpy.array( wh.volume(), copy=False )

  # erode GM a bit on each hemisphere so that they avoid touching from one
  # hemisphere to the other
  ler = aimsalgo.AimsMorphoErosion( lgw, 1 )
  rer = aimsalgo.AimsMorphoErosion( rgw, 1 )
  eroded = ler + rer
  del ler, rer
  barr = numpy.array( eroded.volume(), copy=False )
  # merge with connected WM
  barr[ numpy.where( wharr != 0 ) ] = 32767

  # mask lesion to allow only parts in the brain mask
  larr = numpy.array( lesion, copy=False )
  barr = barr[ 1:-1, 1:-1, 1:-1, : ] # remove border
  larr[ barr == 0 ] = 0
  # put the lesion inside the brain
  barr[ larr == 4095 ] = 1

  # avoid propagating in the sulci
  for graph in graphs:
    for v in graph.vertices():
      for bucket in ( 'aims_ss', 'aims_bottom', 'aims_other' ):
        try:
          bck = v[ bucket ]
          barr[ numpy.column_stack( [ bck[0].keys(),
            numpy.zeros( ( bck[0].size(), 1 ) ) ] ) ] = 0
        except:
          pass

  fm = aims.FastMarching()
  dist = fm.doit( eroded.volume(), [ 32767 ], [ 1 ] )

  # remove FLT_MAX values
  darr = numpy.array( dist, copy=False )
  darr[ numpy.where( darr > 1e4 ) ] = -1
  # set lesion to distance 0
  darr[ larr != 0 ] = 0

  tmpf = tempfile.mkstemp( suffix='.nii', prefix='aims_dist' )
  os.close( tmpf[0] )
  aims.write( dist, tmpf[1] )

  if distfile:
    outdist = distfile
  else:
    tmpf2 = tempfile.mkstemp( suffix='.nii', prefix='aims_closedist' )
    outdist = tmpf2[1]
    os.close( outdist[0] )

  print 'closing the distance map...'
  subprocess.call( [ 'AimsMorphoMath', '-i', tmpf[1], '-o', outdist, '-r', '3',
    '-m', 'clo' ] )
  os.unlink( tmpf[1] )
  os.unlink( tmpf[1] + '.minf' )
  del darr, dist
  dist = aims.read( outdist )
  if not distfile:
    os.unlink( outdist )
    os.unlink( outdist + '.minf' )

darr = numpy.array( dist, copy=False )

if not graphfiles:
  sys.exit( 0 ) # finished

ft = None
if translation is not None:
  if modeltranslation is not None:
    model = aims.read( modeltranslation )
    ft = sigraph.FoldLabelsTranslator( model, translation )
  else:
    ft = sigraph.FoldLabelsTranslator( translation )

if outputcsv:
  csvfile = open( outputcsv, 'w' )
  if subject:
    print >> csvfile, 'subject',
  print >> csvfile, 'label side distance distance_points touching ' \
    'min_lesion_distance'
else:
  csvfile = None

for gnum, graphfile in enumerate( graphfiles ):
  distances = {}
  if len( graphs ) > gnum:
    graph = graphs[ gnum ]
  else:
    graph = aims.read( graphfile )
  labelatt2 = labelatt
  if labelatt is None:
    if graph.has_key( 'label_property' ):
      labelatt2 = graph[ 'label_property' ]
    else:
      labelatt2 = 'label'
  if ft is not None:
    ft.translate( graph, labelatt2, labelatt2 )

  for v in graph.vertices():
    try:
      label = v[ labelatt2 ]
    except:
      continue
    if label == 'unknown':
      continue
    dist = distances.get( label, None )
    if dist is None:
      dist = { 'dist' : 0., 'ndist' : 0, 'touching' : 0, 'mindist' : 0. }
      distances[ label ] = dist
    for bucket in ( 'aims_ss', 'aims_bottom', 'aims_other' ):
      try:
        bck = v[ bucket ]
        for voxel in bck[0].keys():
          d = darr[ voxel[0], voxel[1], voxel[2], 0 ]
          if d < 1e4 and d >= 0: # avoid disconnected zones where the distance
                      # map has not reached
            dist[ 'dist' ] += darr[ voxel[0], voxel[1], voxel[2], 0 ]
            if dist[ 'ndist' ] == 0:
              dist[ 'mindist' ] = d
            elif d < dist[ 'mindist' ]:
              dist[ 'mindist' ] = d
            dist[ 'ndist' ] += 1
            if d == 0:
              dist[ 'touching' ] = 1
      except:
        pass
  for label, dist in distances.iteritems():
    if dist[ 'ndist' ] > 0:
      dist[ 'dist' ] /= dist[ 'ndist' ]
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
        print >> csvfile, subject,
      print >> csvfile, ulabel, side, dist[ 'dist' ], dist[ 'ndist' ], \
        dist[ 'touching' ], dist[ 'mindist' ]


