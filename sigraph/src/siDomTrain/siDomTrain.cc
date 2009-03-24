/*
 *  Copyright (C) 1998-2004 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 */

#include <cstdlib>
#include <si/global/global.h>
#include <si/fold/fgraph.h>
#include <si/fold/frgraph.h>
#include <si/fold/foldReader.h>
#include <si/fold/frgReader.h>
#include <si/fold/frgWriter.h>
#include <si/learner/trainer.h>
#include <si/fold/foldLearnReader.h>
#include <si/fold/labelsTranslator.h>
#include <si/fold/fattrib.h>
#include <si/domain/adapDomain.h>
#include <si/graph/vertexclique.h>
#include <cartobase/stream/sstream.h>
#include <cartobase/exception/parse.h>
#include <cartobase/object/sreader.h>
#include <cartobase/plugin/plugin.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <unistd.h>
#include <string.h>

using namespace carto;
using namespace sigraph;
using namespace std;


class DGraph : public FGraph
{
public:
  void initCliques( MGraph* rg = 0 );

protected:
};


void DGraph::initCliques( MGraph* mg )
{
  iterator				iv, fv=end();
  VertexClique				*cl;
  map<string, VertexClique *>		mc;
  map<string, VertexClique *>::iterator	im;
  string				label;
  Vertex				*v;
  vector<string>			*pl;
  unsigned				i, n = order(), nc = 0;

  FoldLabelsTranslator	transl( *mg );
  transl.translate( *this, SIA_NAME, SIA_LABEL );

  for( iv=begin(), i=1; iv!=fv; ++iv, ++i )
    {
      cout << "\r" << setw( 5 ) << i << " / " << setw( 5 ) << n << flush;
      v = *iv;
      v->getProperty( SIA_LABEL, label );

      im = mc.find( label );
      if( im == mc.end() )
	{
	  set<Vertex *> smv = mg->getVerticesWith( SIA_LABEL, label );
	  assert( smv.size() <= 1 );
	  if( smv.size() == 1 )
	    {
	      cl = new VertexClique;
	      cl->setProperty( SIA_MODEL_TYPE, (string) SIV_RANDOM_VERTEX );
	      cl->setProperty( SIA_LABEL, label );
	      cl->setProperty( SIA_GRAPH, (Graph *) this );
	      _cliques.insert( cl );
	      mc[ label ] = cl;
	      ++nc;
	    }
	  else cl = 0;	// pas de clique
	}
      else cl = (*im).second;

      if( cl )
	{
	  pl = new vector<string>;
	  pl->push_back( label );
	  v->setProperty( SIA_POSSIBLE_LABELS, pl );

	  cl->addVertex( v );
	}
    }
  cout << endl << nc << " cliques built." << endl;
}


void usage( const char* name )
{
  cerr << "usage : \n" 
       << name << " paramFile\n"
       << name << " [-uncb] modelFile.arg " 
       << "graphFile1.arg ... graphFileN.arg\n";
  cerr << "\nLearning of validity domains of a model graph.\n";
  cerr << "\n-u : (uninitialized) don't initialize domains before learning\n";
  cerr << "-n : (nosave) don't save results\n";
  cerr << "-c : (clean) eliminates afterwards unused model elements\n";
  cerr << "-b : (no buckets) don't load buckets (by default, they are " 
       << "loaded)\n";
  exit( 1 );
}


void paramError( const char* name, const string & file, const string & param  )
{
  cerr << name << " : bad param in file " << file << ", param " << param 
       << endl;
  exit( EXIT_FAILURE );
}


void loadParams( const string & paramFile, const char* name, string & model, 
		 vector<string> & graphs, 
		 int & uninitflg, int & nosave, int & clr, int & loadbck, 
                 string & labelsmap )
{
  SyntaxReader	pr( si().basePath() + "/config/siDomTrain.stx" );
  SyntaxSet	ps;

  pr >> ps;

  TreeReader	tr( paramFile, ps );
  Tree		t;
  string	gf, str;

  tr >> t;

  if( !t.getProperty( "modelFile", model ) )
    paramError( name, paramFile, "modelFile" );
  if( !t.getProperty( "graphFiles", gf ) )
    paramError( name, paramFile, "graphFiles" );
  if( !t.getProperty( "uninitflg", uninitflg ) )
    uninitflg = 0;
  if( !t.getProperty( "nosave", nosave ) )
    nosave = 0;
  if( !t.getProperty( "clear", clr ) )
    clr = 0;
  if( !t.getProperty( "load_buckets", loadbck ) )
    loadbck = 1;
  t.getProperty( "labelsMapFile", labelsmap );

  istringstream	sst( gf.c_str() );

  while( !sst.eof() )
    {
      sst >> str;
      if( str.size() > 0 )
	graphs.push_back( str );
    }

  if( graphs.size() == 0 )
    paramError( name, paramFile, "graphFiles" );
}


