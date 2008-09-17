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

#include <si/fold/brainJuncDescr.h>
#include <graph/tree/tree.h>
#include <si/graph/vertexclique.h>
#include <si/model/model.h>
#include <si/fold/fattrib.h>
#include <graph/graph/edge.h>
#include <iostream>

using namespace sigraph;
using namespace carto;
using namespace std;


BrainJuncDescr::BrainJuncDescr() : AdapDescr()
{
}


BrainJuncDescr::BrainJuncDescr( const BrainJuncDescr & h ) : AdapDescr( h )
{
}


BrainJuncDescr::~BrainJuncDescr()
{
}


bool BrainJuncDescr::makeVectorElements( const Clique* cl, 
                                         vector<double> & vec, 
                                         GenericObject* )
{
  const VertexClique		*vcl = (const VertexClique *) cl;
  string			label1, label2;
  set<Edge *>			ed;
  set<Edge *>::const_iterator	ie, fe;
  double			size = 0, csz = 0;
  float				s;

  if( !cl->getProperty( "label1", label1 ) )
    {
      cerr << "Mauvaise clique.\n";
      Object			is;
      string			str, att;
      cerr << "Nb attr : " << cl->AttributedObject::size() << endl;
      for( is=cl->objectIterator(); is->isValid(); is->next() )
	{
          att = is->key();
	  if( att == "model_type" || att == "label" || att == "label1" 
	      || att == "label2" )
	    {
	      cl->getProperty( att, str );
	      cerr << att << " : " << str << endl;
	    }
	  else
	    cerr << att << endl;
	}
      bool clique_has_no_attribute_label1 = true;
      assert( !clique_has_no_attribute_label1 );
    }
  assert( cl->getProperty( "label2", label2 ) );


  //	Opérations sur le sillon avec un vrai label

  string	label;

  if( label2 == SIV_VOID_LABEL )
    label = label1;
  else
    label = label2;

  set<Vertex *> sv = vcl->getVerticesWith( "label", label );
  set<Vertex *>::const_iterator	iv, fv=sv.end();
  double	ss = 0;

  if( sv.size() == 0 )	// pas de sillon
    {
      vec.push_back( 0 );
      vec.push_back( 0 );
      vec.push_back( 0 );
      vec.push_back( 0 );
      vec.push_back( 0 );
      vec.push_back( 0 );
      vec.push_back( 0 );
      vec.push_back( 0 );
      vec.push_back( 0 );
      vec.push_back( 0 );
      return( true );
    }

  for( iv=sv.begin(); iv!=fv; ++iv )
    if( (*iv)->getProperty( "size", s ) )
      ss += s;

  //	relations entre les 2 labels
  vector<float>		gj, g, gc;
  string		synt;

  gj.push_back( 0 );
  gj.push_back( 0 );
  gj.push_back( 0 );
  gc.push_back( 0 );
  gc.push_back( 0 );
  gc.push_back( 0 );

  vcl->edgesBetweenLabels( label1, label2, ed );

  for( ie=ed.begin(), fe=ed.end(); ie!=fe; ++ie )
    {
      if( (*ie)->getProperty( "size", s ) 
	  && (*ie)->getProperty( "gravity_center_Tal", g ) )
	{
	  synt = (*ie)->getSyntax();
	  if( synt == "cortical" )
	    {
	      csz += s;
	      gc[0] += g[0] * s;
	      gc[1] += g[1] * s;
	      gc[2] += g[2] * s;
	    }
	  else if( synt == "junction" )
	    {
	      size += s;
	      gj[0] += g[0] * s;
	      gj[1] += g[1] * s;
	      gj[2] += g[2] * s;
	    }
	}
    }

  if( size > 0 )
    {
      gj[0] /= size;
      gj[1] /= size;
      gj[2] /= size;
    }
  if( csz > 0 )
    {
      gc[0] /= csz;
      gc[1] /= csz;
      gc[2] /= csz;
    }

  //	Remplissage du vecteur

  vec.push_back( size );	// Taille de la relation
  vec.push_back( gj[0] );
  vec.push_back( gj[1] );
  vec.push_back( gj[2] );
  vec.push_back( csz );		// Taille de la relation corticale
  vec.push_back( gc[0] );
  vec.push_back( gc[1] );
  vec.push_back( gc[2] );
  vec.push_back( vcl->connectivity( sv ) );
  vec.push_back( ss );		// taille du sillon

  return( true );
}


void BrainJuncDescr::buildTree( Tree & t )
{
  t.setSyntax( "brain_junction_descriptor" );
}


bool BrainJuncDescr::hasChanged( const Clique* cl, 
				 const map<Vertex*, string> & changes, 
				 const GenericObject* model ) const
{
  const VertexClique	*vcl = (const VertexClique *) cl;
  Model			*mod;
  TopModel		*tm = 0;

  if( !model || !model->getProperty( "model", mod ) || !(tm=mod->topModel()) )
    return( true );	// manque des trucs: recalcule tout

  VertexClique::const_iterator		iv, fv=vcl->end();
  map<Vertex *, string>::const_iterator	im, fm=changes.end();
  string				label2;

  set<string>	& sl = tm->significantLabels();
  string	vl = tm->voidLabel();

  if( sl.size() == 0 || vl.size() == 0 )
    return( true );	// labels significatifs pourris

  set<string>::const_iterator		fs = sl.end();

  for( iv=vcl->begin(); iv!=fv; ++iv )
    if( (im=changes.find( *iv )) != fm )
      {
	const string	&label1 = (*im).second;
	if( label1 != vl && sl.find( label1 ) != fs )	// ancien label
	  return( true );				// non-void
	(*iv)->getProperty( "label", label2 );
	if( label2 != vl && sl.find( label2 ) != fs )	// nouveau label
	  return( true );				// non-void
      }

  return( false );	// si rien n'a changé, on ne recalcule pas
}




