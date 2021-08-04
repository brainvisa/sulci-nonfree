#!/usr/bin/env python

from __future__ import print_function
from __future__ import absolute_import
import sigraph, os, sys
from soma import aims
from optparse import OptionParser
from six.moves import zip

def printDescriptors( dg ):
  c = dg.cliques()
  sumpot = 0

  for cl in c:
    mod = mf.selectModel( cl )[ 'model' ]
    pot = mod.printDescription( cl, True )
    try:
      descr = cl[ 'pot_vector' ]
      dnames = cl[ 'descriptor_names' ]
    except:
      print('warning: clique with no pot_vector or no descriptor_names')
      descr = []
      dnames = []
    sumpot += pot
    if 'label' in cl:
      l = cl[ 'label' ]
      print('sulcus clique:', l, ', pot:', pot)
    elif 'label1' in cl and 'label2' in cl:
      l1 = cl[ 'label1' ]
      l2 = cl[ 'label2' ]
      print('relation clique:', (l1, l2), 'pot:', pot)
    print(list(zip( dnames, descr )))

  print('energy:', sumpot)


defaultmgraph = os.path.join( aims.carto.Paths.shfjShared(), 'models', '3.0',
  'Rfolds_noroots_020200', 'Rfolds_model.arg' )
parser = OptionParser( 'Print potentials and cliques descriptions of a ' \
  'labelled graph according to a model')
parser.add_option( '-m', '--model_graph', action='store',
  dest='mgraph', metavar = 'FILE', default = None,
  help='model graph file [default: ' + defaultmgraph + ']' )
parser.add_option( '-d', '--data_graph', action='store',
  dest='dgraph', metavar = 'FILE', default = None,
  help='data graph file' )

(options, args) = parser.parse_args( sys.argv )
if options.mgraph:
  modelgraph = options.mgraph
else:
  modelgraph = defaultmgraph
datagraph = options.dgraph

r = aims.Reader()
mg = r.read( modelgraph )
dg = r.read( datagraph )

mg.removeWeights()
mf = mg.modelFinder()
mf.initCliques( dg, False, False, True, False )

printDescriptors( dg )

