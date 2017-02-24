/*
 *  Copyright (C) 2000-2004 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 */

#include <anafold/fgraph/fgMethod.h>
#include <anafold/fgraph/afgraph.h>
#include <si/graph/mgraph.h>
#include <si/graph/cgraph.h>
#include <qobject.h>

using namespace anatomist;
using namespace sigraph;
using namespace std;


string FGraphMethod::ID() const
{
  return( QT_TRANSLATE_NOOP( "FusionChooser", "FusionGraphs" ) );
}


string FGraphMethod::generatedObjectType() const
{
  return AObject::objectTypeName( AFGraph::classType() );
}


FGraphMethod::~FGraphMethod()
{
}


int FGraphMethod::canFusion( const set<AObject *> & obj )
{
  if( obj.size() != 2 )
    return 0;

  set<AObject *>::const_iterator	io = obj.begin();
  AObject				*o1, *o2;

  o1 = *io;
  ++io;
  o2 = *io;

  if( o1->type() == AObject::GRAPH && o2->type() == AObject::GRAPH )
  {
    if( dynamic_cast<MGraph *>( ((AGraph *) o1)->graph() ) )
    {
      if( dynamic_cast<CGraph *>( ((AGraph *) o2)->graph() ) )
        return 170;
      else
        return 0;
    }
    else if( dynamic_cast<CGraph *>( ((AGraph *) o1)->graph() ) )
    {
      if( dynamic_cast<MGraph *>( ((AGraph *) o2)->graph() ) )
        return 150;
      else
        return 0;
    }
  }

  return 0;
}


AObject* FGraphMethod::fusion( const vector<AObject *> & obj )
{
  vector<AObject *>::const_iterator	io = obj.begin();
  AGraph				*o = (AGraph *) *io;

  ++io;
  return( new AFGraph( o, (AGraph *) *io ) );
}


