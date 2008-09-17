
#ifndef SI_LEARNER_SELECTIVELEARNER_H
#define SI_LEARNER_SELECTIVELEARNER_H


#include <si/learner/constLearner.h>
#include <sys/types.h>
#include <regex.h>


namespace sigraph
{
  /**	Apprend seuelement les modèles vérifiant un certain critère.

  Attribut syntaxique: \c "selective_learner"
  */
  class SelectiveLearner : public ConstLearner
  {
  public:
    SelectiveLearner( bool allowsChildren=true, 
		      const std::string & synt="selective_learner" );
    virtual ~SelectiveLearner();

    virtual void process(LearnParam *lp);
    virtual void process(LearnConstParam *lp);
    bool checkClique( const Clique* cl );

  protected:
    void initRegexp();

  private:
    regex_t	_filter;
    bool		_ready;
  };

}

#endif


