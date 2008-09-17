
#ifndef SI_LEARNER_EMPTYAVOIDLEARNER_H
#define SI_LEARNER_EMPTYAVOIDLEARNER_H


#include <si/learner/constLearner.h>
#include <sys/types.h>
#include <regex.h>


namespace sigraph
{

  /**	Change la sortie d'apprentissage à 0 pour les cliques qui n'ont pas 
	de noeud significatif non-void
  */
  class EmptyAvoidLearner : public ConstLearner
  {
  public:
    EmptyAvoidLearner( bool allowsChildren=true, 
		       const std::string & synt="empty_avoid_learner" );
    virtual ~EmptyAvoidLearner();

    virtual void process(LearnParam *lp);
    virtual void process(LearnConstParam *lp);
    virtual bool checkClique( const Clique* cl, double & outp );
  };

}

#endif

