#!/usr/bin/env python

from __future__ import print_function
from __future__ import absolute_import
from soma import aims
import sigraph
import math, sys

def sulciRecordStats( inputs, modelfile, output ):
  model = aims.read( modelfile )

  nodecliquestats = {}
  relcliquestats = {}
  totalstats = {}
  subjectspotentials = {}

  def stddev( mean, squaresum, n ):
    return math.sqrt( squaresum / ( n - 1 ) - mean * mean )

  for subj in inputs:
    print(subj)
    gr = aims.read( subj )
    mf = model.modelFinder()
    mf.initCliques( gr, True, False, True )
    del mf
    an = sigraph.Anneal( gr, model )
    p = an.processAllPotentials()
    del an
    cl = gr.cliques()
    totstats_persubject = {}
    for c in cl:
      if 'label' in c:
        label = c[ 'label' ]
        cs = nodecliquestats.get( label )
        if cs is None:
          cs = { 'number' : 0, 'sum' : 0., 'squaresum' : 0. }
          nodecliquestats[ label ] = cs
        cs[ 'number' ] += 1
        p = c[ 'potential' ]
        cs[ 'sum' ] += p
        cs[ 'squaresum' ] += p * p
        ts = totstats_persubject.get( label )
        if ts is None:
          totstats_persubject[ label ] = p
        else:
          totstats_persubject[ label ] += p
      elif 'label1' in c:
        l = [ c[ 'label1' ], c['label2'] ]
        l.sort()
        l = tuple( l )
        label1 = l[0]
        label2 = l[1]
        cs = relcliquestats.get( l )
        if cs is None:
          cs = { 'number' : 0, 'sum' : 0., 'squaresum' : 0. }
          relcliquestats[ l ] = cs
        cs[ 'number' ] += 1
        p = c[ 'potential' ]
        cs[ 'sum' ] += p
        cs[ 'squaresum' ] += p * p
        ts = totstats_persubject.get( label1 )
        if ts is None:
          totstats_persubject[ label1 ] = p
        else:
          totstats_persubject[ label1 ] += p
        ts = totstats_persubject.get( label2 )
        if ts is None:
          totstats_persubject[ label2 ] = p
        else:
          totstats_persubject[ label2 ] += p
    for label, p in totstats_persubject.items():
      cs = totalstats.get( label )
      if cs is None:
        cs = { 'number' : 0, 'sum' : 0., 'squaresum': 0. }
        totalstats[ label ] = cs
      cs[ 'number' ] += 1
      cs[ 'sum' ] += p
      cs[ 'squaresum' ] += p * p
    del c
    del cl
    subjectspotentials[ subj ] = totstats_persubject

  #print('nodes:', nodecliquestats)
  #print('rels:', relcliquestats)
  #print('total:', totalstats)

  print('computing means/std')
  import sys
  sys.stdout.flush()
  for label, cl in nodecliquestats.items():
    cl[ 'mean' ] = cl[ 'sum' ] / cl[ 'number' ]
    if cl[ 'number' ] > 1:
      cl[ 'std' ] = stddev( cl[ 'mean' ], cl[ 'squaresum' ], cl[ 'number' ] )
  for label, cl in relcliquestats.items():
    cl[ 'mean' ] = cl[ 'sum' ] / cl[ 'number' ]
    if cl[ 'number' ] > 1:
      cl[ 'std' ] = stddev( cl[ 'mean' ], cl[ 'squaresum' ], cl[ 'number' ] )
  for label, cl in totalstats.items():
    cl[ 'mean' ] = cl[ 'sum' ] / cl[ 'number' ]
    if cl[ 'number' ] > 1:
      cl[ 'std' ] = stddev( cl[ 'mean' ], cl[ 'squaresum' ], cl[ 'number' ] )

  w = open( output, 'w' )
  w.write( 'nodecliquestats = ' + repr( nodecliquestats) + '\n' )
  w.write( 'relcliquestats = ' + repr( relcliquestats) + '\n' )
  w.write( 'totalstats = ' + repr( totalstats) + '\n' )
  w.write( 'subjectspotentials = ' + repr( subjectspotentials ) + '\n' )

if __name__ == '__main__':
  if sys.argv[1] in ( '-h', '--help' ):
    print('usage:')
    print(sys.argv[0], 'modelfile.arg outputstats.txt subject01.arg [subject02.arg ...]')
    print('Record stats about sulci potentials on a group of subjects.')
    print('The output file is a python dictionary containing sums, means and std deviation on the sulci.')
    print('It also contains per-subject potentials that can be used later for tests: see sulciTestStats.py and sulciCheckBase.py.')
    sys.exit(0)

  modelfile = sys.argv[1]
  output = sys.argv[2]
  inputs = sys.argv[3:]
  sulciRecordStats( inputs, modelfile, output )

