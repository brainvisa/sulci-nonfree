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
#include <si/fold/foldWriter.h>
#include <si/fold/frgReader.h>
#include <si/graph/anneal.h>
#include <si/finder/modelFinder.h>
#include <si/graph/attrib.h>
#include <si/fold/annealConnectExtension.h>
#include <si/graph/annealConfigurator.h>
#include <si/graph/multigraph.h>
#include <cartobase/exception/parse.h>
#include <cartobase/plugin/plugin.h>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#if defined(__linux)
#include <fpu_control.h>
#elif defined(__sun) || defined(__sgi)
#include <ieeefp.h>
#endif

using namespace carto;
using namespace sigraph;
using namespace std;


void usage( const char* name )
{
  cerr << "usage : \n" << name << " paramFile\n";
  cerr << name << " [-l labelsMapFile] [-n] [-u] [-t temp] [-m mode]\n" 
       << "[-d rate] [-i tempICM] [-s stopRate] [-g gibbschange] [-v]\n" 
       << "[-N nodesGrouping] [-p plotfile] [-u] [-L label] " 
       << "modelFile graphFile.arg\n";
  cerr << "\nLabels a folds graph according to a model graph\n" 
       << "by simulated annealing\n\n";
  cerr << "Arguments : \n\n";
  cerr << "paramFile        :  parameters file for inputs for the command \n"
       << "                    (Tree file)\n";
  cerr << "-l labelsMapFile :  correspondance map from labels of the \n" 
       << "                    graph to learn to those used by the model\n" 
       << "                    (default: sillons_modele.def)\n";
  cerr << "-n               :  (nosave) do not save the graph after " 
       << "relaxation\n";
  cerr << "-u               :  (uninitialized) do not initialize the fold\n"
       << "                    graph labels before annealing\n";
  cerr << "-t temp          :  initial temperature of the relaxation\n" 
       << "                    (default: 1.)\n";
  cerr << "-m mode          :  annealing mode: metro, icm, gibbs\n" 
       << "                    (default: gibbs)\n";
  cerr << "-d rate          :  temperature decrease rate factor\n" 
       << "                    (default: 0.95)\n";
  cerr << "-i temp          :  temperature below which annealing switches\n" 
       << "                    to ICM (default: 0., no ICM)\n";
  cerr << "-s rate          :  stop rate: proportion of allowed changes\n" 
       << "                    below which relaxation stops (default: 0.01)\n";
  cerr << "-g gibbschange   :  number of synchronous changes allowed in\n" 
       << "                    Gibbs and ICM modes (default: 2)\n";
  cerr << "-N nodesGrouping :  nodes grouping while iterating on the graph:\n" 
       << "                    VERTEX, CLIQUE (default: VERTEX)\n";
  cerr << "-v               :  verbose mode\n";
  cerr << "-p plotfile      :  saves the energies of each timestep in a \n" 
       << "                    file (useful for plotting)\n";
  cerr << "-U               :  use 'unknown' label for initialization. The \n" 
       << "                    'unknown' label is set with the -L option\n";
  cerr << "-L label         :  sets the default unknown label.\n" 
       << "                    (default: 'unknown')\n";
  cerr << "\nmodelFile.arg  :  model graph file to train\n";
  cerr << "graphFile.arg    :  graph to label\n";
  exit( 1 );
}


void paramError( const char* name, const string & file, const string & param  )
{
  cerr << name << " : bad param in file " << file << ", param " << param 
       << endl;
  exit( 1 );
}


