#!/usr/bin/env python

import os, sys

if sys.argv[1] in ( '-h', '--help' ):
  print 'usage:'
  print sys.argv[0], 'outputfile.txt labelledsulci.arg [labelledsulci2.arg ...]'
  print 
  sys.exit(0)

from soma import aims
import math

subjectsfiles = sys.argv[2:]
outputfile = sys.argv[1]

sum = 0.
num = 0
sqsum = 0.
out = open( outputfile, 'w' )

for s in subjectsfiles:
  sz = 0.
  gr = aims.read( s )
  for v in gr.vertices():
    if v.has_key( 'label' ):
      label = v[ 'label' ]
      if label == 'unknown':
        if v.has_key( 'size' ):
          sz += v[ 'size' ]
  del v
  sum += sz
  sqsum += sz * sz
  num += 1
  print >> out, os.path.basename(s) + '\t' + str( sz )

if num != 0:
  mean = sum / num
  if num >= 2:
    std = math.sqrt( sqsum / ( num - 1 ) - mean * mean )
    print >> out, 'mean_size:\t' + str(mean) + '\tstd:\t' + str(std)

