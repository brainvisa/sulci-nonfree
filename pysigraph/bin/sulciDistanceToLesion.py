#!/usr/bin/env python

import os, sys
from soma import aims, aimsalgo
import numpy
import optparse
import sigraph

parser = optparse.OptionParser( description='calculates a distance map from ' \
  'a lesion in the brain, and mean distance from the lesion to all sulci' )
parser.add_option( '-l', '--lesion', dest='lesionfile',
  help='input lesion mask' )
parser.add_option( '-b', '--brain', dest='brainfile',
  help='brain mask' )
parser.add_option( '-o', '--output', dest='outputcsv',
  help='output CSV file' )
parser.add_option( '-g', '--graph', dest='graphs', action='append',
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

brainfile = options.brainfile
lesionfile = options.lesionfile
outputcsv = options.outputcsv
graphfiles = options.graphs
indistfile = options.indistfile
distfile = options.distfile
labelatt = options.label
subject = options.subject
translation = options.translation
modeltranslation = options.modeltranslation

if not brainfile or not lesionfile:
  parser.parse_args( [ '-h' ] )

brain = aims.read( brainfile )
barr = numpy.array( brain, copy=False )

if indistfile:
  dist = aims.read( indistfile )
else:
  lesion = aims.read( lesionfile )

  # mask lesion to allow only parts in the brain mask
  larr = numpy.array( lesion, copy=False )
  larr[ barr != 255 ] = 0
  # put the lesion inside the brain
  barr[ larr == 4095 ] = 1

  fm = aims.FastMarching()
  dist = fm.doit( brain, [ 255 ], [ 1 ] )

  if distfile:
    aims.write( dist, distfile )
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
  print >> csvfile, 'label side distance distance_points touching'
else:
  csvfile = None

for graphfile in graphfiles:
  distances = {}
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
      dist = { 'dist' : 0., 'ndist' : 0, 'touching' : 0 }
      distances[ label ] = dist
    for bucket in ( 'aims_ss', 'aims_bottom', 'aims_other' ):
      try:
        bck = v[ bucket ]
        for voxel in bck[0].keys():
          if barr[ voxel[0], voxel[1], voxel[2], 0 ] != 0:
            d = darr[ voxel[0], voxel[1], voxel[2], 0 ]
            if d < 1e4: # avoid disconnected zones where the distance map
                        # has not reached
              dist[ 'dist' ] += darr[ voxel[0], voxel[1], voxel[2], 0 ]
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
        dist[ 'touching' ]


