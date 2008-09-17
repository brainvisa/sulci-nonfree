
#ifndef SI_LEARNER_LEARNFACTORY_H
#define SI_LEARNER_LEARNFACTORY_H


#include <graph/tree/tfactory.h>


namespace sigraph
{

  /**	Learner creator
   */
  class LearnFactory : public TreeFactory
  {
  public:
    LearnFactory();
    virtual ~LearnFactory();
    virtual TreeFactory* clone() const;
    virtual Tree* makeTree( const std::string & syntax, 
			    bool allowChildren = true ) const;
  };

}

#endif

