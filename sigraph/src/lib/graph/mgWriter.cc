
#include <si/graph/mgWriter.h>
#include <si/model/model.h>
#include <si/model/mWriter.h>
#include <si/domain/domWriter.h>
#include <cartobase/stream/fileutil.h>

using namespace sigraph;
using namespace carto;
using namespace std;


MGWriter::MGWriter( const string & filename, const SyntaxSet & synt )
  : GraphWriter( synt ), _mgFilename( filename )
{
}


MGWriter::~MGWriter()
{
}


void MGWriter::write( const GraphObject & ao )
{
  string		synt = ao.getSyntax();

  if( synt == "model_node" || synt == "model_edge" || synt == "model_graph" )
    parseModel( ao, "model_file", "model" );
  if( synt == "model_graph" )
    parseModel( ao, "default_model_file", "default_model" );
}


void MGWriter::parseModel( const GraphObject & ao, const string & mfile, 
			   const string & mstr )
{
  if( ao.hasProperty( mfile ) && ao.hasProperty( mstr ) )
    {
      string	nm, file = FileUtil::dirname( name() );
      Model	*ad;
      ao.getProperty( mfile, nm );
      ao.getProperty( mstr, ad );

      if( !file.empty() )
        file += FileUtil::separator();
      if( !_filenameBase.empty() )
        file += _filenameBase + FileUtil::separator();
      file += nm;

      // cout << "Writing model " << file << " (from " << name() << " )\n";
      try
        {
          MWriter	mw( file, true );
          mw << *ad;
        }
      catch( exception & e )
        {
          cerr << e.what() << endl << flush;
          throw;
        }
    }

  if( ao.hasProperty( "domain_file" ) 
      && ao.hasProperty( "domain" ) )
    {
      string	nm, file = FileUtil::dirname( name() );
      Domain	*fd;
      ao.getProperty( "domain_file", nm );
      ao.getProperty( "domain", fd );

      if( !file.empty() )
        file += FileUtil::separator();
      if( !_filenameBase.empty() )
        file += _filenameBase + FileUtil::separator();
      file += nm;

      try
        {
          DomWriter	fdw( file, true );
          fdw << *fd;
        }
      catch( exception & e )
        {
          cerr << e.what() << endl << flush;
          throw;
        }
    }
}


