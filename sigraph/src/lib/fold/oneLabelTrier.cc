/*
 *  Copyright (C) 1998-2004 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 *  Slices of a mask to buckets
 */

#include <si/fold/oneLabelTrier.h>
#include <si/graph/vertexclique.h>
#include <aims/math/random.h>
#include <vector>
#include <math.h>
#include <assert.h>

using namespace sigraph;
using namespace std;


double OneLabelTrier::constrainedNoise( Clique* cl, double & outp, 
					const set<string> & sl, 
					const string & vl )
{
  double	dist;
  string	mtype;

  //cout << "OneLabelTrier...\n";

  if( cl->getProperty( "model_type", mtype ) && mtype == "random_edge" )
    {
      _edge = true;
      cl->getProperty( "label1", _label1 );
      cl->getProperty( "label2", _label2 );
      dist = 0;

      for( int i=0; i<3 && dist==0; ++i ) // 3 essais avant de laisser tomber
	dist = constrainedNoiseOLT( cl, outp, sl, vl );
    }
  else
    {
      _edge = false;
      dist = 0;
      for( int i=0; i<3 && dist==0; ++i ) // 3 essais avant de laisser tomber
	dist = constrainedNoiseOLT( cl, outp, sl, vl );
    }

  outp = output( outp, dist );

  return( dist );
}


double 
OneLabelTrier::constrainedNoiseOLT( Clique* cl, double &, 
				    const set<string> & sl, 
				    const string & vl )
{
  //cout << "constrainedNoiseOLT()\n";
  VertexClique::const_iterator	iv, fv=((const VertexClique *) cl)->end();
  // noeuds potentiellement ajoutés, enlevés, changés
  set<Vertex *>			an, rn;
  string			label, oldlabel;
  set<string>::const_iterator	fsl = sl.end();
  Vertex			*v;
  vector<string>		*plv;

  // Distance a l'exemple d'origine
  double			dist = 0;

  //	tri des noeuds de la clique en 3 catégories
  for( iv=((const VertexClique *) cl)->begin(); iv!=fv; ++iv )
    {
      v = *iv;
      v->getProperty( "label", label );
      if( label == vl || sl.find( label ) == fsl )
	an.insert( v );		// ajoutable
      else
	{
	  rn.insert( v );	// enlevable
	  assert( v->getProperty( "possible_labels", plv ) );
	}
    }

  if( an.size() + rn.size() == 0 )	// on peut rien changer
    return( false );

  unsigned	na;		// nombres de changements

  if( an.size() == 0 )
    na = 0;	// nb d'ajouts effectifs
  else
    na = 1;

  // bon, maintenant qu'on sait combien on change de chaque, il faut le faire

  set<unsigned>				todo;
  set<unsigned>::iterator		it, ft;
  unsigned				i, j, num;
  set<string>::iterator			il;
  bool					changed = false;
  //  int				toto;

  //	enlèvements de labels significatifs (-> void)

  set<Vertex *>::iterator	ir, fr;

  //cout << "enleves : " << rn.size() << "; ";
  for( ir=rn.begin(), fr=rn.end(); ir!=fr; ++ir )
    {
      //	mettre au label void
      (*ir)->getProperty( "label", oldlabel );
      (*ir)->setProperty( "label", vl );
      changed = true;
      //(*ir)->getProperty( "skeleton_label", toto );
      //cout << toto << "  ";

      // Distance a l'exemple d'origine
      dist += distance( *ir, oldlabel, vl );
    }
  //cout << endl;

  //	ajouts de noeuds (label void -> significatif)

  todo.erase( todo.begin(), todo.end() );
  //cout << "ajouts : " << na << " / " << an.size() << endl;
  while( todo.size() < na )
    todo.insert( unsigned( UniformRandom( (const unsigned &) 0U, 
                                        (const unsigned &) an.size() - 1 ) ) );

  for( ir=an.begin(), i=0, it=todo.begin(), ft=todo.end(); it!=ft; ++it )
    {
      //	positionner l'itérateur de noeuds
      while( i < *it )
	{
	  ++i;
	  ++ir;
	}
      //	choix du nouveau label: dans inter( sl, pl ) \ vl
      (*ir)->getProperty( "possible_labels", plv );
      set<string>	pl( plv->begin(), plv->end() );
      set<string>	poss;
      intersection( pl, sl, poss );
      poss.erase( vl );

      if( poss.size() != 0 )
	{
	  // tirage parmi ceux-là
	  num = unsigned( UniformRandom( (const unsigned &) 0U, 
                                         (const unsigned &) poss.size() 
                                         - 1 ) );
	  for( j=0, il=poss.begin(); j<num; ++j )
	    ++il;
	  (*ir)->getProperty( "label", oldlabel );
	  (*ir)->setProperty( "label", *il );
	  changed = true;
	  // Distance a l'exemple d'origine
	  dist += distance( *ir, oldlabel, *il );
	}
      //else
      //cerr << "pas de label significatif pour un noeud.\n";
    }
  //cout << endl;

  //	Bon, moi je trouve ça compliqué, tâtu et tout, comme fonction.
  //	j'ai mal au crâne...

  return( changed ? dist : 0. );
}
