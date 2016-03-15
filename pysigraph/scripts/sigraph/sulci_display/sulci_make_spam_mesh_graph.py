#!/usr/bin/env python2

import os, sys, glob
from soma import aims

def makeSpamGraph( level, meshdir ):
  meshes = glob.glob( os.path.join( meshdir, '*_' + str(level) + '.mesh' ) )
  graph = aims.Graph( 'CorticalFoldArg' )
  graph[ 'voxel_size' ] = [ 1., 1., 1. ]
  graph[ 'boundingbox_min' ] = [ -90., -80., -90. ]
  graph[ 'boundingbox_max' ] = [ 90., 120., 60. ]
  graph[ 'filename_base' ] = '*'
  graph[ 'label_property' ] = 'name'
  i = 0
  side = None
  for mesh in meshes:
    label = os.path.basename( mesh )[5:-7]
    if label.endswith( 'left' ):
      side = 'L'
    elif label.endswith( 'right' ):
      side = 'R'
    m = aims.read( mesh )
    vert = graph.addVertex( 'fold' )
    vert[ 'point_number' ] = 0
    vert[ 'name' ] = label
    vert[ 'skeleton_label' ] = i
    vert[ 'size' ] = 0
    vert[ 'ss_point_number' ] = 0
    vert[ 'bottom_point_number' ] = 0
    aims.GraphManip.storeAims( graph, vert.get(), 'aims_Tmtktri', m )
    i += 1
  return graph, side

def writeref( argfile, uuid ):
    f = open( argfile + '.minf', 'w' )
    print >> f, "attributes = { 'referential' : '" + uuid + "' }"
    f.close()

if __name__ == '__main__':
  if len(sys.argv) == 1 or sys.argv[1] in ( '-h', '--help' ):
    print 'usage:', sys.argv[0], 'mesh_dir [referential_uuid]'
    print 'makes 3 graphs from a set of SPAM model mesges (3 levels)'
    print 'output graphs are written in the meshes directory'
    sys.exit( 1 )

  meshdir = sys.argv[1]
  outdir = meshdir
  if len( sys.argv ) >= 3:
    refuuid = sys.argv[2]
  else:
    refuuid = '5f83f18d-e211-6705-99a0-720c4707901b'
  graph0, side = makeSpamGraph( 0, meshdir )
  gname = os.path.join( outdir, side + 'spam_model_meshes_0.arg' )
  aims.write( graph0, gname )
  writeref( gname, refuuid )
  graph1, side = makeSpamGraph( 1, meshdir )
  gname = os.path.join( outdir, side + 'spam_model_meshes_1.arg' )
  aims.write( graph1, gname )
  writeref( gname, refuuid )
  graph2, side = makeSpamGraph( 2, meshdir )
  gname = os.path.join( outdir, side + 'spam_model_meshes_2.arg' )
  aims.write( graph2, gname )
  writeref( gname, refuuid )

