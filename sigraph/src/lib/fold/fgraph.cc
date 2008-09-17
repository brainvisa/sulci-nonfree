/*
 *  Copyright (C) 1998-2003 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 */

#include <si/fold/fgraph.h>
#include <si/fold/fattrib.h>
#include <si/fold/foldvertex.h>
#include <si/fold/interfoldedge.h>

using namespace sigraph;
using namespace carto;
using namespace std;


FGraph::FGraph( const string s ) : CGraph( s )
{
  static bool done = false;
  if( !done )
    {
      // FIXME: FoldVertex is disabled until tested...
      _factory->registerGenerator( SIA_FOLD_SYNTAX, &FoldVertex::makeFold );
      _factory->registerGenerator( SIA_CORTICAL_SYNTAX, 
                                   &CorticalEdge::makeCortical );
      _factory->registerGenerator( SIA_JUNCTION_SYNTAX, 
                                   &JunctionEdge::makeJunction );
      _factory->registerGenerator( SIA_HULLJUNCTION_SYNTAX, 
                                   &HullJunctionEdge::makeHullJunction );
      _factory->registerGenerator( SIA_PLI_DE_PASSAGE_SYNTAX, 
                                   &PliDePassageEdge::makePliDePassage );
      done = true;
    }
}


FGraph::~FGraph()
{
}


void FGraph::flipHemispheres()
{
  vector<float>		gc, gc2;
  Vertex		*v;
  iterator		iv, ev=end();
  Vertex::iterator	ie, ee;
  Edge			*e;
  int			flipped = 0;
  bool			had_fa;
  float			tmp;

  //	global attributes
  if( getProperty( SIA_TAL_BOUNDINGBOX_MIN, gc ) 
      && getProperty( SIA_TAL_BOUNDINGBOX_MAX, gc2 ) )
    {
      tmp = gc[0];
      gc[0] = -gc2[0];
      gc2[0] = -tmp;
      setProperty( SIA_TAL_BOUNDINGBOX_MIN, gc );
      setProperty( SIA_TAL_BOUNDINGBOX_MAX, gc2 );
    }
  had_fa = getProperty( SIA_FLIPPED_HEMISPHERES, flipped );
  flipped = 1 - flipped;
  if( !flipped && had_fa )
    removeProperty( SIA_FLIPPED_HEMISPHERES );
  else if( flipped )
    setProperty( SIA_FLIPPED_HEMISPHERES, flipped );

  // contents
  for( iv=begin(); iv!=ev; ++iv )
    {
      v = *iv;
      //	flip vertex
      if( v->getProperty( SIA_REFGRAVITY_CENTER, gc ) )
	{
	  gc[0] = -gc[0];
	  v->setProperty( SIA_REFGRAVITY_CENTER, gc );
	}
      if( v->getProperty( SIA_REFNORMAL, gc ) )
	{
	  gc[0] = -gc[0];
	  v->setProperty( SIA_REFNORMAL, gc );
	}
      if( v->getProperty( SIA_TAL_BOUNDINGBOX_MIN, gc ) 
	  && v->getProperty( SIA_TAL_BOUNDINGBOX_MAX, gc2 ) )
	{
	  tmp = gc[0];
	  gc[0] = -gc2[0];
	  gc2[0] = -tmp;
	  v->setProperty( SIA_TAL_BOUNDINGBOX_MIN, gc );
	  v->setProperty( SIA_TAL_BOUNDINGBOX_MAX, gc2 );
	}

      //	flip edges
      for( ie=v->begin(), ee=v->end(); ie!=ee; ++ie )
	{
	  e = *ie;
	  if( *e->begin() == v )	// take each edge only once
	    {
	      if( e->getProperty( SIA_REFDIRECTION, gc ) )
		{
		  gc[0] = -gc[0];
		  e->setProperty( SIA_REFDIRECTION, gc );
		}
	      if( e->getProperty( SIA_REFEXTREMITY1, gc ) )
		{
		  gc[0] = -gc[0];
		  e->setProperty( SIA_REFEXTREMITY1, gc );
		}
	      if( e->getProperty( SIA_REFEXTREMITY2, gc ) )
		{
		  gc[0] = -gc[0];
		  e->setProperty( SIA_REFEXTREMITY2, gc );
		}
	      if( e->getProperty( SIA_REFSS1NEAREST, gc ) )
		{
		  gc[0] = -gc[0];
		  e->setProperty( SIA_REFSS1NEAREST, gc );
		}
	      if( e->getProperty( SIA_REFSS2NEAREST, gc ) )
		{
		  gc[0] = -gc[0];
		  e->setProperty( SIA_REFSS2NEAREST, gc );
		}
	    }
	}
    }
}



