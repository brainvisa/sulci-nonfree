

#include <si/fold/frgReader.h>
#include <si/fold/frgWriter.h>

using namespace sigraph;
using namespace std;


FrgWriter::FrgWriter( const string & filename )
  : MGWriter( filename, FrgReader::syntax() )
{
}


FrgWriter::~FrgWriter()
{
}





