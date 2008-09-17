
#ifndef SI_LEARNER_BADLEARNER_H
#define SI_LEARNER_BADLEARNER_H


#include <si/learner/constLearner.h>


namespace sigraph
{
  /**	Inverse la sortie d'apprentissage
   */
  class BadLearner : public ConstLearner
  {
  public:
    BadLearner();
    virtual ~BadLearner();
    virtual void process(LearnConstParam *lp);
    virtual void process(LearnParam *lp);

  protected:

  private:
  };

}

#endif

