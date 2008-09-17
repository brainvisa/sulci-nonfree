#include <si/graph/exoticTreeWriter.h>
#include <graph/tree/tree.h>
#include <cartobase/stream/directory.h>
#include <cartobase/stream/fileutil.h>

using namespace sigraph;
using namespace carto;
using namespace std;


ExoticTreeWriter::ExoticTreeWriter( const string & filename, 
				    const SyntaxSet & attr, bool makedir )
  : TreeWriter( attr )
{
  if( makedir )
    {
      string::size_type	pos = 0;
      while( ( pos = filename.find( FileUtil::separator(), pos+1 ) ) != string::npos )
	{
	  string	dir( filename.substr( 0, pos ) );
          Directory	d( dir );
          d.mkdir();
	}
    }
  open( filename );
}


ExoticTreeWriter::~ExoticTreeWriter()
{
}


void ExoticTreeWriter::writeTree( const Tree* tr, const string & type )
{
  parseTree( tr );
  TreeWriter::writeTree( tr, type );
}


void ExoticTreeWriter::parseTree( const Tree* tr )
{
  //Tree::const_iterator	in, fn=tr->end();

  //	attributs globaux

  parse( tr );

  //	obsolete avec le nouveau TreeWriter

  //	sous-éléments
  /*for( in=tr->begin(); in!=fn; ++in )
    parseTree( (Tree *) *in );*/
}
