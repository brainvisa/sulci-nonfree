
#include <si/fold/ffParser.h>
#include <si/fold/foldFinder.h>
#include <si/gyrus/gfparser.h>
#include <si/roi/roifparser.h>
#include <graph/tree/tree.h>

using namespace sigraph;
using namespace carto;
using namespace std;


FFParser::~FFParser()
{
}


TreePostParser::FactorySet FFParser::factories()
{
  FactorySet	fs;

  fs[ "fold_finder"  ] = buildFoldFinder;
  fs[ "gyrus_finder" ] = GFParser::buildGyrusFinder;
  fs[ "roi_finder"   ] = RoiFParser::buildRoiFinder;

  return( fs );
}


void FFParser::buildFoldFinder( AttributedObject* parent, Tree*, 
				const string & )
{
  parent->setProperty( "model_finder_ptr", 
			(ModelFinder *) new FoldFinder( (MGraph &) *parent) );
}



