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

#include <si/fold/interFoldDescr.h>
#include <si/graph/vertexclique.h>
#include <si/model/model.h>
#include <si/fold/interFoldCache.h>
#include <si/fold/fattrib.h>
#include <graph/tree/tree.h>
#include <graph/graph/graph.h>
#include <iostream>
#include <math.h>

using namespace sigraph;
using namespace carto;
using namespace std;


InterFoldDescr::~InterFoldDescr()
{
}


bool InterFoldDescr::makeVector( const Clique* cl, vector<double> & vec, 
				 GenericObject* model )
{
  CliqueCache			*cch;
  InterFoldCache		*ifc = 0;

  if( cl->getProperty( SIA_CACHE, cch ) 
      || cl->getProperty( SIA_ORIGINAL_CACHE, cch ) )
    {
      ifc = dynamic_cast<InterFoldCache *>( cch );
      assert( ifc );
      if( ifc->vecValid )
	{
	  vec = ifc->inputVector;
	  if (!vec[0]) return false;
	  return true;
	}
    }
  bool x = makeVectorElements( cl, vec, model );
  if( x && ifc )
    {
      // update cache
      ifc->inputVector = vec;
      ifc->vecValid = true;
    }
  return x;
}


bool InterFoldDescr::makeVectorElements( const Clique* cl, 
                                         vector<double> & vec, 
                                         GenericObject* )
{
  const VertexClique		*vcl = (const VertexClique *) cl;
  string			label1, label2;
  set<Edge *>			ed;
  set<Edge *>::const_iterator	ie, fe;
  if( !cl->getProperty( SIA_LABEL1, label1 ) )
    {
      cerr << "Mauvaise clique.\n";
      Object			is;
      string			str, att;
      cerr << "Nb attr : " << cl->AttributedObject::size() << endl;
      for( is=cl->objectIterator(); is->isValid(); is->next() )
	{
          att = is->key();
	  if( att == SIA_MODEL_TYPE || att == SIA_LABEL || att == SIA_LABEL1 
	      || att == SIA_LABEL2 )
	    {
	      cl->getProperty( att, str );
	      cerr << att << " : " << str << endl;
	    }
	  else
	    cerr << att << endl;
	}
      bool la_clique_n_a_pas_l_attribut_label1 = true;
      assert( !la_clique_n_a_pas_l_attribut_label1 );
    }
  assert( cl->getProperty( SIA_LABEL2, label2 ) );

  set<Vertex *>	sl1 = vcl->getVerticesWith( SIA_LABEL, label1 );
  set<Vertex *> sl2 = vcl->getVerticesWith( SIA_LABEL, label2 );
  set<Vertex *>::const_iterator	iv, fv=sl1.end();
  vector<float>			gc1, gc2, gc, gcc, gj;
  float				s, sum1 = 0, sum2 = 0;

  if( sl1.size() == 0 || sl2.size() == 0 )
    {	// un des sillons n'est pas là: pas de relation
      vec.push_back( 0 );	// pas valide
      vec.push_back( 0 );	// long.
      vec.push_back( 0 );	// dist
      vec.push_back( 0 );	// direction
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
      vec.push_back( 0 );
      vec.push_back( 0 );
      vec.push_back( 0 );
      vec.push_back( 0 );
      vec.push_back( 0 );
      return( false );
    }

  //	centres de gravites
  gc1.push_back( 0 );
  gc1.push_back( 0 );
  gc1.push_back( 0 );

  gc2.push_back( 0 );
  gc2.push_back( 0 );
  gc2.push_back( 0 );

  for( iv=sl1.begin(); iv!=fv; ++iv )
    if( (*iv)->getProperty( SIA_REFGRAVITY_CENTER, gc ) 
	&& (*iv)->getProperty( SIA_SIZE, s ) )
      {
	sum1 += s;
	gc1[0] += s * gc[0];
	gc1[1] += s * gc[1];
	gc1[2] += s * gc[2];
      }

  if( sum1 > 0 )
    {
      gc1[0] /= sum1;
      gc1[1] /= sum1;
      gc1[2] /= sum1;
    }

  for( iv=sl2.begin(), fv=sl2.end(); iv!=fv; ++iv )
    if( (*iv)->getProperty( SIA_GRAVITY_CENTER, gc ) 
	&& (*iv)->getProperty( SIA_SIZE, s ) )
      {
	sum2 += s;
	gc2[0] += s * gc[0];
	gc2[1] += s * gc[1];
	gc2[2] += s * gc[2];
      }

  if( sum2 > 0 )
    {
      gc2[0] /= sum2;
      gc2[1] /= sum2;
      gc2[2] /= sum2;
    }

  double	d = sqrt( (gc2[0] - gc1[0]) * (gc2[0] - gc1[0]) 
			  + (gc2[1] - gc1[1]) * (gc2[1] - gc1[1]) 
			  + (gc2[2] - gc1[2]) * (gc2[2] - gc1[2]) );


  //	relations entre les 2 labels 
  string	synt;
  double	sc = 0, sj = 0;

  vcl->edgesBetween( sl1, sl2, ed );
  gcc.push_back( 0 );	// centre gravité des relations corticales
  gcc.push_back( 0 );
  gcc.push_back( 0 );
  gj.push_back( 0 );	// centre gravité des jonctions
  gj.push_back( 0 );
  gj.push_back( 0 );

  for( ie=ed.begin(), fe=ed.end(); ie!=fe; ++ie )
    {
      synt = (*ie)->getSyntax();
      if( synt == SIA_CORTICAL_SYNTAX )
	{
	  if( (*ie)->getProperty( SIA_SIZE, s ) )
	    {
	      sc += s;
	      if( (*ie)->getProperty( SIA_GRAVITY_CENTER, gc ) )
		{
		  gcc[0] += gc[0] * s;
		  gcc[1] += gc[1] * s;
		  gcc[2] += gc[2] * s;
		}
	    }
	}
      else if( synt == SIA_JUNCTION_SYNTAX )
	{
	  if( (*ie)->getProperty( SIA_SIZE, s ) )
	    {
	      sj += s;
	      if( (*ie)->getProperty( SIA_GRAVITY_CENTER, gc ) )
		{
		  gj[0] += gc[0] * s;
		  gj[1] += gc[1] * s;
		  gj[2] += gc[2] * s;
		}
	    }
	}
    }

  if( sc > 0 )	// taille relations corticales
    {
      gcc[0] /= sc;
      gcc[1] /= sc;
      gcc[2] /= sc;
    }
  if( sj > 0 )	// taille jonctions
    {
      gj[0] /= sj;
      gj[1] /= sj;
      gj[2] /= sj;
    }

  vec.push_back( 1 );		// rel. valide

  vec.push_back( sum1 );	// taille 1er sillon
  vec.push_back( vcl->connectivity( sl1, 0, SIA_JUNCTION_SYNTAX ) );
  vec.push_back( sum2 );	// taille 2e sillon
  vec.push_back( vcl->connectivity( sl2, 0, SIA_JUNCTION_SYNTAX ) );

  vec.push_back( d );	// distance des centres de gravité
  vec.push_back( sc );	// taille des relations corticales
  vec.push_back( sj );	// taille des jonctions

  if( d == 0 )
    d = 1.;	// précaution
  vec.push_back( ( gc2[0] - gc1[0] ) / d );
  vec.push_back( ( gc2[1] - gc1[1] ) / d );
  vec.push_back( ( gc2[2] - gc1[2] ) / d );

  vec.push_back( sc > 0 ? 1 : 0 );	// flag cort. valide
  vec.push_back( gcc[0] );
  vec.push_back( gcc[1] );
  vec.push_back( gcc[2] );

  vec.push_back( sj > 0 ? 1 : 0 );	// flag jonc. valide
  vec.push_back( gj[0] );
  vec.push_back( gj[1] );
  vec.push_back( gj[2] );

  return true;
}


bool InterFoldDescr::makeLearnVector( const Clique* cl, vector<double> & vec, 
				      GenericObject* model )
{
  return( makeVectorElements( cl, vec, model ) );	// provisoire
}


void InterFoldDescr::buildTree( Tree & t )
{
  t.setSyntax( SIA_INTER_FOLD_DESCRIPTOR );
}


bool InterFoldDescr::hasChanged( const Clique* cl, 
				 const map<Vertex*, string> & changes, 
				 const GenericObject* model ) const
{
  const VertexClique	*vcl = (const VertexClique *) cl;
  Model			*mod;
  TopModel		*tm = 0;

  if( !model || !model->getProperty( SIA_MODEL, mod ) 
      || !(tm=mod->topModel()) )
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
	(*iv)->getProperty( SIA_LABEL, label2 );
	if( label2 != vl && sl.find( label2 ) != fs )	// nouveau label
	  return( true );				// non-void
      }

  return( false );	// si rien n'a changé, on ne recalcule pas
}
