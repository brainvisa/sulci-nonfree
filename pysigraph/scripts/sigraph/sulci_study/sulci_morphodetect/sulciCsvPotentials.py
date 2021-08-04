#!/usr/bin/env python

from __future__ import print_function
from __future__ import absolute_import
import os, sys
import six

def csvPotentials( stats, outpotfile, stds ):
  f = open( outpotfile, 'w' )
  f.write( 'subject side label potential normpotential mean_pot stddev_pot numobs_pot\n' )
  for sfile, pots in six.iteritems(stats):
    side = 'none'
    subject = os.path.basename( sfile )
    if subject.endswith( '.arg' ):
      subject = subject[ :-4 ]
    if subject.startswith( 'L' ) or subject.startswith( 'R' ):
      if subject[0] == 'L':
        side = 'left'
      else:
        side = 'right'
      subject = subject[ 1: ]
    if subject.endswith( '_auto' ):
      subject = subject[ :-5 ]
    elif subject.endswith( '_manual' ):
      subject = subject[ :-7 ]
    if subject.endswith( '_default_session' ):
      subject = subject[ :-16 ]
    for slabel, pot in six.iteritems(pots):
      label = slabel
      if label.endswith( '_left' ):
        side = 'left'
        label = label[ :-5 ]
      elif label.endswith( '_right' ):
        side = 'right'
        label = label[ :-6 ]
      cs = stds[ slabel ]
      if cs[ 'std' ] == 0:
        npot = 0.
      else:
        npot = ( pot - cs[ 'mean' ] ) / cs[ 'std' ]
      f.write( '%s %s %s %f %f %f %f %d\n' % ( subject, side, label, pot, npot, cs[ 'mean' ], cs[ 'std' ], cs[ 'number' ] ) )
  f.close()


if __name__ == '__main__':
  if sys.argv[1] in ( '-h', '--help' ):
    print('usage:')
    print(sys.argv[0], 'stats.txt outcsv.txt [altstats.txt]')
    print('Transform stats dictionary (given by sulciRecordStats) into a CSV potentials file with the following columns columns: subject, side, sulcus, potential, centered/scaled potential, mean pot, stdev pot, num pot recordings')
    print('if altstats.txt is provided, means/stdev are taken from this alternate source')
    sys.exit(0)

  statsfile = sys.argv[1]
  outcsv = sys.argv[2]
  exec(compile(open( statsfile, "rb" ).read(), statsfile, 'exec'))
  if len( sys.argv) > 3:
    altstats = sys.argv[3]
    x = {}
    exec(compile(open( altstats, "rb" ).read(), altstats, 'exec'), x, x)
    totalstats = x[ 'totalstats' ]
  csvPotentials( subjectspotentials, outcsv, totalstats )

