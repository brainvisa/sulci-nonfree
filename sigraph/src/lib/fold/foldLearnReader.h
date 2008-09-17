
#ifndef SI_FOLD_FOLDLEARNREADER_H
#define SI_FOLD_FOLDLEARNREADER_H


#include <si/learner/learnReader.h>


namespace sigraph
{

  class FoldLearnReader : public LearnReader
  {
  public:
    FoldLearnReader( const LearnFactory & lf, const std::string & filename, 
		     const carto::SyntaxSet & as = syntax() );
    FoldLearnReader( const std::string & filename, 
		     const carto::SyntaxSet & as = syntax() );
    virtual ~FoldLearnReader();

  protected:
    static carto::SyntaxSet & syntax();

  private:
  };

}

#endif



