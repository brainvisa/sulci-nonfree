
#include <si/fold/foldReader.h>
#include <si/fold/foldWriter.h>
#include <si/fold/fgraph.h>
#include <aims/getopt/getopt2.h>
#include <iostream>

using namespace sigraph;
using namespace aims;
using namespace carto;
using namespace std;


int main( int argc, const char** argv )
{
  AimsApplication	app( argc, argv, "Flips coordinates attributes to " 
                             "appear as the other hemisphere in cortical " 
                             "folds graphs" );
  string	gname, oname;
  app.addOption( gname, "-i", "input graph file" );
  app.addOption( oname, "-o", "output (flipped) graph file", true );

  try
    {
      app.initialize();
      if( oname.empty() )
        oname = gname;

      FGraph	fg;

      Reader<Graph>	fr( gname );
      fr.read( fg );

      fg.flipHemispheres();

      Writer<Graph>	fw( oname );
      fw.write( fg );
      cout << "graph " << oname << " saved\n";
      return EXIT_SUCCESS;
    }
  catch( user_interruption & )
    {
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
    }
  return EXIT_FAILURE;
}


