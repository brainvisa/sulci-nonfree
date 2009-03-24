
#include <cstdlib>
#include <si/fold/frgraph.h>
#include <si/fold/fgraph.h>
#include <si/fold/foldReader.h>
#include <si/fold/frgReader.h>
#include <cartobase/plugin/plugin.h>
#include <iostream>

using namespace sigraph;
using namespace carto;
using namespace std;


void usage( const char* name )
{
  cerr << "usage : " << name << "model.arg folds.arg directory\n";
  exit( 1 );
}


void createTriangDomainFiles( FRGraph & mg, FGraph &, 
			      const string & dir )
{
  mg.createTriangDomainFiles( dir );
}


int main( int argc, char** argv )
{
  if( argc != 4 )
    usage( argv[0] );

  PluginLoader::load();

  FRGraph	mg;
  FGraph	fg;

  try
    {
      FrgReader	fr( argv[1] );
      fr >> mg;
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
      exit( 1 );
    }

  /*try
    {
      FoldReader	fr( argv[2] );
      fr >> fg;
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
      exit( 1 );
      }*/

  createTriangDomainFiles( mg, fg, argv[3] );
}




