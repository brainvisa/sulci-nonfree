
#ifndef SI_FOLD_FOLDLEARNFACTORY
#define SI_FOLD_FOLDLEARNFACTORY


#include <si/learner/learnFactory.h>


namespace sigraph
{

  class FoldLearnFactory : public LearnFactory
  {
  public:
    FoldLearnFactory();
    virtual ~FoldLearnFactory();
    virtual TreeFactory* clone() const;

    virtual Tree* makeTree( const std::string & syntax, 
			    bool allowchildren = true ) const;

  protected:

  private:
  };

}

#endif


