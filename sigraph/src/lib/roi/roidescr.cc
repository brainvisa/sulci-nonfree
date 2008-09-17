/*
 *  Copyright (C) 2003 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 */

#include <si/roi/roidescr.h>
#include <si/roi/roiattrib.h>
#include <si/graph/vertexclique.h>
#include <graph/tree/tree.h>

using namespace sigraph;
using namespace carto;
using namespace std;

RoiDescr::RoiDescr() : AdapDescr()
{
}


RoiDescr::RoiDescr( const RoiDescr & f )
  : AdapDescr( f )
{
}


RoiDescr::~RoiDescr()
{
}


bool RoiDescr::makeVectorElements( const Clique* cl, vector<double> & vec, 
                                   GenericObject* )
{
  const VertexClique	*vcl = (const VertexClique *) cl;

  //cout << "RoiDescr::makeVector\n";

  VertexClique::iterator	iv, ev = vcl->end();
  Vertex	*v;
  float		size = 0, st;

  for( iv=vcl->begin(); iv!=ev; ++iv )
    {
      v = *iv;
      st = 0;
      v->getProperty( "size", st );
      size += st;
    }

  //	filling vector

  vec.push_back( size );

  return( true );
}


void RoiDescr::buildTree( Tree & t )
{
  t.setSyntax( SIA_ROI_DESCRIPTOR );
}


bool RoiDescr::hasChanged( const Clique* /*cl*/, 
                           const map<Vertex*, string> & /*changes*/, 
                           const GenericObject* /*model*/ ) const
{
  return( true );
}


vector<string> RoiDescr::descriptorsNames() const
{
  static vector<string>	names;
  if( names.empty() )
    {
      names.reserve( 2 );

      names.push_back( "volume" );
    }
  return names;
}


string RoiDescr::name() const
{
  return SIA_ROI_DESCRIPTOR;
}


