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
#include <si/fold/fgraph.h>
#include <si/fold/frgraph.h>
#include <si/fold/foldReader.h>
#include <si/fold/frgReader.h>
#include <si/fold/frgWriter.h>
#include <cartobase/stream/sstream.h>
#include <cartobase/exception/parse.h>
#include <si/fold/fattrib.h>
#include <si/model/topModel.h>
#include <si/model/topAdaptive.h>
#include <si/model/adaptiveLeaf.h>
#include <si/subadaptive/subAdaptive.h>
#include <aims/getopt/getopt2.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <unistd.h>

using namespace carto;
using namespace aims;
using namespace sigraph;
using namespace std;



int main( int argc, const char** argv )
{
  try
    {
      AimsApplication	app( argc, argv, "no doc at the moment..." );
      string		modelFile;
      string		valueFile;

      app.addOption( modelFile, "-m", "model file" );
      app.addOption( valueFile, "-v", "value file" );

      app.initialize();

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

      MGraph::const_iterator	im, fm=rg.end();
      Model                     *model;
      TopModel                  *tm;
      string model_name;

      string value_name;
      float value;
      ifstream	tf( valueFile.c_str() );
      while ( tf && !tf.eof() )
	{
	  tf >> value_name >> value;
	  cout << value_name << " " << value << endl;
	  
	  for( im=rg.begin(); im!=fm; ++im )
	    {
	      if( (*im)->getProperty( SIA_LABEL, model_name ) )
		{
		  string::size_type f = model_name.find("_left");
		  if( f != string::npos )
		    model_name.erase( model_name.size() - 5, 5 );
		  f = value_name.find(model_name);
		  if( f != string::npos )
		    {
		      cout << model_name << endl;
		      if( (*im)->getProperty( SIA_MODEL, model ) )
			{
			  tm = model->topModel();
			  tm->setWeight(value);
			  TopAdaptive *ta = dynamic_cast<TopAdaptive *>( model );
			  if( ta )
			    {
			      Model *m2 = ta->model();
			      AdaptiveLeaf *al = dynamic_cast<AdaptiveLeaf *>( m2 );
			      if( al )
				{
				  SubAdaptive & sa = al->workEl();
				  sa.setGenErrorRate( 0 );
				  sa.setGenGoodErrorRate(0);
				  sa.setGenBadErrorRate(0);
				}
			    }
			}
		    }
		}
	    }

      	}



      //	sauvegarde
      string		outmodelFile;
      if (argc>3) outmodelFile = string( argv[3] );
      else outmodelFile = string( argv[2] ) + ".arg";

      rg.setProperty( "filename_base",  string( argv[2] ) + ".data" );

      FrgWriter	rw ( outmodelFile );
      
      try
	{
	  rw << rg;
	  cout << "Ecriture FRGraph OK." << endl;
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

      return 0;
      
    }
  catch( user_interruption & )
    {
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
    }
}
