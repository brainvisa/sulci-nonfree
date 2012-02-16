/*
 *  Copyright (C) 1998-2005 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 */

#include <si/graph/annealConfigurator.h>
#include <si/global/global.h>
#include <si/fold/frgReader.h>
#include <si/fold/foldReader.h>
#include <si/fold/annealConnectExtension.h>
#include <si/fold/annealConnectVoidExtension.h>
#include <si/finder/modelFinder.h>
#include <si/graph/multigraph.h>
#include <si/graph/attrib.h>
#include <cartobase/object/sreader.h>
#include <graph/tree/treader.h>
#include <graph/tree/twriter.h>
#include <graph/tree/tree.h>
#include <cartobase/exception/ioexcept.h>
#include <cartobase/stream/sstream.h>
#include <iostream>

using namespace sigraph;
using namespace carto;
using namespace std;

AnnealConfigurator::AnnealConfigurator()
{
}


AnnealConfigurator::~AnnealConfigurator()
{
}


void AnnealConfigurator::init()
{
  modelFile.erase( modelFile.begin(), modelFile.end() );
  graphFile.erase( graphFile.begin(), graphFile.end() );
  output = "";
  labelsMapFile.erase( labelsMapFile.begin(), labelsMapFile.end() );
  save = 1;
  initMode = 1;
  temp = 50.;
  mode = "gibbs";
  rate = 0.98;
  tempICM = 0;
  stopRate = 0.05;
  gibbsChange = 1;
  verbose = 1;
  iterType = "VERTEX";
  bmode = Anneal::GIBBS;
  bItType = Anneal::VERTEX;
  setWeights = -1;
  removeVoid = 1;
  initLabelTypeString = "VOID";
  initLabelType = Anneal::INITLABELS_VOID;
  voidLabel = "unknown";
  voidMode = "REGULAR";
  voidOccurency = 10;
  bvoidmode = Anneal::VOIDMODE_REGULAR;
  extensionMode = ""; //"CONNECT_VOID CONNECT";
  extPassOccurency = 10;
  doubleDrawingLots = 0;
  niterBelowStopProp = 1;
  extModes.erase( extModes.begin(), extModes.end() );
  plotFile.erase();
  allowThreads = 0;
  maxIterations = 0;
  mpmUnrecordedIterations = 0;
  forbidVoidLabel = 0;
}


bool AnnealConfigurator::loadConfig( const string & filename )
{
  SyntaxReader	pr( si().basePath() + "/config/siRelax.stx" );
  carto::SyntaxSet		ps;

  pr.read( ps );

  TreeReader	tr( filename, ps );
  Tree		t;
  string	gf, str;

  tr >> t;

  init();

  t.getProperty( "modelFile", modelFile );
  t.getProperty( "graphFile", graphFile );
  if( !t.getProperty( "output", output ) )
    output = graphFile;
  t.getProperty( "labelsMapFile", labelsMapFile );
  t.getProperty( "initflg", initMode );
  if( t.hasProperty( "initflg" ) )
    cerr << "Warning: the obsolete and unused 'initflg' config parameter has "
      "been used, it may not be doing what you expect...\n";
  t.getProperty( "save", save );
  t.getProperty( "temp", temp );
  t.getProperty( "mode", mode );
  t.getProperty( "rate", rate );
  t.getProperty( "tempICM", tempICM );
  t.getProperty( "stopRate", stopRate );
  t.getProperty( "gibbsChange", gibbsChange );
  if( gibbsChange < 1 )
    gibbsChange = 1;
  t.getProperty( "verbose", verbose );
  t.getProperty( "grouping_mode", iterType );
  t.getProperty( "set_weights", setWeights );
  if( !t.getProperty( "remove_void", removeVoid ) )
    t.getProperty( "remove_brain", removeVoid );
  t.getProperty( "plotfile", plotFile );
  t.getProperty( "initLabelsType", initLabelTypeString );
  t.getProperty( "voidLabel", voidLabel );
  t.getProperty( "voidMode", voidMode );
  t.getProperty( "voidOccurency", voidOccurency );
  t.getProperty( "extensionMode", extensionMode );
  t.getProperty( "extensionPassOccurency", extPassOccurency );
  t.getProperty( "doubleDrawingLots", doubleDrawingLots );
  t.getProperty( "niterBelowStopProp", niterBelowStopProp );
  t.getProperty( "allowThreads", allowThreads );
  t.getProperty( "maxIterations", maxIterations );
  t.getProperty( "MPMUnrecordedIterations", mpmUnrecordedIterations );
  t.getProperty( "forbidVoidLabel", forbidVoidLabel );

  return processParams();
}


