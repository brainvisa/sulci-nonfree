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

#include <si/fold/foldLabels.h>
#include <si/subadaptive/subAdMlp.h>
#include <si/model/topModel.h>
#include <si/model/adaptiveLeaf.h>
#include <si/model/adaptiveTree.h>
#include <si/model/topAdaptive.h>
#include <si/model/nullModel.h>
#include <si/domain/nullDomain.h>
#include <cartobase/config/version.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <si/fold/fattrib.h>

using namespace sigraph;
using namespace std;


void FoldLabels::readLabels( const string & filename )
{
  char		buf[500], sill[200], trans[200];
  ifstream	file( filename.c_str() );

  assert( file );
  erase( begin(), end() );

  while( !file.eof() )
  {
    do
    {
      file.getline( buf, 500, '\n' );
    }
    while( !file.eof() && buf[0] != '\0' && buf[0] != '%' );
    // (sauter commentaires)

    if( !file.eof() && buf[0] != '\0' )
    {
      // cout << "ligne label : " << buf << endl;
      sscanf( buf+1, "%s", sill );
      trans[0] = '\0';
      sscanf( buf+1+strlen( sill ), "%s", trans );
      if( sill[0] != '\0' )
      {
        if( trans[0] != '\0' )
        {
          /*cout << "translated label " << trans << " from base "
            << sill << endl;*/
          insert( string( trans ) );
        }
        else
        {
          //cout << "base label " << sill << endl;
          insert( string( sill ) );
        }
      }
    }
  }

  iterator	is = find( SIV_VOID_LABEL );
  if( is != end() )
    {
      //cout << "Etiquette '" << SIV_VOID_LABEL << "' enlevée.\n";
      erase( is );
    }

  cout << size() << " labels de sillons lus.\n";
}


MGraph* FoldLabels::createModel( const Model* mod, const Domain* dom, 
				 const Model* defMod ) const
{
  MGraph		*mg = new FRGraph( SIA_MODEL_GRAPH_SYNTAX );
  const_iterator	is, fs=end();

  mg->setProperty( SIA_MODEL_VERSION, carto::cartobaseShortVersion() );
  mg->setProperty( SIA_MODEL_COMPAT_DATA_VERSION, string( "3.1" ) );

  for( is=begin(); is!=fs; ++is )
    {
      //	Hémisphère gauche
      makeVertex( mg, *is + "_left", mod, dom );

      //	Hémisphère droit
      makeVertex( mg, *is + "_right", mod, dom );
    }

  NullModel	nmod;
  NullDomain	ndom;

  makeVertex( mg, SIV_VOID_LABEL, &nmod, &ndom );

  if( defMod )
    {
      Model	*dm2 = defMod->clone();
      dm2->setBaseName( "default" );

      mg->setProperty( SIA_DEFAULT_MODEL, dm2 );
      mg->setProperty( SIA_DEFAULT_MODEL_FILE, 
			string( "adap/default.mod" ) );
    }

  return( mg );
}


void FoldLabels::makeVertex( MGraph* mg, const string & label, 
			     const Model* mod, const Domain* dom ) const
{
  Vertex	*v;
  Model		*cm = mod->clone();
  Domain	*cd = dom->clone();

  cm->setBaseName( label );

  v = mg->addVertex( SIA_MODEL_NODE );
  v->setProperty( SIA_LABEL, label );
  v->setProperty( SIA_MODEL_FILE, string("adap/") + label + ".mod" );
  v->setProperty( SIA_DOMAIN_FILE, string("domain/")
		   + label + ".dom" );
  v->setProperty( SIA_MODEL, cm );
  v->setProperty( SIA_DOMAIN, cd );

  // Labels reconnus

  TopModel	*tm = cm->topModel();
  if( tm )
    {
      set<string> & labs = tm->significantLabels();
      labs.erase( labs.begin(), labs.end() );
      labs.insert( label );
      if( label != SIV_VOID_LABEL )
	{
	  labs.insert( SIV_VOID_LABEL );
	  cm->topModel()->setVoidLabel( SIV_VOID_LABEL );
	}
      else
	{
	  labs.insert( SIV_OTHER );
	  cm->topModel()->setVoidLabel( SIV_OTHER );
	}
    }
}







