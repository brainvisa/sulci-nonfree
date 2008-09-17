/*
 *  Copyright (C) 1998-2004 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 */

#include <si/global/global.h>
#include <si/fold/foldLabels.h>
#include <si/model/mReader.h>
#include <si/domain/domReader.h>
#include <si/fold/frgReader.h>
#include <si/fold/frgWriter.h>
#include <si/fold/foldReader.h>
#include <si/fold/fdParser.h>
#include <si/fold/labelsTranslator.h>
#include <cartobase/exception/parse.h>
#include <cartobase/object/sreader.h>
#include <cartobase/stream/sstream.h>
#include <cartobase/plugin/plugin.h>
#include <iostream>
#include <string.h>

using namespace carto;
using namespace sigraph;
using namespace std;


struct Params
{
  string		modelFile;
  string		labelsFile;
  string		adapFile;
  float			freqMin;
  int			removeVoid;
  int			setWeights;
  //  string		domFile;
  vector<string>	graphs;
};


void usage( const char* name )
{
  cerr << "usage : \n" << name << " [-o num] modelGraph.arg labelsFile " 
       << "adapFile.mod foldgraphlist\n";
  cerr << name << " paramFile\n\n";
  cerr << "-o num  : removes edges whose occurence frequency in the data is \n"
       << "less or equal to num\n";
  exit( 1 );
}


void paramError( const char* name, const string & file, const string & param  )
{
  cerr << name << " : bad param in file " << file << ", param " << param 
       << endl;
  exit( EXIT_FAILURE );
}


void loadParams( const string & paramFile, const char* name, Params & params )
{
  SyntaxReader	pr( si().basePath() + "/config/siMkModelEdges.stx" );
  SyntaxSet	ps;

  pr >> ps;

  TreeReader	tr( paramFile, ps );
  Tree		t;
  string	gf, str;

  tr >> t;

  if( !t.getProperty( "model", params.modelFile ) )
    paramError( name, paramFile, "modelFile" );
  t.getProperty( "labels", params.labelsFile );
  if( !t.getProperty( "adap", params.adapFile ) )
    paramError( name, paramFile, "adap" );
  //  if( !t.getProperty( "domain", params.domFile ) )
  //    paramError( name, paramFile, "domain" );
  if( !t.getProperty( "graphs", gf ) )
    paramError( name, paramFile, "graphs" );
  if( !t.getProperty( "frequency_min", params.freqMin ) )
    params.freqMin = 0;
  if( !t.getProperty( "remove_void", params.removeVoid ) 
      && !t.getProperty( "remove_brain", params.removeVoid ) )
    params.removeVoid = 0;
  if( !t.getProperty( "set_weights", params.setWeights ) )
    params.setWeights = 1;

  istringstream	sst( gf.c_str() );

  while( !sst.eof() )
    {
      sst >> str;
      if( str.size() > 0 )
	params.graphs.push_back( str );
    }

  if( params.graphs.size() == 0 )
    paramError( name, paramFile, "graphs" );
}


int main( int argc, char** argv )
{
  Params	params;
  unsigned	i;

  PluginLoader::load();

  if( argc == 2 && strcmp( argv[1], "-h" ) != 0 && strcmp( argv[1], "--help" ) != 0 )
    loadParams( string( argv[1] ), argv[0], params );
  else if( argc >= 5 )
    {
      params.modelFile = argv[1];
      params.labelsFile = argv[2];
      params.adapFile = argv[3];
      //      params.domFile = argv[4];
      for( i=4; i<(unsigned)argc; ++i )
	params.graphs.push_back( argv[i] );
    }
  else usage( argv[0] );

  try
    {
      //	Lecture graphe modèle

      FRGraph	rg;
      FrgReader	rr( params.modelFile );

      try
	{
	  rr >> rg;
	  cout << "Lecture FRGraph OK." << endl;
	}
      catch( parse_error & e )
	{
	  cerr << e.what() << " : " << e.filename() << ", line " << e.line() 
	       << endl;
	  throw;
	}
      catch( exception & e )
	{
	  cerr << e.what() << endl;
	  throw;
	}

      //	Lecture de l'élément adaptatif

      MReader	ar( params.adapFile );
      FDParser	fdp;
      ar.addFactories( fdp.factories() );
      Model	*mod = ar.readModel();
      assert( mod );


      //	Lecture graphes exemples

      vector<FGraph*>		fg;

      for( i=0; i<params.graphs.size(); ++i )
	{
	  FoldReader	fr( params.graphs[i] );

	  fg.push_back( new FGraph );
	  try
	    {
	      fr >> *fg[i];
	      cout << "Lecture FGraph " << params.graphs[i] << " OK." << endl;
	    }
	  catch( parse_error & e )
	    {
	      cerr << e.what() << " : " << e.filename() << ", line " 
		   << e.line() << endl;
	      throw;
	    }
	  catch( exception & e )
	    {
	      cerr << e.what() << endl;
	      throw;
	    }
	}


      //	Correspondance des labels

      //if( params.labelsFile != "" )
      //si().setLabelsTranslPath( params.labelsFile );

      FoldLabelsTranslator	transl( rg, params.labelsFile );


      //	Ajout des relations dans le modèle

      for( i=0; i<fg.size(); ++i )
	{
	  transl.translate( *fg[i], "name", "label" );
	  rg.addEdges( *fg[i], mod );
	}

      //	Enlevage des relations avec unknown

      if( params.removeVoid )
	rg.removeEdgesToVoid();

      //	Enlevage des relations rares

      if( params.freqMin != 0 )
	rg.removeRareEdges( params.freqMin );


      //	Mettre les poids

      if( params.setWeights )
	rg.setWeights();


      //	Sauvegarde du modèle

      FrgWriter	rw( params.modelFile );

      try
	{
	  rw << rg;
	  cout << "Ecriture FRGraph OK." << endl;
	}
      catch( parse_error & e )
	{
	  cerr << e.what() << " : " << e.filename() << ", line " << e.line() 
	       << endl;
	  throw;
	}
      catch( exception & e )
	{
	  cerr << e.what() << endl;
	  throw;
	}


      //	Destruction des graphes

      for( i=0; i<fg.size(); ++i )
	delete fg[i];

      cout << "OK." << endl;
      return 0;
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
    }
}




