/*
 *  Copyright (C) 1998-2005 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 */

#include <si/finder/modelFinder.h>
#include <si/graph/cgraph.h>
#include <si/graph/vertexclique.h>
#include <si/graph/attrib.h>
#include <aims/selection/selection.h>
#include <cartobase/exception/assert.h>

using namespace sigraph;
using namespace aims;
using namespace carto;
using namespace std;


ModelFinder::ModelFinder( MGraph & rg ) : _mgraph( rg )
{
}


ModelFinder::~ModelFinder()
{
}


double ModelFinder::potential( const Clique* cl )
{
  double		res;
  AttributedObject	*modV = selectModel( cl );

  if( modV )
    {
      Model	*mod;
      ASSERT( modV->getProperty( "model", mod ) );
      res = mod->prop( cl );
    }
  else
    {
      res = 1.;
    }

  return( res );
}


double ModelFinder::potential( const Clique* cl, 
			       map<Vertex*, string> & changes )
{
  double		res;
  AttributedObject	*modV = selectModel( cl );

  if( modV )
    {
      Model	*mod;
      ASSERT( modV->getProperty( "model", mod ) );
      res = mod->prop( cl, changes );
    }
  else
    {
      res = 1.;
    }

  return( res );
}


double ModelFinder::update( const Clique* cl )
{
  double		res;
  AttributedObject	*modV = selectModel( cl );

  if( modV )
    {
      Model	*mod;
      ASSERT( modV->getProperty( "model", mod ) );
      res = mod->update( cl );
    }
  else
    {
      res = 0.;
    }

  return( res );
}


double ModelFinder::update( const Clique* cl, 
			    map<Vertex*, string> & changes )
{
  double		res;
  AttributedObject	*modV = selectModel( cl );

  if( modV )
    {
      Model	*mod;
      ASSERT( modV->getProperty( "model", mod ) );
      res = mod->update( cl, changes );
    }
  else
    {
      res = 0.;
    }

  return( res );
}


double ModelFinder::printDescription( Clique *cl, bool naming )
{
  double		res;
  AttributedObject	*modV = selectModel( cl );

  if( modV )
    {
      Model	*mod;
      ASSERT( modV->getProperty( "model", mod ) );
      res = mod->printDescription( cl, naming );
    }
  else
    {
      res = 1.;
    }

  return( res );
}


void ModelFinder::initCliques( CGraph & data, bool, bool, bool, bool, 
                               const SelectionSet *sel )
{
  data.deleteCliques();

  //	correspondance labels
  map<string, string>	trans;
  SelectionSet::const_iterator	isel, esel;
  Selection::const_iterator	is, es;
  string			label;
  string			voidl = SIV_VOID_LABEL;

  _mgraph.getProperty( SIA_VOID_LABEL, voidl );

  if( sel )
    for( isel=sel->begin(), esel=sel->end(); isel!=esel; ++isel )
      {
        label = isel->name();
        for( is=isel->begin(), es=isel->end(); is!=es; ++is )
          trans[*is] = label;
      }

  CGraph::iterator			iv, fv=data.end();
  Vertex				*v;
  map<string, VertexClique*>		mc;
  map<string, VertexClique*>::iterator	imc, emc = mc.end();
  VertexClique				*cl;
  CGraph::CliqueSet			& cliques = data.cliques();
  map<string, string>::iterator		il, el = trans.end();
  bool	found;

  for( iv=data.begin(); iv!=fv; ++iv )
    {
      v = *iv;

      label = "";
      found = v->getProperty( SIA_LABEL, label );
      if( !found )
        found = v->getProperty( SIA_NAME, label );
      if( found && label == voidl )
      	found = false;

      if( found && sel )
        {
          il = trans.find( label );
          if( il == el )
            found = false;
        }

      if( found )
        {
          imc = mc.find( label );
          if( imc == emc )
            {
              cl = new VertexClique;
              cl->setProperty( SIA_MODEL_TYPE, (string) SIV_RANDOM_VERTEX );
              cl->setProperty( SIA_LABEL, label );
              cl->setProperty( SIA_GRAPH, (Graph *) &data );
              mc[ label ] = cl;
              cliques.insert( rc_ptr<Clique>( cl ) );
            }
          else
            cl = imc->second;

          cl->addVertex( v );	// noeud dans la clique
        }
    }
}


#include <cartobase/object/object_d.h>
INSTANTIATE_GENERIC_OBJECT_TYPE( ModelFinder * )

