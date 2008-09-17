

#include <si/global/global.h>
#include <si/domain/domReader.h>
#include <si/domain/nullDomain.h>
#include <cartobase/exception/parse.h>
#include <si/graph/attrib.h>
#include <aims/def/path.h>
#include <iostream>

using namespace carto;
using namespace aims;
using namespace sigraph;
using namespace std;


SyntaxSet & DomReader::syntax()
{
  static SyntaxSet	synt( initSyntax( Path::singleton().syntax() 
					  + "/domain.stx" ) );
  return synt;
}

map<string, DomReader::BuildFunc> & DomReader::builders()
{
  static map<string, DomReader::BuildFunc> build;

  if( build.empty() )
    build[ SIA_NULL_DOMAIN ] = &buildNull;

  return build;
}


DomReader::DomReader( const string & filename ) 
  : ExoticTreeReader( filename, DomReader::syntax() )
{
}


DomReader::~DomReader()
{
}


Domain* DomReader::readDom()
{
  Domain	*fd = 0;
  Tree		tree;

  try
    {
      readTree( &tree );
    }
  catch( parse_error & e )
    {
      cerr << e.what() << " - file : " << e.filename() << ", line " 
	   << e.line() << endl;
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
    }

  if( tree.hasProperty( SIA_POINTER ) )
    {
      string	synt = tree.getSyntax();
      if( builders().find( synt ) != builders().end() )
	{
	  tree.getProperty( "pointer", fd );
	}
      else
	{
	  cerr << "Base Domain syntactic attribute not recognized.\n";
	}
    }
  else
    {
      cerr << "No Domain element has been created!\n";
    }

  return( fd );
}


void DomReader::parse( Tree * ao )
{
  string synt = ao->getSyntax();
  //cout << "  Synt : " << synt << endl;

  Tree					*parent = (Tree*) ao->getParent();
  map<string, BuildFunc>::const_iterator	ib = builders().find( synt );

  if( ib != builders().end() )
    (*ib).second( parent, ao );
  else
    {
      cerr << "DomReader::parse : unknown syntactic attribute " 
	   << synt << endl;
    }
}


void DomReader::buildNull( Tree*, Tree* ao )
{
  ao->setProperty( SIA_POINTER, (Domain *) new NullDomain );
}



