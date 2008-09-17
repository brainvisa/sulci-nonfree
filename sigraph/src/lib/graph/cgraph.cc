/*
 *  Copyright (C) 1998-2004 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 */

#include <si/graph/cgraph.h>
#include <neur/rand/rand.h>
#include <si/graph/attrib.h>
#include <si/finder/modelFinder.h>
#include <aims/io/aimsGraphR.h>

using namespace sigraph;
using namespace aims;
using namespace carto;
using namespace std;


CGraph::CGraph( string s ) : Graph( s )
{
}


CGraph::~CGraph()
{
  deleteCliques();

  iterator	in, fn=end();

  //	attributs globaux graphe
  parseDelete( this );

  //	noeuds
  for( in=begin(); in!=fn; ++in )
    parseDelete( *in );

  //	relations
  set<Edge*> ed = edges();
  set<Edge*>::iterator	ir, fr=ed.end();

  for( ir=ed.begin(); ir!=fr; ++ir )
    parseDelete( *ir );
}


void CGraph::deleteCliques()
{
  set<Clique*>::iterator	ic, fc=_cliques.end();

  for( ic=_cliques.begin(); ic!=fc; ++ic )
    delete *ic;
  _cliques.erase( _cliques.begin(), fc );
}


void CGraph::parseDelete( AttributedObject *ao )
{
  if( ao->hasProperty( SIA_POSSIBLE_LABELS ) )
    {
      vector<string>	*ptr2;

      ao->getProperty( SIA_POSSIBLE_LABELS, ptr2 );
      delete ptr2;
    }
}


void CGraph::clearAll()
{
  iterator	in, fn=end();

  //	attributs globaux graphe
  parseDelete( this );

  //	noeuds
  for( in=begin(); in!=fn; ++in )
    parseDelete( *in );

  //	relations
  set<Edge*> ed = edges();
  set<Edge*>::iterator	ir, fr=ed.end();

  for( ir=ed.begin(); ir!=fr; ++ir )
    parseDelete( *ir );

  clearProperties();
  deleteCliques();
  clear();
}


void CGraph::randLabels()
{
  iterator		iv, fv=end();
  Vertex		*v;
  vector<string>	*pl;

  for( iv=begin(); iv!=fv; ++iv )
    {
      v = *iv;
      if( v->getProperty( SIA_POSSIBLE_LABELS, pl ) )
	{
	  if( pl->size() > 0 )
	    v->setProperty( SIA_LABEL, 
			     (*pl)[ (unsigned) ( ran1()*pl->size() ) ] );
	  else
	    {
	      string	label;
	      if( !v->getProperty( SIA_LABEL, label ) )
		label = "*** no label ***";
	      cerr << "Warning : possible_labels empty for node " << label 
		   << endl;
	      v->setProperty( SIA_LABEL, (string) "A" );
	    }
	}
      else v->setProperty( SIA_LABEL, (string) "A" );
    }
}


void CGraph::setAllLabels( const string & label )
{
  iterator		iv, fv=end();

  for( iv=begin(); iv!=fv; ++iv )
    (*iv)->setProperty( SIA_LABEL, label );
}


void CGraph::ensureAllLabelsPossible()
{
  const_iterator	iv, fv=end();
  string		label, vl;
  vector<string>	*pl;
  Vertex		*v;

  for( iv=begin(); iv!=fv; ++iv )
    {
      v = *iv;
      if( v->getProperty( SIA_LABEL, label ) 
	  && v->getProperty( SIA_POSSIBLE_LABELS, pl ) && !pl->empty()
	  &&  find( pl->begin(), pl->end(), label ) == pl->end() )
	{
	  if( !v->getProperty( "void_label", vl ) )
	    vl = *pl->begin();
	  v->setProperty( SIA_LABEL, vl );
	  cout << "Node label " << label << " changed to " << vl << endl;
	}
    }
}


void CGraph::loadBuckets( const string & filenamebase, bool rels )
{
  AimsGraphReader	gr( filenamebase );
  try
    {
      gr.readElements( *this, 1 + ( rels ? 2 : 0) );
      setProperty( "aims_elements_loaded", (bool) true );
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
    }
}


