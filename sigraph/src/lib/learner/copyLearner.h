
#ifndef SI_LEARNER_COPYLEARNER_H
#define SI_LEARNER_COPYLEARNER_H


#include <si/learner/constLearner.h>


namespace sigraph
{
  /**	Duplicateur de clique
   */
  class CopyLearner : public ConstLearner
  {
  public:
    CopyLearner();
    virtual ~CopyLearner();

    virtual void process(LearnConstParam *lp);
    virtual void process(LearnParam *lp);

  protected:
    CopyLearner( const std::string & syntax ) : ConstLearner( true, syntax ) {}

  private:
  };

  // inline

  inline void CopyLearner::process(LearnParam *lp)
  {
	  LearnConstParam     lp2(*lp);
	  process(&lp2);
  }

}

#endif


