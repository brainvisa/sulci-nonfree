
#include <cstdlib>
#include <si/fold/foldReader.h>
#include <cartobase/exception/parse.h>
#include <iostream>

using namespace carto;
using namespace sigraph;
using namespace std;


int main( int argc, char** argv )
{
  if( argc != 2 )
    {
      cerr << "usage : readFolds filename.arg\n";
      exit( 1 );
    }

  FoldReader	fr( argv[1] );
  FGraph	fg;

  try
    {
      fr >> fg;

      cout << "OK.\nnb nodes : " << fg.order() << "\nnb edges : " 
           << fg.edgesSize() << endl;
    }
    catch( parse_error & e )
      {
	cerr << e.what() << " : " << e.filename() << ", line " << e.line() 
	     << endl;
      exit( 1 );
      }
  catch( exception & e )
    {
      cerr << e.what() << endl;
      exit( 1 );
    }

  return( 0 );
}
