
#include <si/fold/foldLearnReader.h>
#include <si/fold/foldLearnFactory.h>
#include <si/global/global.h>

using namespace sigraph;
using namespace carto;
using namespace std;


SyntaxSet & FoldLearnReader::syntax()
{
  static SyntaxSet	as( LearnReader::syntax() );
  return as;
}

//

FoldLearnReader::FoldLearnReader( const LearnFactory & lf, 
				  const string & filename, 
				  const SyntaxSet & as ) 
  : LearnReader( lf, filename, as )
{
}


FoldLearnReader::FoldLearnReader( const string & filename, 
				  const SyntaxSet & as ) 
  : LearnReader( FoldLearnFactory(), filename, as )
{
}


FoldLearnReader::~FoldLearnReader()
{
}







