/*
 *  Copyright (C) 1998-2006 CEA
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
#include <si/finder/modelFinder.h>
#include <si/fold/foldLearnReader.h>
#include <si/learner/selectiveTrainer.h>
#include <si/subadaptive/stopCriterion.h>
#include <cartobase/stream/sstream.h>
#include <cartobase/exception/parse.h>
#include <cartobase/object/sreader.h>
#include <cartobase/plugin/plugin.h>
#include <vector>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#ifndef _WIN32
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#if defined(__linux)
#include <fpu_control.h>
#elif defined(__sun) || defined(__sgi)
#include <ieeefp.h>
#endif
#endif

using namespace carto;
using namespace sigraph;
using namespace std;


FRGraph	rg;	// global pour permettre l'accès dans une interruption
SelectiveTrainer	*strn = 0;
bool	insave = false;


struct Params
{
  string		model;
  string		trainscheme;
  vector<string>	graphs;
  vector<string>	testGraphs;
  string		labelsMap;
  Trainer::TrainerMode	mode;
  int			uninitflg;
  int			nosave;
  int			cycles;
  int			cycles_tst;
  string		atts;
  string		pattern;
  int			verbose;
  int			stopDelay;
  float			maxAppError;
  int			closeLearning;
  string		labelatt;
};


Params	par;


void usage( const char* name )
{
  cerr << "usage : \n" << name << " paramFile\n";
  cerr << name << " [-l labelsMapFile] [-n] [-u] [-s] [-c cycles] " 
       << "modelFile trainschemeFile graphFile1.arg ... graphFileN.arg\n";
  cerr << "\nLearns a model graph\n\n";
  cerr << "paramFile        :  parameters file for inputs for the command \n"
       << "                    (Tree file)\n";
  cerr << "-l labelsMapFile :  correspondance map from labels of the \n" 
       << "                    graph to learn to those used by the model\n" 
       << "                    (default: sillons_modele.def)\n\n";
  cerr << "-n               :  (nosave) do not save the model after learning" 
       << endl;
  cerr << "-u               :  (uninitialized) do not reinitialize the model\n"
       << "                    before learning\n";
  cerr << "-s               :  (stats) initializes learning of stats\n";
  cerr << "-a attr          :  label attribute used to get labels from:\n"
       << "                    usually 'label' or 'name', 'auto' means try \n"
       << "                    first label, and if no label is present, take \n"
       << "                    name [default:auto]'\n";
  cerr << "-c cycles        :  number of learning and testing cycles over the\n"
       << "                    graph set while database generation\n" 
       << "                    (default: 1)\n";
  cerr << "\nmodelFile.arg  :  model graph file to train\n";
  cerr << "trainschemeFile  :  learning sequence description file\n";
  cerr << "graphFile1.arg .. graphFileN.arg    :  graphs to learn\n";
  cerr << "\n\nSignal handlers:\n";
  cerr << "SIGINT (kill -2) (Ctrl-C) : prompt for save, then exits\n";
  cerr << "SIGSEGV, SIGBUS, SIGILL (kill -9,10,11) (error) : save and exit\n";
  cerr << "SIGUSR1 (kill -16) : save and continues learning\n\n";
  cerr << "WARNING -- Commandline-options are not being maintained any\n";
  cerr << "longer. Use config file for new params.\n\n";
  exit( 1 );
}


void paramError( const char* name, const string & file, const string & param  )
{
  cerr << name << " : bad param in file " << file << ", param " << param 
       << endl;
  exit( EXIT_FAILURE );
}


void loadParams( const string & paramFile, const char* name, Params & par )
{
  SyntaxReader	pr( si().basePath() + "/config/siLearn.stx" );
  SyntaxSet		ps;

  try
    {
      pr >> ps;
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
      exit( EXIT_FAILURE );
    }

  TreeReader	tr( paramFile, ps );
  Tree		t;
  string	gf, str, tgf, mode;

  try
    {
      tr >> t;
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
      exit( EXIT_FAILURE );
    }

  if( !t.getProperty( "modelFile", par.model ) )
    paramError( name, paramFile, "modelFile" );
  if( !t.getProperty( "trainschemeFile", par.trainscheme ) )
    paramError( name, paramFile, "trainschemeFile" );
  if( !t.getProperty( "graphFiles", gf ) )
    paramError( name, paramFile, "graphFiles" );
  if( !t.getProperty( "labelsMapFile", par.labelsMap ) )
    par.labelsMap = "";
  t.getProperty( "uninitflg", par.uninitflg );
  t.getProperty( "nosave", par.nosave );
  t.getProperty( "cycles", par.cycles );
  t.getProperty( "cycles_tst", par.cycles_tst);
  t.getProperty( "label_attribute", par.labelatt);
  if ( !t.getProperty( "mode", mode ))
	par.mode = Trainer::GenerateOnly;
  else if (mode == string("generateOnly"))
	par.mode = Trainer::GenerateOnly;
  else if (mode == string("generateAndTrain"))
	par.mode = Trainer::GenerateAndTrain;
  else if (mode == string("readAndTrain"))
	par.mode = Trainer::ReadAndTrain;
  else if (mode == string("trainDomain"))
	par.mode = Trainer::TrainDomain;
  else if (mode == string("trainStats"))
	par.mode = Trainer::TrainStats;
  else {
	cerr << "error: '" << mode << "' is not a valid mode" << endl;
	cerr << "try : generateOnly, generateAndTrain, readAndTrain, trainStats"
	     << endl;
	exit(EXIT_FAILURE);
  }

  t.getProperty( "filter_attributes", par.atts );
  t.getProperty( "filter_pattern", par.pattern );
  t.getProperty( "verbose", par.verbose );
  t.getProperty( "testGraphFiles", tgf );
  t.getProperty( "stop_delay", par.stopDelay );
  t.getProperty( "max_app_error", par.maxAppError );
  t.getProperty( "close_learning", par.closeLearning );

  istringstream	sst( gf.c_str() );

  while( !sst.eof() )
    {
      sst >> str;
      if( str.size() > 0 )
	par.graphs.push_back( str );
    }

  if( par.graphs.size() == 0 )
    paramError( name, paramFile, "graphFiles" );

  if( !tgf.empty() )
    {
      istringstream	tst( tgf.c_str() );

      while( !tst.eof() )
	{
	  tst >> str;
	  if( str.size() > 0 )
	    {
	      par.testGraphs.push_back( str );
	      cout << "test graph : " << str << endl;
	    }
	}
    }

  if (par.cycles < 0 || par.cycles_tst <= 0 || ( par.cycles == 0 && par.testGraphs.empty() ) )
    paramError( name, paramFile, "cycles" );

  cout << "Graphes d'app : " << par.graphs.size() 
       << "\nGraphes de test : " << par.testGraphs.size() << endl;
}


void saveModel()
{
  if( insave )
    {
      cout << "--- Recursive save ---\n(Not done again)" << endl;
      return;
    }

  insave = true;
  if( par.nosave == 0 )
    {
      cout << "Saving model..." << endl;
      FrgWriter	rw( par.model );

      try
	{
	  if( strn )
	    {
	      cout << "(writing parts only)" << endl;
	      strn->save( rw );
	    }
	  else
	    rw << rg;
	  cout << "Ecriture FRGraph OK." << endl;
	}
      catch( exception & e )
	{
	  cerr << e.what() << endl;
	  throw;
	}
    }
  insave = false;
}


void sig_break( int )
{
  cout << "SIGINT : Sauver ? " << flush;
  char	c;
  cin >> c;
  if( c == 'o' || c == 'O' )
    saveModel();
  exit(0);
}


#ifndef _WIN32
void sig_saveAndCont( int )
{
  // remets le signal pour la fois suivante
  signal( SIGUSR1, sig_saveAndCont );
  cout << "--- Saving ... ---" << endl;
  saveModel();
  cout << "--- Continuing... ---" << endl;
}


void sig_term( int )
{
  cerr << "*** Reveived signal SIGTERM ***" << endl;
  cerr << "Saving before stoping..." << endl;
  saveModel();
  exit( EXIT_FAILURE );
}


void sig_crash( int sig )
{
  cerr << "*** CRASH ***" << endl;
  cerr << "Received signal ";
  switch( sig )
    {
    case SIGSEGV:
      cerr << "SIGSEGV, segmentation fault" << endl;
      break;
    case SIGBUS:
      cerr << "SIGBUS, bus error" << endl;
      break;
    case SIGILL:
      cerr << "SIGILL, illegal instruction" << endl;
      break;
    case SIGFPE:
      cerr << "SIGFPE, floating point exception" << endl;
    default:
      cerr << sig << endl;
    }

  saveModel();
  exit( EXIT_FAILURE );
}
#endif // _WIN32


void	graph_select_names_or_labels(const FGraph *fg)
{
	std::string    todel;

	if( par.labelatt.empty() ||  par.labelatt == "auto" ) return;
	if( par.labelatt == "name" )
		todel = "label";
	else	todel = "name";
	
	/* right now, we use brutality (until a better system is done
	in LabelsTranslator) */

	Graph::iterator	iv, ev = fg->end();
	
	for(iv = fg->begin(); iv != ev; ++iv)
		if ((*iv)->hasProperty(todel))
			(*iv)->removeProperty(todel);
}

