#!/usr/bin/env python2
# -*- coding: utf-8 -*-

from __future__ import print_function
import optparse
from soma import aims
import sys

# --- args
parser = optparse.OptionParser(
    description='Consistency check for sulci graph labels: typically, ensure there is no hemisphere mismatch (left label in right hemisphere).' )
parser.add_option( '-i', '--input', dest='graph', help='labelled sulci graph' )
parser.add_option( '-l', '--labelatt', dest='label', default=None,
    help='label attribute (label or name), default: guessed if specified in ' \
    'graph, or take name' )
parser.add_option( '-s', '--side', dest='side', default=None,
    help='side (left or right)' )

(options, args) = parser.parse_args()

graphfile = options.graph
labelatt = options.label
side = options.side

if not graphfile:
    parser.print_help()
    sys.exit( 2 )

graph = aims.read( graphfile )

if not labelatt:
    try:
        print('checking label_property')
        labelatt = graph[ 'label_property' ]
        print('done')
        print(labelatt)
    except:
        pass
    if not labelatt:
        print('Could not guess label attribute.', file=sys.stderr)
        sys.exit( 2 )

leftlabels = set()
rightlabels = set()

for v in graph.vertices():
    label = v[ labelatt ]
    if label.endswith( '_left' ):
        leftlabels.add( label )
    elif label.endswith( '_right' ):
        rightlabels.add( label )

print('left labels:', len( leftlabels ))
print('right labels:', len( rightlabels ))

if side is not None:
    if side == 'left':
        if len( rightlabels ) != 0:
            print(
                '** inconsistency: %d right labels present **' \
                % len( rightlabels ), file=sys.stderr)
            sys.exit( 1 )
        else:
            print('No inconsistency found.')
            sys.exit( 0 )
    elif side == 'right':
        if len( leftlabels ) != 0:
            print(
                '** inconsistency: %d left labels present **' \
                % len( leftlabels ), file=sys.stderr)
            sys.exit( 1 )
        else:
            print('No inconsistency found.')
            sys.exit( 0 )
    else:
        print'cannot understand side parameter - ignoring', file=sys.stderr)

if len( leftlabels ) != 0 and len( rightlabels ) != 0:
    print(
        '** inconsistency: %d left labels and %d right labels present **' \
        % ( len( leftlabels ), len( rightlabels ) ), file=sys.stderr)
    sys.exit( 1 )
else:
    print('No inconsistency found.')

