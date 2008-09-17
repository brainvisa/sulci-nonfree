
#ifndef SI_LEARNER_CONSTLEARNER_H
#define SI_LEARNER_CONSTLEARNER_H


#include <si/learner/learner.h>


namespace sigraph
{
  /**	S�quence d'apprentissage de cliques constantes.

	Classe de base pour les Learner constants, qui sert aussi 
	pour les d�buts d'arborescences � plusieurs enfants d�s le 1er 
	niveau.

	Attribut syntaxique: \c "const_learner"
  */
  class ConstLearner : public Learner
  {
  public:
    ConstLearner( bool allowsChildren=true, 
		  const std::string & synt="const_learner" );
    virtual ~ConstLearner();

    /**	Cette version de \c process() op�rant sur des cliques 
	non-\c const doit �tre red�finie par les classes d�riv�es: on ne 
	peut pas simplement la "brancher" sur la fonction op�rant sur des 
	cliques \c const sinon cela emp�che d'utiliser des enfants ne 
	d�rivant pas de \c ConstLearner.
    */
    virtual void process(LearnParam *lp);
    virtual void process(LearnConstParam *lp);

  protected:

  private:
  };

}

#endif

