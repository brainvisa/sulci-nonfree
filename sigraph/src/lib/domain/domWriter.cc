

#include <si/domain/domReader.h>
#include <si/domain/domWriter.h>
#include <graph/tree/tree.h>

using namespace sigraph;
using namespace std;


DomWriter::DomWriter( const string & filename, bool makedir )
  : ExoticTreeWriter( filename, DomReader::syntax(), makedir )
{
}


DomWriter::~DomWriter()
{
}


void DomWriter::write( const Domain & fd )
{
  Tree	tr;

  buildTree( tr, fd );

  visitTree( &tr );
}


void DomWriter::buildTree( Tree & tr, const Domain & fd )
{
  fd.buildTree( tr );
}