void AnnealConfigurator::saveConfig( const string & filename )
{
  SyntaxReader	pr( si().basePath() + "/config/siRelax.stx" );
  carto::SyntaxSet		ps;

  pr.read( ps );

  TreeWriter	tw( filename, ps );
  Tree		t( true, "siRelax" );

  t.setProperty( "modelFile", modelFile );
  t.setProperty( "graphFile", graphFile );
  if( !output.empty() )
    t.setProperty( "output", output );
  if( !labelsMapFile.empty() )
    t.setProperty( "labelsMapFile", labelsMapFile );
  if( initMode == 0 )
    t.setProperty( "initflg", initMode );
  if( save == 0 )
    t.setProperty( "save", save );
  t.setProperty( "temp", temp );
  if( mode != "gibbs" )
    t.setProperty( "mode", mode );
  t.setProperty( "rate", rate );
  if( tempICM != 0 )
    t.setProperty( "tempICM", tempICM );
  if( stopRate != 0.01 )
    t.setProperty( "stopRate", stopRate );
  if( gibbsChange != 1 )
    t.setProperty( "gibbsChange", gibbsChange );
  if( verbose == 0 )
    t.setProperty( "verbose", verbose );
  if( iterType != "VERTEX" )
    t.setProperty( "grouping_mode", iterType );
  if( setWeights != 0 )
    t.setProperty( "set_weights", setWeights );
  if( removeVoid != 0 )
    t.setProperty( "remove_void", removeVoid );
  if( !plotFile.empty() )
    t.setProperty( "plotfile", plotFile );
  if( initLabelTypeString != "RANDOM" )
    t.setProperty( "initLabelsType", initLabelTypeString );
  if( voidLabel != "unknown" )
    t.setProperty( "voidLabel", voidLabel );
  if( voidMode != "NONE" )
    t.setProperty( "voidMode", voidMode );
  if( voidOccurency != 0 )
    t.setProperty( "voidOccurency", voidOccurency );
  if( !extensionMode.empty() )
    t.setProperty( "extensionMode", extensionMode );
  if( extPassOccurency != 20 )
    t.setProperty( "extensionPassOccurency", extPassOccurency );
  if( doubleDrawingLots != 0 )
    t.setProperty( "doubleDrawingLots", doubleDrawingLots );
  if( niterBelowStopProp != 1 )
    t.setProperty( "niterBelowStopProp", niterBelowStopProp );
  if( allowThreads != 0 )
    t.setProperty( "allowThreads", allowThreads );
  if( maxIterations != 0 )
    t.setProperty( "maxIterations", maxIterations );
  if( mpmUnrecordedIterations != 0 )
    t.setProperty( "MPMUnrecordedIterations", mpmUnrecordedIterations );
  if( forbidVoidLabel != 0 )
    t.setProperty( "forbidVoidLabel", forbidVoidLabel );

  tw << t;
}


bool AnnealConfigurator::processParams()
{
  if( mode == "icm" )
    bmode = Anneal::ICM;
  else if( mode == "metro" )
    bmode = Anneal::METROPOLIS;
  else if( mode == "gibbs" )
    bmode = Anneal::GIBBS;
  else if( mode == "mpm" )
    bmode = Anneal::MPM;
  else
    {
      cerr << "Unknown annealing mode." << endl;
      return false;
    }
  if( voidMode == "NONE" )
    bvoidmode = Anneal::VOIDMODE_NONE;
  else if( voidMode == "REGULAR" )
    bvoidmode = Anneal::VOIDMODE_REGULAR;
  else if( voidMode == "STOCHASTIC" )
    bvoidmode = Anneal::VOIDMODE_STOCHASTIC;
  else
    {
      cerr << "Unknown void mode." << endl;
      return false;
    }

  if( iterType == "VERTEX" )
    bItType = Anneal::VERTEX;
  else if( iterType == "CLIQUE" )
    bItType = Anneal::CLIQUE;
  else
    {
      cerr << "Unknown nodes grouping mode." << endl;
      return false;
    }

  if( initLabelTypeString == "VOID" )
    initLabelType = Anneal::INITLABELS_VOID;
  else if( initLabelTypeString == "NONE" )
    initLabelType = Anneal::INITLABELS_NONE;
  else
    initLabelType = Anneal::INITLABELS_RANDOM;

  istringstream	sst( extensionMode.c_str() );
  string	str;
  extModes.clear();

  while( !sst.eof() )
    {
      sst >> str;
      if( str.size() > 0 )
	extModes.push_back( str );
    }

  return true;
}


