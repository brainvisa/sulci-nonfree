
#ifndef SI_LEARNER_STATLEARNER_H
#define SI_LEARNER_STATLEARNER_H

#include <si/learner/terminalLearner.h>


namespace sigraph
{

  /**	Apprend les stats des r�seaux, pas les r�seaux eux-m�mes
   */
  class StatLearner : public TerminalLearner
  {
  public:
    StatLearner( const std::string & synt="stats_learner" );
    virtual ~StatLearner();

    virtual void process(LearnConstParam *lp);
  };

}

#endif


