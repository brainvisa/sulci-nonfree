
#ifndef SI_LEARNER_LEARNER_H
#define SI_LEARNER_LEARNER_H


#include <graph/tree/tree.h>
#include <si/graph/clique.h>
#include <si/finder/adapFinder.h>

namespace sigraph
{
  /**	S�quence d'apprentissage, classe de base (abstraite).

	Pour fonctionner correctement, le Learner de base d'un arbre 
	de Learners doit avoir dans l'attribut \c "model", un 
	pointeur sur le graphe mod�le (MGraph *).

	En utilisation normale, ceci est effectu� par le Trainer, qui 
	utilise lui-m�me les Learner.
  */
  class Learner : public Tree
  {
  public:
    virtual ~Learner();
    ///	Effectue l'apprentissage, r�cursivement en transmettant aux enfants
    virtual void process(LearnParam *lp) = 0;
    /**	Apprentissage sur une clique constante. 
	Par d�faut (non-const), provoque une erreur. */
    virtual void process(LearnConstParam *lp);
    virtual void getVectors(LearnParam *lp);
    virtual void getVectors(LearnConstParam *lp);

  protected:
    /**	Constructeur prot�g�
	@param	allowsChildren	autorise enfants? (cf BaseTree)
	@param	synt		attribut syntaxique
    */
    Learner( bool allowsChildren, const std::string & synt="" );

  private:
  };


  //	Fonctions inline
inline void	Learner::getVectors(LearnParam *lp)
{
	process(lp);
}

inline void	Learner::getVectors(LearnConstParam *lp)
{
	process(lp);
}

}

#endif