void AnnealConfigurator::loadGraphs( MGraph & rg, CGraph & fg )
{
  //	Lecture graphe modèle

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
  catch( ... )
    {
      throw;
    }

  // void label

  rg.getProperty( SIA_VOID_LABEL, voidLabel );


  //	Lecture graphe exemple

  string::size_type pos = graphFile.find( '|' );
  string::size_type pos2 = 0;

  while( pos != string::npos )
    {
      graphFiles.push_back( graphFile.substr( pos2, pos - pos2 ) );
      pos2 = pos + 1;
      pos = graphFile.find( '|', pos2 );
    }
  graphFiles.push_back( graphFile.substr( pos2, graphFile.length() - pos2 ) );

  if( output.empty() )
    output = graphFile;

  pos = output.find( '|' ), pos2 = 0;

  while( pos != string::npos )
    {
      outputs.push_back( output.substr( pos2, pos - pos2 ) );
      pos2 = pos + 1;
      pos = output.find( '|', pos2 );
    }
  outputs.push_back( output.substr( pos2, output.length() - pos2 ) );

  if( graphFiles.size() != outputs.size() )
    {
      throw runtime_error( "Input and output graphs numbers differ" );
    }

  if( graphFiles.size() == 1 )
    {
      FoldReader	fr( graphFile );

      try
        {
          cout << "Lecture du graphe sillons " << graphFile << endl;
          fr >> fg;
          cout << "Lecture FGraph OK." << endl;
        }
      catch( parse_error & e )
        {
          cerr << e.what() << " : " << e.filename() << ", line " << e.line() 
               << endl;
          throw;
        }
      catch( exception & e )
        {
          cerr << graphFile << ": " << e.what() << endl;
          throw;
        }
    }
  else
    {
      vector<string>::iterator	i, e = graphFiles.end();
      vector<Graph *>	vg(1);

      for( i=graphFiles.begin(); i!=e; ++i )
        {
          FoldReader	fr( *i );

          try
            {
              cout << "Lecture du graphe " << *i << endl;
              FGraph	graph;
              fr >> graph;
              cout << "Lecture FGraph OK.\n";

              vg[0] = &graph;
              concatenateGraphs( vg, fg, "sujet" ); // -----*****
            }
          catch( parse_error & e )
            {
              cerr << e.what() << " : " << e.filename() << ", line " 
                   << e.line() << endl;
              throw;
            }
          catch( exception & e )
            {
              cerr << *i << ": " << e.what() << endl;
              throw;
            }
        }
    }

  //	Enlevage des relations avec unknown

  if( removeVoid )
    rg.removeEdgesToVoid();

  //	Fixation des poids des modèles

  cout << "setWeights : " << setWeights << endl;
  if( setWeights > 0 )
    rg.setWeights( setWeights );
  else if( setWeights < 0 )
    {
      cout << "remove weights" << endl;
      rg.removeWeights();
    }

  //	Préparation des cliques

  cout << "Init cliques..." << endl;

  if( labelsMapFile != "" )
    si().setLabelsTranslPath( labelsMapFile );
  rg.modelFinder().initCliques( fg );

  /* forbidVoidLabel option forbids to use the "unknown" label.
     It is useful only in very specific situations.
   */
  if( forbidVoidLabel )
  {
    cout << "Forbid label " << voidLabel << "." << endl;
    vector<string> *pl = 0;
    Graph::iterator iv, ev = fg.end();
    string label;
    for( iv=fg.begin(); iv!=ev; ++iv )
      if( (*iv)->getProperty( "possible_labels", pl ) )
      {
        vector<string>::iterator ip = find( pl->begin(), pl->end(),
                                            voidLabel );
        if( ip != pl->end() )
        {
          pl->erase( ip );
          // remove the unknown label on the vertex label right now.
          if( (*iv)->getProperty( SIA_LABEL, label ) && label == voidLabel
            && !pl->empty() )
            (*iv)->setProperty( SIA_LABEL, *pl->begin() );
        }
      }
  }
}


void AnnealConfigurator::initAnneal( Anneal & an, ofstream *plotf ) const
{
  cout << "Init Anneal..." << endl;
  an.init( bmode, temp, rate, tempICM, stopRate, gibbsChange, verbose, 
 	   bItType, initLabelType, voidLabel, plotf, 
           (unsigned) niterBelowStopProp );
  an.setVoidMode( bvoidmode, voidOccurency );
  an.setDoubleDrawingLots( doubleDrawingLots );
  an.setMaxIterations( maxIterations );
  an.setMPMUnrecordedIterations( mpmUnrecordedIterations );

  unsigned		i;
  AnnealExtension	*ae = 0, *nae;

  an.deleteExtensions();

  for( i=0; i<extModes.size(); ++i )
    {
      nae = 0;
      if( extModes[i] == "CONNECT" )
	{
	  nae = new AnnealConnectExtension( &an );
	  cout << "CONNECT anneal extension\n";
	}
      else if( extModes[i] == "CONNECT_VOID" )
	{
	  nae = new AnnealConnectVoidExtension( &an );
	  cout << "CONNECT_VOID anneal extension\n";
	}

      if( nae )
	{
	  an.addExtension( nae, extPassOccurency );
	  ae = nae;
	}
    }
  if( ae )
    cout << "Anneal extensions occurency = " 
	 << extPassOccurency << endl;
  else if( !extModes.empty() )
    cerr << "Unknown extension mode(s) " << extensionMode << endl;

  an.setAllowThreads( (bool) allowThreads );
}
