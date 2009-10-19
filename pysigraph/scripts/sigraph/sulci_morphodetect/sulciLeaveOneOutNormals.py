#!/usr/bin/env python

import sys,os, math

if sys.argv[1] in ( '-h', '--help' ):
  print 'usage:'
  print sys.argv[0], 'statsfile.txt'
  print 'Checks the value for each sulcus / std deviation, in a leqve-one-out way.'
  print 'statsfile.txt must be computed using sulciRecordStats.py'
  print 'The output is the ratio score for each subject/sulcus. No filtering/thresholding is applied.'
  print 'Output goes to the standard output and can be redirected to a file'
  sys.exit(0)

statsfile = sys.argv[1]

execfile( statsfile )

for s,stats in subjectspotentials.iteritems():
  for label,pot in stats.iteritems():
    st = totalstats.get( label )
    if st and st.has_key( 'std' ) and st[ 'number' ] >= 2:
      # remove this subject from stats
      sum = st[ 'sum' ] - pot
      sqsum = st[ 'squaresum' ] - pot*pot
      num = st[ 'number' ] -1
      mean = sum / num;
      std = math.sqrt( sqsum / ( num - 1 ) - mean * mean )
      if std != 0:
        score = ( pot - mean ) / std
        print os.path.basename(s) + '\t' + label + '\t' + str( score )