
#include <cstdlib>
#include <si/fold/frgReader.h>
#include <si/fold/frgWriter.h>
#include <si/fold/frgraph.h>
#include <aims/getopt/getopt2.h>
#include <iostream>

using namespace sigraph;
using namespace aims;
using namespace carto;
using namespace std;


void usage( const char* name )
{
  cerr << "usage : " << name << " modelgraph.arg\n\n";
  exit( 1 );
}


int main( int argc, const char** argv )
{
  try
    {
      string	mname;
      AimsApplication	app( argc, argv, "Reverts an overlearned model graph " 
                             "to its optimal state (saved during learning)" );
      app.addOption( mname, "-m", "model graph" );

      app.initialize();

      FRGraph	mg;
      FrgReader	frr( mname );
      frr >> mg;

      mg.closeLearning();

      FrgWriter	frw( mname );
      frw << mg;
    }
  catch( user_interruption & )
    {
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
      exit( 1 );
    }
}


