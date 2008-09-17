/*
 *  Copyright (C) 2004-2005 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 */

#include <si/sulcalsketch/sulcalsketchfinder.h>
#include <si/sulcalsketch/sulcalsketchattrib.h>
#include <si/graph/cgraph.h>
#include <si/graph/vertexclique.h>
#include <si/domain/domain.h>
#include <iostream>

using namespace sigraph;
using namespace aims;
using namespace carto;
using namespace std;


SulcalSketchFinder::SulcalSketchFinder( MGraph & mg ) : ModelFinder( mg )
{
}


SulcalSketchFinder::~SulcalSketchFinder()
{
}


AttributedObject* SulcalSketchFinder::selectModel( const Clique* cl )
{
  AttributedObject	*mod = 0;
  string		mt;
  if( !cl->getProperty( SIA_MODEL_TYPE, mt ) )
    {
      cerr << "SulcalSketchFinder::selectModel: clique with no type\n";
    }
  if( mt == "sulcalsketch_similarity" )
    {
      string	str, mtype;

      cl->getProperty( SIA_LABEL, str );
      set<Vertex*>	sv = _mgraph.getVerticesWith( SIA_LABEL, str );
      set<Vertex*>::iterator	im, em = sv.end();
      for( im=sv.begin(); im!=em; ++im )
        {
          mtype = "";
          (*im)->getProperty( SIA_MODEL_TYPE, mtype );
          if( mtype != "datadriven" )
            {
              mod = *im;
              break;
            }
        }
    }
  else if( mt == "sulcalsketch_datadriven" )
    {
      string	str, mtype;

      cl->getProperty( SIA_LABEL, str );
      set<Vertex*>	sv = _mgraph.getVerticesWith( SIA_LABEL, str );
      set<Vertex*>::iterator	im, em = sv.end();
      for( im=sv.begin(); im!=em; ++im )
        {
          mtype = "";
          (*im)->getProperty( SIA_MODEL_TYPE, mtype );
          if( mtype == "datadriven" )
            {
              mod = *im;
              break;
            }
        }
    }
  else
    cerr << "Clique of model " << mt << " : unknown model type.\n";
  return mod;
}


void SulcalSketchFinder::initCliques( CGraph &data, bool /* verbose */, 
                                      bool /* withCache */, 
                                      bool /* translateLabels */, 
                                      bool /* checkLabels */, 
                                      const SelectionSet * /* sel */ )
{
  Graph::iterator	iv, ev = data.end();
  const MGraph		& mg = mGraph();
  Graph::const_iterator	imv, emv = mg.end();
  map<string, VertexClique *>	cliquessim;
  map<string, map<string, VertexClique *> > cliquesdata;
  string			label, label1, mtype, subject;
  VertexClique			*vc;
  set<Clique *>			& scl = data.cliques();
  Domain			*fd;
  Vertex			*v;
  map<string, VertexClique *>::iterator	imc, fmc = cliquessim.end();
  vector<string>		*pl;
  VertexClique			*cl;
  string			voidl = SIV_VOID_LABEL;

  _mgraph.getProperty( SIA_VOID_LABEL, voidl );

  //cout << "SulcalSketchFinder::initCliques : " << data.order() << endl;
  for( imv=mg.begin(); imv!=emv; ++imv )
    {
      (*imv)->getProperty( SIA_LABEL, label );
      mtype = "";
      (*imv)->getProperty( SIA_MODEL_TYPE, mtype );
      if( mtype != "datadriven" )
        {
          vc = new VertexClique;
          cliquessim[ label ] = vc;
          vc->setProperty( SIA_LABEL, label );
          vc->setProperty( SIA_MODEL_TYPE, 
                            string( "sulcalsketch_similarity" ) );
          scl.insert( vc );
        }
    }

  for( iv=data.begin(); iv!=ev; ++iv )
    {
      v = *iv;
      if( v->getProperty( SIA_POSSIBLE_LABELS, pl ) )
	delete pl;
      pl = new vector<string>;
      pl->push_back( voidl );
      v->setProperty( SIA_POSSIBLE_LABELS, pl );

      for( imv=_mgraph.begin(); imv!=emv; ++imv )
	{
	  (*imv)->getProperty( SIA_DOMAIN, fd );
          if( fd->canBeFound( v, &data ) )
            {
	      (*imv)->getProperty( SIA_LABEL, label1 );
              mtype = "";
	      (*imv)->getProperty( SIA_MODEL_TYPE, mtype );
              if( mtype == "datadriven" )
                {
                  v->getProperty( SIA_SUBJECT, subject );
                  map<string, VertexClique *>	& dc = cliquesdata[ subject ];
                  VertexClique	*& c = dc[ label1 ];
                  if( c == 0 )
                    {
                      c = new VertexClique;
                      c->setProperty( SIA_LABEL, label1 );
                      c->setProperty( SIA_MODEL_TYPE, 
                                       string( "sulcalsketch_datadriven" ) );
                      scl.insert( c );

                    }
                  pl->push_back( label1 );	// label possible pour le noeud
                  c->addVertex( v );	// noeud dans la clique
                }
              else
                {
                  imc = cliquessim.find( label1 );
                  if( imc != fmc )
                    {
                      pl->push_back( label1 );	// label possible pour le noeud
                      cl = (*imc).second;
                      cl->addVertex( v );	// noeud dans la clique
                    }
                  else
                    cerr << "problem: model label " << label1 
                         << " not in cliques list\n";
                }
            }
        }
      //cout << "vertex " << v << ": " << pl->size() << " possible labels\n";
    }

  map<string, map<string, VertexClique *> >::iterator 
    idc, edc = cliquesdata.end();
  map<string, VertexClique *>::iterator	idc2, edc2;
  int	ns = cliquesdata.size();

  for( idc=cliquesdata.begin(); idc!=edc; ++idc )
    for( idc2=idc->second.begin(), edc2=idc->second.end(); idc2!=edc2; ++idc2 )
      idc2->second->setProperty( "num_subjects", ns );
}


