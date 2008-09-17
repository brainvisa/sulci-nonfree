
#ifndef SI_LEARNER_CLIQUEMEMORIZER_H
#define SI_LEARNER_CLIQUEMEMORIZER_H


#include <si/learner/constLearner.h>


namespace sigraph
{
  /**	Memorizes clique labels and restores them afterwards
   */
  class CliqueMemorizer : public ConstLearner
  {
  public:
    CliqueMemorizer();
    virtual ~CliqueMemorizer();

    virtual void process(LearnConstParam *lp);
    virtual void process(LearnParam *lp);

  protected:
    CliqueMemorizer( const std::string & syntax ) 
      : ConstLearner( true, syntax ) {}

  private:
  };

  // inline

  inline void CliqueMemorizer::process(LearnParam *lp)
  {
    LearnConstParam     *lp2 = new LearnConstParam(*lp);
    process(lp2);
    delete lp2;
  }

}

#endif


