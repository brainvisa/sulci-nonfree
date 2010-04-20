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

#include <si/fold/foldFinder.h>
#include <si/model/adaptive.h>
#include <si/graph/cgraph.h>
#include <si/graph/vertexclique.h>
#include <si/domain/domain.h>
#include <si/fold/labelsTranslator.h>
#include <si/fold/foldCache.h>
#include <si/fold/interFoldCache.h>
#include <si/fold/fattrib.h>
#include <aims/selection/selection.h>
#include <cartobase/exception/assert.h>
#include <iostream>
#include <iomanip>

using namespace sigraph;
using namespace aims;
using namespace carto;
using namespace std;


FoldFinder::FoldFinder( MGraph & rg ) : AdapFinder( rg )
{
}


FoldFinder::~FoldFinder()
{
}


AttributedObject* FoldFinder::selectModel( const Clique* cl )
{
  AttributedObject	*mod = 0;
  string		mt;

  map<const Clique*, AttributedObject*>::const_iterator	ic = _cache.find( cl );
  if( ic != _cache.end() )	// était dans le cache ?
    {
      //cout << "FoldFinder::selectModel : cached model found\n";
      return( (*ic).second );
    }

  ASSERT( cl->getProperty( SIA_MODEL_TYPE, mt ) );  // doit être pré-calculé
  if( mt == SIV_RANDOM_VERTEX )
    {
      string	str;

      cl->getProperty( SIA_LABEL, str );
      set<Vertex*>	sv = _mgraph.getVerticesWith( SIA_LABEL, str );
      if( sv.size() >= 1 )
	mod = *sv.begin();
      if( sv.size() != 1 )
	cerr << "FoldFinder::selectModel : " << sv.size() 
	     << " model(s) corresponding to vertex clique " << str << "\n";
    }
  else if( mt == SIV_RANDOM_EDGE )
    {
      string	str1, str2;

      cl->getProperty( SIA_LABEL1, str1 );
      cl->getProperty( SIA_LABEL2, str2 );
      set<Vertex*>	sv1 = _mgraph.getVerticesWith( SIA_LABEL, str1 );
      set<Vertex*>	sv2 = _mgraph.getVerticesWith( SIA_LABEL, str2 );
      if( sv1.size() == 1 && sv2.size() == 1 )
	{
	  set<Edge*>	se = (*sv1.begin())->edgesTo( *sv2.begin() );
	  if( se.size() == 1 )
	    mod = *se.begin();
	  else
	    cerr << "FoldFinder::selectModel : wrong number of relations in " 
		 << "model : " << se.size() << " between " << str1 << " and " 
		 << str2 << ", should be 1\n";
	}
      else
	cerr << "FoldFinder::selectModel : clique marked as edge from "
	     << str1 << " to " << str2 << ", not found in model: " 
	     << sv1.size() << " nodes -> " << sv2.size() << " nodes\n";
    }
  else if( mt == SIV_FAKE_REL )
    {
      if( _mgraph.hasProperty( SIA_FAKEREL_MODEL ) )
	mod = &_mgraph;
    }
  else
    {
      cout << "Clique of model " << mt << " : unknown model type.\n";
      // prends le modèle par défaut s'il y en a un
      _mgraph.getProperty( SIA_DEFAULT_MODEL, mod );
    }

  if( !cl->hasProperty( SIA_IS_COPY ) )
    _cache[ cl ] = mod;	// cacher seulement si la clique n'est pas une copie
  return( mod );
}


void FoldFinder::clear()
{
  _cache.erase( _cache.begin(), _cache.end() );
}


