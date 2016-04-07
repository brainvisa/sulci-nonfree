/*
 *  Copyright (C) 2005 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 */

#include <si/fold/foldvertex.h>
#include <si/fold/fattrib.h>

using namespace sigraph;
using namespace carto;
using namespace std;


FoldVertex::~FoldVertex()
{
}


FoldVertex::FoldVertex( const std::string & s )
  : Vertex( s ), size( 0 ), refsize( 0 ), refsize_valid( false ), 
    index( 0 ), index_valid( false ), 
    label_valid( false ), gravity_center_valid( false ), 
    refgravity_center_valid( false ), normal_valid( false ), 
    refnormal_valid( false ), 
    mindepth( 0 ), mindepth_valid( false ), 
    refmindepth( 0 ), refmindepth_valid( false ), 
    maxdepth( 0 ), maxdepth_valid( false ), 
    refmaxdepth( 0 ), refmaxdepth_valid( false ), 
    meandepth( 0 ), meandepth_valid( false ), 
    refmeandepth( 0 ), refmeandepth_valid( false ), 
    surface_area( 0 ), surface_area_valid( false ), 
    refsurface_area( 0 ), refsurface_area_valid( false ), 
    bottom_point_number( 0 ), bottom_point_number_valid( false )
{
  addBuiltins();
}


FoldVertex::FoldVertex( const FoldVertex & x )
  : RCObject(), Vertex( x ), size( x.size ), 
    refsize( x.refsize ), refsize_valid( x.refsize_valid ), 
    index( x.index ), index_valid( x.index_valid ), 
    label( x.label ), label_valid( x.label_valid ), 
    gravity_center( x.gravity_center), 
    gravity_center_valid( x.gravity_center_valid ), 
    refgravity_center( x.refgravity_center), 
    refgravity_center_valid( x.refgravity_center_valid ), 
    normal( x.normal ), normal_valid( x.normal_valid ), 
    refnormal( x.refnormal ), refnormal_valid( x.refnormal_valid ), 
    mindepth( x.mindepth ), mindepth_valid( x.mindepth_valid ), 
    refmindepth( x.refmindepth ), refmindepth_valid( x.refmindepth_valid ), 
    maxdepth( x.maxdepth ), maxdepth_valid( x.maxdepth_valid ), 
    refmaxdepth( x.refmaxdepth ), refmaxdepth_valid( x.refmaxdepth_valid ), 
    meandepth( x.meandepth ), meandepth_valid( x.meandepth_valid ), 
    refmeandepth( x.refmeandepth ), 
    refmeandepth_valid( x.refmeandepth_valid ), 
    surface_area( x.surface_area ), 
    surface_area_valid( x.surface_area_valid ), 
    refsurface_area( x.refsurface_area ), 
    refsurface_area_valid( x.refsurface_area_valid ), 
    bottom_point_number( x.bottom_point_number ), 
    bottom_point_number_valid( x.bottom_point_number_valid )
{
  addBuiltins();
}


void FoldVertex::addBuiltins()
{
  PropertySet	& ps = getValue();
  ps.addBuiltinProperty( SIA_SIZE, size );
  ps.addBuiltinProperty( SIA_REFSIZE, refsize, refsize_valid );
  ps.addBuiltinProperty( SIA_INDEX, index, index_valid );
  ps.addBuiltinProperty( SIA_LABEL, label, label_valid );
  ps.addBuiltinProperty( SIA_GRAVITY_CENTER, gravity_center, 
                         gravity_center_valid );
  ps.addBuiltinProperty( SIA_REFGRAVITY_CENTER, refgravity_center, 
                         refgravity_center_valid );
  ps.addBuiltinProperty( SIA_NORMAL, normal, normal_valid );
  ps.addBuiltinProperty( SIA_REFNORMAL, refnormal, refnormal_valid );
  ps.addBuiltinProperty( SIA_MINDEPTH, mindepth, mindepth_valid );
  ps.addBuiltinProperty( SIA_REFMINDEPTH, refmindepth, refmindepth_valid );
  ps.addBuiltinProperty( SIA_MAXDEPTH, maxdepth, maxdepth_valid );
  ps.addBuiltinProperty( SIA_REFMAXDEPTH, refmaxdepth, refmaxdepth_valid );
  ps.addBuiltinProperty( SIA_MEANDEPTH, meandepth, meandepth_valid );
  ps.addBuiltinProperty( SIA_REFMEANDEPTH, refmeandepth, refmeandepth_valid );
  ps.addBuiltinProperty( SIA_SURFACE_AREA, surface_area, surface_area_valid );
  ps.addBuiltinProperty( SIA_REFSURFACE_AREA, refsurface_area, 
                         refsurface_area_valid );
  ps.addBuiltinProperty( SIA_BOTTOM_POINT_NUMBER, bottom_point_number, 
                         bottom_point_number_valid );
}


Vertex * FoldVertex::cloneVertex() const
{
  return new FoldVertex( *this );
}


GenericObject* FoldVertex::makeFold( const string & s )
{
  return new FoldVertex( s );
}