int main( int argc, char** argv )
{
  PluginLoader::load();

  string		modelFile, labelsmap;
  vector<string>	graphFiles;
  unsigned		i;
  int			uninitflg = 0, nosave = 0, clr = 0, loadbck = 1;

  if( argc == 2 && strcmp( argv[1], "-h" ) != 0 
      && strcmp( argv[1], "--help" ) != 0 )
    loadParams( string( argv[1] ), argv[1], modelFile, 
		graphFiles, uninitflg, nosave, clr, loadbck, labelsmap );
  else
    {
      int	c;
      bool	errorflg = false;

      while( ( c = getopt( argc, argv, "cunhb" ) ) != EOF )
	switch( c )
	  {
	  case 'u':
	    uninitflg = 1;
	    break;
	  case 'n':
	    nosave = 1;
	    break;
	  case 'c':
	    clr = 1;
	    break;
	  case 'b':
	    loadbck = 0;
	    break;
	  case 'h':
	  case '?':
	    errorflg = true;
	  }
      if( errorflg ) usage( argv[0] );

      if( argc-optind >= 3 )
	{
	  modelFile = argv[optind];
	  for( i=optind+1; (int)i<argc; ++i )
	    {
	      graphFiles.push_back( string( argv[i] ) );
	      //cout << "graphe " << argv[i] << endl;
	    }
	}
      else usage( argv[0] );
    }

  if( !labelsmap.empty() )
    si().setLabelsTranslPath( labelsmap );

  try
    {
      //	Lecture graphe modèle

      FRGraph	rg;
      FrgReader	rr( modelFile );

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


      //	Lecture graphes exemples

      set<CGraph*>::const_iterator	dgi, dgf;
      set<CGraph*>			fg;
      cout << "num of graphs : " << graphFiles.size() << endl;

      for( i=0; i<graphFiles.size(); ++i )
	{
          DGraph	*dg = new DGraph;

	  fg.insert(dg);
	  try
	    {
	      FoldReader	fr( graphFiles[i] );
	      fr >> *dg;
	      cout << "Read FGraph " << graphFiles[i] << " OK." << endl;
	      if( loadbck )
		{
		  dg->loadBuckets( graphFiles[i] );
		  cout << "...buckets read." << endl;
		}
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


      //	Préparation des cliques

      cout << "Init cliques..." << endl;
      
      for(dgi = fg.begin(), dgf = fg.end(); dgi != dgf; ++dgi)
	(dynamic_cast<DGraph *>(*dgi))->initCliques( &rg );

      //	Apprentissage

      Trainer			tr(rg);
      MGraph::const_iterator	im, fm=rg.end();
      bool			needMorePass;
      Domain			*dom;
      AdapDomain		*adom;
      unsigned			pass = 1;

      if( uninitflg == 0 )
	tr.resetDomains();

      tr.setMode(Trainer::TrainDomain);

      do
	{
	  cout << "Learning, pass " << pass << "..." << endl;

	    tr.train(&fg);

	  //	tester s'il faut une autre passe
	  needMorePass = false;
	  for( im=rg.begin(); im!=fm; ++im )
	    {
	      (*im)->getProperty( SIA_DOMAIN, dom );
	      adom = dynamic_cast<AdapDomain *>( dom );
	      if( adom && adom->needsMorePasses() )
		{
		  needMorePass = true;
		  adom->nextPass();
		}
	    }
	  ++pass;
	}
      while( needMorePass );

      cout << "OK." << endl;
      /*char	c;
	cin >> c;*/

      //	Elimination de ce qui ne sert pas

      if( clr )
	{
	  cout << "Pruning useless elements..." << flush;
	  rg.removeUnusedModels( true, modelFile );
	  cout << " OK." << endl;
	}


      //	sauvegarde

      if( !nosave )
	{
	  FrgWriter	rw( modelFile );

	  try
	    {
	      rw << rg;
	      cout << "Wrote FRGraph OK." << endl;
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

      //	effacer les graphes
      set<CGraph*>::iterator	dgei, dgef;

      for(dgei = fg.begin(), dgef = fg.end(); dgei != dgef; ++dgei)
	delete *dgei;

      return( 0 );
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
    }
}


