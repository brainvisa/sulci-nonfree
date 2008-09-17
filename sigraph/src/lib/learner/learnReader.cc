
#include <si/learner/learnReader.h>
#include <si/global/global.h>

using namespace sigraph;
using namespace carto;
using namespace std;


SyntaxSet & LearnReader::syntax()
{
  static SyntaxSet	ss( initSyntax( si().basePath() 
					+ "/config/learner.stx" ) );
  return ss;
}

//

LearnReader::LearnReader( const LearnFactory & lf, const string & filename, 
			  const SyntaxSet & as ) 
  : ExoticTreeReader( lf, filename, as )
{
}


LearnReader::LearnReader( const string & filename, 
			  const SyntaxSet & as ) 
  : ExoticTreeReader( LearnFactory(), filename, as )
{
}


LearnReader::~LearnReader()
{
}







