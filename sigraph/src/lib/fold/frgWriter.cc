

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


// ---------------

LowLevelFRGArgWriter::LowLevelFRGArgWriter()
  : aims::LowLevelArgWriter()
{
}


LowLevelFRGArgWriter::~LowLevelFRGArgWriter()
{
}


void LowLevelFRGArgWriter::write( const string & filename, Graph & graph,
                                  SavingMode, bool )
{
  FrgWriter	w( filename );
  w.write( reinterpret_cast<FRGraph &>( graph ) );
}