void FoldFinder::initCliques( CGraph & data, bool verbose, bool withCache, 
                              bool translateLabels, bool checkLabels, 
                              const SelectionSet *sel )
{
  data.deleteCliques();

  //	correspondance labels
  FoldLabelsTranslator	transl;
  string		label;
  set<string>		selected;

  if( sel )
    {
      SelectionSet::const_iterator	isel, esel;
      Selection::const_iterator		is, es;

      for( isel=sel->begin(), esel=sel->end(); isel!=esel; ++isel )
        {
          label = isel->name();
          selected.insert( label );
          for( is=isel->begin(), es=isel->end(); is!=es; ++is )
            transl[*is] = label;
        }
    }
  else
    transl.makeFromModel( _mgraph, "" );

  if( translateLabels || sel )
    transl.translate( data, SIA_LABEL, SIA_LABEL, SIA_NAME );

  CGraph::iterator			iv, fv=data.end();
  MGraph::iterator			im, fm=_mgraph.end();
  Domain				*fd;
  Vertex				*v, *rv1, *rv2;
  string				label1, label2;
  vector<string>			*pl;
  map<string, VertexClique*>		mc;
  map<string, VertexClique*>::iterator	imc, fmc, jmc;
  VertexClique				*cl, *cl1, *cl2;
  set<Vertex*>				sv;
  set<Edge*>				se;
  VertexClique::iterator		ic, fc;
  unsigned				i, n;
  set<Clique *>				& cliques = data.cliques();

  //	Création des cliques "Random Vertex" (vides d'abord)

  if( verbose )
    cout << "Creating Random Vertex cliques...\n";
  n = _mgraph.order();

  for( im=_mgraph.begin(), i=1; im!=fm; ++im, ++i )
    {
      (*im)->getProperty( SIA_LABEL, label1 );
      if( !sel || selected.find( label1 ) != selected.end() )
        {
          cl = new VertexClique;
          cl->setProperty( SIA_MODEL_TYPE, (string) SIV_RANDOM_VERTEX );
          cl->setProperty( SIA_LABEL, label1 );
          cl->setProperty( SIA_GRAPH, (Graph *) &data );
          if( withCache )
            cl->setProperty( SIA_ORIGINAL_CACHE, 
                              (CliqueCache *) new FoldCache );
          mc[ label1 ] = cl;
          cliques.insert( cl );
        }
      if( verbose )
	cout << "\r" << setw(5) << i << " / " << setw( 5 ) << n << flush;
    }

  //	Remplissage des cliques type "Random Vertex" et labels possibles 
  //	pour chaque noeud

  if( verbose )
    cout << "\nFilling Random Vertex cliques...\n";
  n = data.order();
  bool	checklabel, found;
  fmc=mc.end();

  for( iv=data.begin(), i=1; iv!=fv; ++iv, ++i )
    {
      if( verbose )
	cout << "\r" << setw( 5 ) << i << " / " << setw( 5 ) << n << flush;
      v = *iv;
      if( v->getProperty( SIA_POSSIBLE_LABELS, pl ) )
	delete pl;
      pl = new vector<string>;
      pl->push_back( SIV_VOID_LABEL );
      v->setProperty( SIA_POSSIBLE_LABELS, pl );

      label2 = "";
      checklabel = v->getProperty( SIA_LABEL, label2 );
      if( checklabel && label2 == SIV_VOID_LABEL )
      	checklabel = false;
	
      for( im=_mgraph.begin(); im!=fm; ++im )
	{
	  (*im)->getProperty( SIA_DOMAIN, fd );
	  found = fd->canBeFound( v, &data );
	  if( found || checklabel )
	    {
	      (*im)->getProperty( SIA_LABEL, label1 );
	      if( found || label1 == label2 )
		{
		  imc = mc.find( label1 );
                  if( imc != fmc )
                    {
                      pl->push_back( label1 );	// label possible pour le noeud
                      cl = (*imc).second;
                      cl->addVertex( v );	// noeud dans la clique
                    }
		}
	    }
	}

    }

  //	Cliques type "Random Edge"

  if( _mgraph.size() > 0 )	// si pas de random edges, rien à faire
    {
      if( verbose )
	cout << "\nCreating Random Edge cliques...\n";
      n = mc.size();

      for( imc=mc.begin(), fmc=mc.end(), i=1; imc!=fmc; ++imc, ++i )
	{
	  if( verbose )
	    cout << "\r" << setw(5) << i << " / " << setw( 5 ) << n << flush;
	  cl1 = (*imc).second;
	  cl1->getProperty( SIA_LABEL, label1 );
	  sv = _mgraph.getVerticesWith( SIA_LABEL, label1 );
	  ASSERT( sv.size() == 1 );
	  rv1 = *sv.begin();		//	random vertex de label label1

	  for( jmc=imc; jmc!=fmc; ++jmc )
	    {
	      cl2 = (*jmc).second;
	      cl2->getProperty( SIA_LABEL, label2 );
	      sv = _mgraph.getVerticesWith( SIA_LABEL, label2 );
	      ASSERT( sv.size() == 1 );
	      rv2 = *sv.begin();	//	random vertex de label label2
	      se = rv1->edgesTo( rv2 );	//	relation existante ?

	      if( se.size() >= 1 )
		{
		  cl = new VertexClique;
		  cl->setProperty( SIA_MODEL_TYPE, (string) SIV_RANDOM_EDGE );
		  cl->setProperty( SIA_GRAPH, (Graph *) &data );
		  cl->setProperty( SIA_LABEL1, label1 );
		  cl->setProperty( SIA_LABEL2, label2 );
		  if( withCache )
		    cl->setProperty( SIA_ORIGINAL_CACHE, 
				      (CliqueCache *) new InterFoldCache );
		  cliques.insert( cl );
		  // ajouter tous les noeuds (des 2 cliques)
		  for( ic=((const VertexClique *)cl1)->begin(), 
			 fc=((const VertexClique *)cl1)->end(); ic!=fc; ++ic )
		    cl->addVertex( *ic );
		  for( ic=((const VertexClique *)cl2)->begin(), 
			 fc=((const VertexClique *)cl2)->end(); ic!=fc; ++ic )
		    cl->addVertex( *ic );
		}
	    }
	}
    }

  //	clique "FAKE" pour toutes les relations qui ne sont pas dans le modèle
  if( _mgraph.hasProperty( SIA_FAKEREL_MODEL ) 
      && ( !sel || selected.find( SIV_VOID_LABEL ) != selected.end() ) )
    {
      cl = new VertexClique;
      cl->setSyntax( SIA_FAKEREL );
      cl->setProperty( SIA_MODEL_TYPE, (string) SIV_FAKE_REL );
      cl->setProperty( SIA_GRAPH, (Graph *) &data );
      cliques.insert( cl );
      for( iv=data.begin(), i=1; iv!=fv; ++iv, ++i )
	cl->addVertex( *iv );
    }

  if( verbose )
    cout << endl;

  if( checkLabels )
    data.ensureAllLabelsPossible();
}