int main( int argc, char** argv )
{
  PluginLoader::load();

  unsigned		i;
  set<string>		attrs;

  par.uninitflg = 0;
  par.nosave = 0;
  par.mode = Trainer::GenerateAndTrain;
  par.cycles = 0;
  par.cycles_tst = 1;
  par.stopDelay = 2000;
  par.maxAppError = 0.25;
  par.verbose = 1;
  par.closeLearning = false;

  if( argc == 2 && strcmp( argv[1], "-h" ) != 0 && strcmp( argv[1] , "--help" ) != 0 )
    {
      loadParams( string( argv[1] ), argv[0], par );
    }
  else
    {
      int	c, nb;
      bool	errorflg = false;

      while( ( c = getopt( argc, argv, "aunhcl" ) ) != EOF )
	switch( c )
	  {
          case 'a':
            par.labelatt = argv[optind];
            ++optind;
            break;
	  case 'u':
	    par.uninitflg = 1;
	    break;
	  case 'n':
	    par.nosave = 1;
	    break;
	  case 'c':
	    sscanf( argv[optind], "%d", &nb );
	    ++optind;
	    par.cycles = nb;
	    par.cycles_tst = nb;
	    break;
	  case 'l':
	    par.labelsMap = argv[ optind ];
	    ++optind;
	    break;
	  case 's':
            par.mode = Trainer::TrainStats;
	    break;
	  case 'h':
	  case '?':
	    errorflg = true;
	  }
      if( errorflg || par.cycles <= 0 || par.cycles_tst <= 0) usage( argv[0] );

      if( argc-optind >= 3 )
	{
	  par.model = argv[optind];
	  par.trainscheme = argv[optind + 1];
	  for( i=optind+2; (int)i<argc; ++i )
	    {
	      par.graphs.push_back( string( argv[i] ) );
	      //cout << "graphe " << argv[i] << endl;
	    }
	}
      else usage( argv[0] );
    }

  if( par.atts != "" )
    {
      istringstream	sst( par.atts.c_str() );
      string		str;

      while( !sst.eof() )
	{
	  sst >> str;
	  if( str.size() > 0 )
	    attrs.insert( str );
	}
    }


  //	Augmenter le nb de fichier ouvrables

#ifndef _WIN32
  struct rlimit	rlim;

  getrlimit( RLIMIT_NOFILE, &rlim );
  /*if( par.verbose )
    cout << "limite fichiers : " << rlim.rlim_cur << " / " << rlim.rlim_max 
    << endl;*/
  rlim.rlim_cur = rlim.rlim_max;
  setrlimit( RLIMIT_NOFILE, &rlim );
#endif // _WIN32

  //	Exceptions math

#if defined(__linux)
  // function __setcw replaced by _FPU_SETCW macro in recent linux glibc2.1
  /*__setfpucw( __fpu_control & 
    ~(_FPU_MASK_IM | _FPU_MASK_ZM | _FPU_MASK_OM) );*/
  fpu_control_t	fpuval = __fpu_control & 
    ~(_FPU_MASK_IM | _FPU_MASK_ZM | _FPU_MASK_OM);
  _FPU_SETCW( fpuval );
#elif defined(__sun) || defined(__sgi)
  fp_except	mask;
  mask = fpgetmask();
  mask |= FP_X_INV | FP_X_OFL | FP_X_DZ; // | FP_X_UFL;
  fpsetmask( mask );
#endif


  try
    {
      //	Lecture graphe modèle

      FrgReader	rr( par.model );

      try
	{
	  rr >> rg;
	  cout << "Model graph read OK." << endl;
	}
      catch( exception & e )
	{
	  cerr << e.what() << endl;
	  throw;
	}


      //	Lecture de la séquence d'apprentissage

      FoldLearnReader	flr( par.trainscheme );
      Learner		*learn;
      Tree		*tree;

      try
	{
	  tree = flr.read();
	  learn = dynamic_cast<Learner *>( tree );
	  assert( learn );
	  cout << "Learning scheme OK" << endl;
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

      set<CGraph *>			apps, tsts;
      set<CGraph *>::const_iterator	is, fs;
      FGraph				*fg;

      cout << "Reading learn base : " << endl;
      for( i=0; i<par.graphs.size(); ++i )
	{
	  FoldReader	fr( par.graphs[i] );

	  fg = new FGraph;
	  apps.insert( fg );
	  try
	    {
	      fr >> *fg;
	      cout << "Data graph " << par.graphs[i] << " read OK." << endl;

              MGraph::VersionCheck vc = rg.checkCompatibility( *fg );
              if( !vc.ok )
                {
                  cerr << "Warning: model / data graphs version mismatch:" 
                       << endl;
                  cerr << vc.message << endl << endl;
                  cerr << "I will continue but wrong or inaccurate results " 
                       << "can be achieved" << endl << endl;
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
          graph_select_names_or_labels(fg);
	}

      if( !par.testGraphs.empty() )
	cout << "Reading test base :" << endl;
      else
	cout << "(No test base)" << endl;
      for( i=0; i<par.testGraphs.size(); ++i )	// base de test
	{
	  FoldReader	fr( par.testGraphs[i] );

	  fg = new FGraph;
	  tsts.insert( fg );
	  try
	    {
	      fr >> *fg;
	      cout << "Data graph " << par.testGraphs[i] << " read OK." 
                   << endl;

              MGraph::VersionCheck vc = rg.checkCompatibility( *fg );
              if( !vc.ok )
                {
                  cerr << "Warning: model / data graphs version mismatch:" 
                       << endl;
                  cerr << vc.message << endl << endl;
                  cerr << "I will continue but wrong or inaccurate results " 
                       << "can be achieved" << endl << endl;
                }
	    }
	  catch( exception & e )
	    {
	      cerr << e.what() << endl;
	      throw;
	    }
          graph_select_names_or_labels(fg);
	}


      //	Correspondance des labels

      if( par.labelsMap != "" )
	si().setLabelsTranslPath( par.labelsMap );


      //	Préparation des cliques
      cout << "Init cliques..." << endl;
      for( is=apps.begin(), fs=apps.end(); is!=fs; ++is )
	{
	  rg.modelFinder().initCliques( **is, par.verbose, true );
	}
      for( is=tsts.begin(), fs=tsts.end(); is!=fs; ++is )
	{
	  rg.modelFinder().initCliques( **is, par.verbose, true );
	}

      //	Initialisations du modèle

      if( par.uninitflg == 0 )
	rg.initAdap();
      if (par.mode == Trainer::TrainStats)
	rg.initStats();

#ifndef _WIN32
      //	Traitement des interruptions

      void	(*sig_old)(int);
      sig_old = signal( SIGINT, sig_break );
      signal( SIGSEGV, sig_crash );
      signal( SIGILL, sig_crash );
      signal( SIGBUS, sig_crash );
      signal( SIGFPE, sig_crash );
      signal( SIGUSR1, sig_saveAndCont );
      signal( SIGTERM, sig_term );
#endif // _WIN32


      //	Apprentissage

      Trainer		*tr;
      set<CGraph *>	*ts;

      LearnStopCriterion::theCriterion->StopDelay = par.stopDelay;
      LearnStopCriterion::theCriterion->MaxAppError = par.maxAppError;

      if( attrs.size() != 0 && par.pattern != "" )
	{
	  strn = new SelectiveTrainer( rg, learn, par.pattern );
	  tr = strn;
	  strn->setFiltAttributes( attrs );
	}
      else
	tr = new Trainer( rg, learn );

      tr->init(par.mode);
      if( tsts.empty() )
	ts = 0;
      else
	ts = &tsts;

      cout << "App...---> nothing in fact :)" << endl;
      cout << "ne plus utiliser ce programme, siLearn.py est bien mieux" << endl;
      //tr->train(&apps, ts, par.cycles, par.cycles_tst);

      //	Arrêter (fermer les modèles encore en cours d'apprentissage 
      //	et revenir aux derniers mémos)

      if( par.closeLearning )
	{
	  rg.closeLearning();
	}


      //	Sauvegarde du modèle

      saveModel();

      delete tr;
      delete learn;


      //	Destruction des graphes

      for( is=apps.begin(), fs=apps.end(); is!=fs; ++is )
	delete *is;
      for( is=tsts.begin(), fs=tsts.end(); is!=fs; ++is )
	delete *is;

      cout << "OK." << endl;
      return 0;
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
    }
}

