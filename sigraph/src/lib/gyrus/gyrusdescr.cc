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

#include <si/gyrus/gyrusdescr.h>
#include <si/gyrus/gyrusattrib.h>
#include <si/graph/vertexclique.h>
#include <graph/tree/tree.h>
#include <iostream>

using namespace sigraph;
using namespace carto;
using namespace std;

GyrusDescr::GyrusDescr() : AdapDescr()
{
}


GyrusDescr::GyrusDescr( const GyrusDescr & f )
  : AdapDescr( f )
{
}


GyrusDescr::~GyrusDescr()
{
}


bool GyrusDescr::makeVectorElements( const Clique* cl, vector<double> & vec, 
                                     GenericObject* )
{
  const VertexClique	*vcl = (const VertexClique *) cl;

  //cout << "GyrusDescr::makeVector\n";

  if( vcl->size() != 1 )
    {
      cerr << "Gyrus cliques should have 1 vertex, " << vcl->size() 
           << " found here" << endl;
      return false;
    }

  Vertex	*v = *vcl->begin();

  //	filling vector

  float	area = 0, size = 0;
  v->getProperty( "surface_area", area );
  vec.push_back( area );
  v->getProperty( "size", size );
  vec.push_back( size );
  vec.push_back( size / area );

  return( true );
}


void GyrusDescr::buildTree( Tree & t )
{
  t.setSyntax( SIA_GYRUS_DESCRIPTOR );
}


bool GyrusDescr::hasChanged( const Clique* /*cl*/, 
			     const map<Vertex*, string> & /*changes*/, 
			     const GenericObject* /*model*/ ) const
{
  return( true );
}


vector<string> GyrusDescr::descriptorsNames() const
{
  static vector<string>	names;
  if( names.empty() )
    {
      names.reserve( 2 );

      names.push_back( "surface_area" );
      names.push_back( "size" );
      names.push_back( "thickness" );
    }
  return names;
}


string GyrusDescr::name() const
{
  return SIA_GYRUS_DESCRIPTOR;
}


