
#ifndef SI_LEARNER_CONSTLEARNER_H
#define SI_LEARNER_CONSTLEARNER_H


#include <si/learner/learner.h>


namespace sigraph
{
  /**	Séquence d'apprentissage de cliques constantes.

	Classe de base pour les Learner constants, qui sert aussi 
	pour les débuts d'arborescences à plusieurs enfants dès le 1er 
	niveau.

	Attribut syntaxique: \c "const_learner"
  */
  class ConstLearner : public Learner
  {
  public:
    ConstLearner( bool allowsChildren=true, 
		  const std::string & synt="const_learner" );
    virtual ~ConstLearner();

    /**	Cette version de \c process() opérant sur des cliques 
	non-\c const doit être redéfinie par les classes dérivées: on ne 
	peut pas simplement la "brancher" sur la fonction opérant sur des 
	cliques \c const sinon cela empêche d'utiliser des enfants ne 
	dérivant pas de \c ConstLearner.
    */
    virtual void process(LearnParam *lp);
    virtual void process(LearnConstParam *lp);

  protected:

  private:
  };

}

#endif

