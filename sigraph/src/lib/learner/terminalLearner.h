
#ifndef SI_LEARNER_TERMINALLEARNER_H
#define SI_LEARNER_TERMINALLEARNER_H


#include <si/learner/constLearner.h>


namespace sigraph
{

  /**	Terminal element for learning sequence tree
   */
  class TerminalLearner : public ConstLearner
  {
  public:
    TerminalLearner( const std::string & synt="terminal_learner" );
    virtual ~TerminalLearner();

    virtual void process(LearnConstParam *lp);
    virtual void process(LearnParam *lp);
  };

  // inline

  inline void TerminalLearner::process(LearnParam *lp)
  {
	  LearnConstParam     lp2(*lp);
	  process(&lp2);
  }

}

#endif