int main( int argc, char** argv )
{
  cout << "siRelax\n";
  PluginLoader::load();

  AnnealConfigurator		par;

  if( argc == 2 && strcmp( argv[1], "-h" ) != 0 && strcmp( argv[1], "--help" ) != 0 )
    {
      par.loadConfig( argv[1] );
    }
  else
    {
      int	c, nbi;
      bool	errorflg = false;
      float	nbf;

      par.init();

      while( ( c = getopt( argc, argv, "ungiltmdsvphNUL" ) ) != EOF )
	switch( c )
	  {
	  case 'u':
	    par.initMode = 0;
	    break;
	  case 'n':
	    par.save = 0;
	    break;
	  case 'l':
	    par.labelsMapFile = optarg;
	    ++optind;
	    break;
	  case 't':
	    sscanf( argv[optind], "%f", &nbf );
	    ++optind;
	    par.temp = nbf;
	    break;
	  case 'm':
	    par.mode = argv[optind];
	    ++optind;
	    break;
	  case 'd':
	    sscanf( argv[optind], "%f", &nbf );
	    ++optind;
	    par.rate = nbf;
	    break;
	  case 's':
	    sscanf( argv[optind], "%f", &nbf );
	    ++optind;
	    par.stopRate = nbf;
	    break;
	  case 'i':
	    sscanf( argv[optind], "%f", &nbf );
	    ++optind;
	    par.tempICM = nbf;
	    break;
	  case 'g':
	    sscanf( argv[optind], "%d", &nbi );
	    ++optind;
	    par.gibbsChange = nbi;
	    break;
	  case 'N':
	    par.iterType = argv[optind];
	    ++optind;
	    break;
	  case 'v':
	    par.verbose = 1;
	    break;
	  case 'p':
	    par.plotFile = optarg;
	    ++optind;
	    break;
	  case 'U':
	    par.initLabelTypeString = "VOID";
	    break;
	  case 'L':
	    par.voidLabel = optarg;
	    ++optind;
	    break;
	  case 'h':
	  case '?':
	    errorflg = true;
	  }
      if( errorflg ) usage( argv[0] );

      if( argc-optind == 2 )
	{
	  par.modelFile = argv[optind];
	  par.graphFile = argv[optind + 1];
	}
      else usage( argv[0] );
      par.processParams();
    }

  FRGraph	rg;
  FGraph	fg;

  par.loadGraphs( rg, fg );

  MGraph::VersionCheck vc = rg.checkCompatibility( fg );
  if( !vc.ok )
    {
      cerr << "Warning: model / data graphs version mismatch:" << endl;
      cerr << vc.message << endl << endl;
      cerr << "I will continue but wrong or inaccurate results can be achieved"
           << endl << endl;
    }

  try
    {
      //	Exceptions math

#if defined(__linux)
      // function __setfpucw replaced by _FPU_SETCW macro in recent 
      // linux glibc2.1
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


      //	Recuit

      ofstream	*plotf = 0;
      Anneal	an( fg, rg );

      if( !par.plotFile.empty() )
	plotf = new ofstream( par.plotFile.c_str() );
      if( plotf && !*plotf )
	{
	  cerr << "Cannot open plot file " << par.plotFile << endl;
	  delete plotf;
	  plotf = 0;
	}
      par.initAnneal( an, plotf );

      cout << "Fit...\n";
      an.fit();
      if( plotf )
	plotf->close();
      delete plotf;


      //	Sauvegarde

      if( par.save )
	{
	  if( par.output.empty() )
	    par.output = par.graphFile;

          if( par.outputs.size() == 1 )
            {
              FoldWriter	fr( par.output );

              fr << fg;
            }
          else
            {
              unsigned	i, n = par.graphFiles.size();
              vector<Graph *>	vg( 1 );

              for( i=0; i<n; ++i )
                {
                  try
                    {
                      FoldReader	fr( par.graphFiles[i] );
                      FGraph	tmpfg;
                      fr >> tmpfg;

                      vg[0] = &tmpfg;
                      recoverIndividualGraph( vg, fg, "index" ); // warning...
                      FoldWriter	fw( par.outputs[i] );
                      fw << tmpfg;
                    }
                  catch( parse_error & e )
                    {
                      cerr << e.what() << " : " << e.filename() << ", line " 
                           << e.line() << endl;
                    }
                  catch( exception & e )
                    {
                      cerr << e.what() << endl;
                    }
                }
            }
	}


      //	Affichage des potentiels des cliques

      set<Clique *>::const_iterator	ic, fc=fg.cliques().end();
      string				label1, label2;
      double				pot, pot2;
      ModelFinder			& mf = rg.modelFinder();
      vector<double>			vec;
      Clique				*cl;
      //unsigned				i;

      cout << "potentiels des cliques en fin de recuit :\n";
      for( ic=fg.cliques().begin(); ic!=fc; ++ic )
	{
	  cl = *ic;
	  cout << cl->getSyntax() << "\t";
	  if( cl->getProperty( "label", label1 ) )
	    cout << label1 << "\t\t\t";
	  else if( cl->getProperty( "label1", label1 ) 
		   && cl->getProperty( "label2", label2 ) )
	    cout << label1 << " - " << label2 << "\t";
	  if( cl->getProperty( "potential", pot ) )
	    {
	      pot2 = mf.printDescription( cl );
	      cout << pot << " (" << pot2 << ")" << endl;
	      /*if( cl->getProperty( SIA_POT_VECTOR, vec ) )
		{
		  cout << "vector : ";
		  for( i=0; i<vec.size(); ++i )
		    cout << vec[i] << "  ";
		  cout << endl;
		}
	      if( cl->getProperty( SIA_POT_VECTOR_NORM, vec ) )
		{
		  cout << "nvect  : ";
		  for( i=0; i<vec.size(); ++i )
		    cout << vec[i] << "  ";
		  cout << endl;
		  }*/
	    }
	  else
	    cout << "--no pot--\n";
	}

      cout << "OK.\n";
      return( 0 );
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
    }
}
