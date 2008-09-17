
#include <aims/getopt/getopt2.h>
#include <si/global/global.h>
#include <aims/def/path.h>
#include <cartobase/stream/fileutil.h>

using namespace sigraph;
using namespace aims;
using namespace carto;
using namespace std;

int main( int argc, const char** argv )
{
  // force using sigraph library (needed on Mac-Darwin)
  si();
  try
    {
      AimsApplication	app( argc, argv, 
			     "Get configuration info about SiGraph library "
			     "and commands" );
      bool	nomenc = false, shared = false, gshared = false;
      app.addOption( nomenc, "--nomenclature", "Print nomenclature path", 
		     true );
      app.addOption( shared, "--sishared", "Print sigraph data/config path", 
		     true );
      app.addOption( gshared, "--shfjshared", "Print global SHFJ shared path", 
		     true );
      app.initialize();

      if( nomenc )
	cout << Path::singleton().nomenclature() << endl;
      if( shared )
	cout << si().basePath() << endl;
      if( gshared )
	cout << FileUtil::dirname( Path::singleton().nomenclature() ) << endl;

      if( !nomenc && !shared && !gshared )
	{
	  cerr << argv[0] << " [--help] [options]\n";
	  exit( EXIT_FAILURE );
	}

      exit( EXIT_SUCCESS );
    }
  catch( user_interruption & )
    {
      exit( EXIT_SUCCESS );
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
    }
  exit( EXIT_FAILURE );
}

