
#ifndef SI_LEARNER_LEARNREADER_H
#define SI_LEARNER_LEARNREADER_H


#include <si/graph/exoticTreeReader.h>
#include <si/learner/learnFactory.h>
#include <si/learner/learner.h>

namespace sigraph
{

class LearnReader : public ExoticTreeReader
{
public:
  LearnReader( const LearnFactory & lf, const std::string & filename, 
	       const carto::SyntaxSet & as = syntax() );
  LearnReader( const std::string & filename, 
	       const carto::SyntaxSet & as = syntax() );
  virtual ~LearnReader();

protected:
  static carto::SyntaxSet & syntax();

private:
};

}

#endif


