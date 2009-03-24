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

#include <cstdlib>
#include <si/model/mReader.h>
#include <si/fold/fdParser.h>
#include <si/model/adaptiveTree.h>
#include <si/model/adaptiveLeaf.h>
#include <si/model/topAdaptive.h>
#include <si/subadaptive/subAdaptive.h>
#include <si/subadaptive/subAdGauss.h>
#include <si/model/mWriter.h>
#include <cartobase/plugin/plugin.h>
#include <neur/rand/rand.h>
#include <iostream>
#include <unistd.h>
#include <assert.h>
#if defined(__linux)
#include <fpu_control.h>
#elif defined(__sun) || defined(__sgi)
#include <ieeefp.h>
#endif

using namespace sigraph;
using namespace carto;
using namespace std;


int main( int argc, char **argv )
{
  int	c;
  bool	errorflg = false, cont = false, save = false;

  while( ( c = getopt( argc, argv, "csh" ) ) != EOF )
    switch( c )
      {
      case 'c':
	cont = true;
	break;
      case 's':
	save = true;
	break;
      default:
	errorflg = true;
      }

  if( errorflg || argc-optind != 2 )
    {
      cerr << "usage : " << argv[0] << " model.mod ncycles [c(ont)]\n";
      exit( 1 );
    }

  PluginLoader::load();

  string	modelFile = argv[optind];
  unsigned	ncycles;

  sscanf( argv[optind+1], "%d", &ncycles );

  //	Lecture Model
  MReader	ar( modelFile );
  FDParser	fp;
  ar.addFactories( fp.factories() );
  Model		*mod = ar.readModel();
  assert( mod && mod->isAdaptive() );
  Adaptive	*ad = (Adaptive *) mod;

  if( !cont )
    ad->init();

  AdaptiveLeaf			*al;
  AdaptiveTree			*at;
  TopAdaptive			*ta;
  SubAdGauss			*sa = 0;

  while( mod && mod->isAdaptive() )
    {
      al = dynamic_cast<AdaptiveLeaf *>( mod );
      if( al )
	{
	  sa = dynamic_cast<SubAdGauss *>(&al->workEl());
          if (sa) {
		  mod = 0;
		  sa->randinit();
	  }
	}
      else
	{
	  at = dynamic_cast<AdaptiveTree *>( mod );
	  if( at )
	    mod = *at->begin();
	  else
	    {
	      ta = dynamic_cast<TopAdaptive *>( mod );
	      if( ta )
		mod = ta->model();
	      else
		mod = 0;
	    }
	}
    }
  assert( sa );

  unsigned	i, j, k, ni = sa->stats().size();

  assert( ni >= 2 );
  if( save )
    if( !sa->openFile( "sa.dat" ) )
      cout << "pas pu ouvrir le fichier de résultats\n";

  vector<double>	vec;

  for( j=0; j<ni; ++j )
    vec.push_back( 0 );

  //	Exceptions math

#if defined(__linux)
  // function __setfpucw replaced by _FPU_SETCW macro in recent linux glibc2.1
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
  for( i=0; i<ncycles; ++i )
    {
      cout << "cycle " << i << " / " << ncycles << "\r" << flush;

      vec[0] = GaussRand( -1, 0.2 );
      vec[1] = GaussRand( -1, 0.2 );

      for( j=2; j<ni; ++j )
	vec[j] = GaussRand( 0, 0.1 );

      GaussVectorLearnable vl1(vec, 1);
      sa->learn(vl1);

      vec[0] = GaussRand( 1, 0.2 );
      vec[1] = GaussRand( 1, 0.2 );
      for( j=2; j<ni; ++j )
	vec[j] = GaussRand( 0, 0.1 );

      GaussVectorLearnable vl2(vec, -1);
      sa->learn(vl2);

      vec[0] = GaussRand( -0.5, 0.1 );
      vec[1] = GaussRand( -1, 0.1 );
      for( j=2; j<ni; ++j )
	vec[j] = GaussRand( 0, 0.1 );

      GaussVectorLearnable vl3(vec, 0);
      sa->learn(vl3);

      for( k=0; k<20; ++k )
	{
	  for( j=0; j<ni; ++j )
	    vec[j] = ran1() * 10 - 5;

          GaussVectorLearnable vl(vec, 1);
	  sa->learn(vl);
	}
    }
  cout << endl;

  //	Ecriture

  MWriter	mw( modelFile );
  mw << *ad;

  return( 0 );
}
