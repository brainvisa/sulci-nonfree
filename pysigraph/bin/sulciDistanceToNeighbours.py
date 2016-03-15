#!/usr/bin/env python2

import os, sys, re
from soma import aims, aimsalgo
import numpy, optparse, tempfile, subprocess
import sigraph


# --- config variables

# --- args
parser = optparse.OptionParser( 
  description='calculate sulcuswise distance to other neighbouring sulci' )
parser.add_option( '-i', '--input', dest='graph', help='labelled sulci graph' )
parser.add_option( '-o', '--output', dest='csvfile', help='output CSV curvature stats file' )
parser.add_option( '-l', '--labelatt', dest='label', 
  help='label attribute (label or name), default: guessed if specified in ' \
  'graph, or take label' )
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


(options, args) = parser.parse_args()

graphfile = options.graph
csvfile = options.csvfile
labelatt = options.label
subject = options.subject
labelsfilter = options.labelsfilter
translation = options.translation
modeltranslation = options.modeltranslation

# ---

def distanceToNeighbours( vertex, label ):
  meandist = 0.
  ndist = 0
  mindist = 0.
  for e in vertex.edges():
    if e.vertices()[0] == vertex:
      other = e.vertices()[1]
    else:
      other = e.vertices()[0]
    if not other.has_key( labelatt ):
      continue
    olabel = other[ labelatt ]
    if olabel == label:
      continue
    if e.getSyntax() == 'cortical':
      dist = e[ 'dist' ]
      if dist != 0:
        meandist += dist
	if ndist == 0:
	  mindist = dist
	elif dist < mindist:
	  mindist = dist
	ndist += 1
  #if mindist == 0.:
    #print 'null min, ndist:', ndist
    #if meandist != 0.:
      #print 'null min, mean:', mean, 'ndist:', ndist
  return mindist, meandist, ndist


r = aims.Reader( options={ 'subobjectsfilter' : 0 } )
graph = r.read( graphfile )
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
labelsfilter = re.compile( labelsfilter )
diststats = {}

for v in graph.vertices():
  if v.has_key( labelatt ):
    label = v[ labelatt ]
    print label
    if labelsfilter.match( label ):
      mindist, meandist, ndist = distanceToNeighbours( v, label )
      stats = diststats.get( label, None )
      if stats is None:
        stats = { 'mindist' : 0., 'meandist' : 0., 'ndist' : 0 }
	diststats[ label ] = stats
    stats[ 'meandist' ] += meandist
    if ndist != 0:
      if stats[ 'ndist' ] == 0:
	stats[ 'mindist' ] = mindist
      elif mindist < stats[ 'mindist' ]:
	stats[ 'mindist' ] = mindist
    stats[ 'ndist' ] += ndist

csv = open( csvfile,'w' )
if subject:
  csv.write( 'subject ' )
csv.write( 'label  side  mindistance  meandistance  ndist\n' )
for label, stats in diststats.iteritems():
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
  ndist = stats[ 'ndist' ]
  meandist = stats[ 'meandist' ]
  if ndist != 0:
    meandist /= ndist
  print >> csv, ulabel, side, stats[ 'mindist' ], meandist, ndist
