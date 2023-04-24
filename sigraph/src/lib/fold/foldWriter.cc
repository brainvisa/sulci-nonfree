#include <si/fold/foldReader.h>
#include <si/fold/foldWriter.h>
#include <graph/graph/graph.h>
#include <si/fold/fattrib.h>
#include <aims/bucket/bucket_g.h>
#include <aims/def/def_g.h>
#include <aims/io/aimsGraphW.h>
#include <cartobase/config/paths.h>

using namespace sigraph;
using namespace aims;
using namespace carto;
using namespace std;


FoldWriter::FoldWriter( const string & filename )
  : GraphWriter( filename, FoldReader::syntax )
{
  if( FoldReader::syntax.empty() )
  {
    FoldReader::syntax = FoldReader::initSyntax( Paths::findResourceFile(
      "nomenclature/syntax/graph.stx", "aims" ) );
    setSyntax( FoldReader::syntax );
  }
}


FoldWriter::~FoldWriter()
{
}


#if 0
void FoldWriter::write( const GraphObject & ao )
{
  string		nm;
  void			*ptr;

  //cout << "FoldWriter::visitGraphObject\n";

  /*  if( ao->hasProperty( "volPtr" ) )
    {
      if( ao->hasProperty( "name" ) )
	ao->getProperty( "name", nm );
      else nm = "***";
      ao->getProperty( "volPtr", ptr );
      cout << "element " << nm << " , Synt : " << ao->getSyntax() 
	   << " : volume " << ptr << "\n";
      if( !ao->hasProperty( "volume" ) )
	cerr << "Pas de nom de fichier\n";
	}*/

  if( ao.hasProperty( SIA_SS_BUCKET ) )
    {
      AimsBucket<Void>	*bck;

      if( ao.hasProperty( SIA_NAME ) )
	ao.getProperty( SIA_NAME, nm );
      else nm = "***";
      ao.getProperty( SIA_SS_BUCKET, bck );
      /*cout << "element " << nm << " , Synt : " << ao.getSyntax() 
	<< " : bucket " << bck << "\n";*/
      if( !ao.hasProperty( SIA_SS_FILENAME ) )
	cerr << "Pas de nom de fichier\n";
    }

  if( ao.hasProperty( SIA_BOTTOM_BUCKET ) )
    {
      AimsBucket<Void>	*bck;

      if( ao.hasProperty( SIA_NAME ) )
	ao.getProperty( SIA_NAME, nm );
      else nm = "***";
      ao.getProperty( SIA_BOTTOM_BUCKET, bck );
      /*cout << "element " << nm << " , Synt : " << ao.getSyntax() 
	<< " : bucket " << bck << "\n";*/
      if( !ao.hasProperty( SIA_BOTTOM_FILENAME ) )
	cerr << "Pas de nom de fichier\n";
    }

  if( ao.hasProperty( SIA_OLDTRI ) )
    {
      if( ao.hasProperty( SIA_NAME ) )
	ao.getProperty( SIA_NAME, nm );
      else nm = "***";
      ao.getProperty( SIA_OLDTRI, ptr );
      /*cout << "element " << nm << " , Synt : " << ao.getSyntax() 
	<< " : triangulation " << ptr << "\n";*/
      if( !ao.hasProperty( SIA_OLDTRI_FILENAME ) )
	cerr << "Pas de nom de fichier\n";
    }
}
#endif


void FoldWriter::write( const Graph & ao )
{
  cout << "FoldWriter::write( const Graph & )\n";
  write( const_cast<Graph &>( ao ) );
}


void FoldWriter::write( Graph & ao )
{
  cout << "FoldWriter::write " << name() << endl;
  /*bool	subelem = false;
  ao.getProperty( "aims_elements_loaded", subelem );
  if( subelem )
    {
  */
  aims::AimsGraphWriter	gw( name() );
  cout << "writing elements...\n";
  gw.writeElements( ao );
  cout << "elements writen.\n";
  try
  {
    GraphWriter::write( ao );
  }
  catch( exception & e )
  {
    cout << "exception: " << e.what() << endl;
    throw;
  }
  //  }
}


