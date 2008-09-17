

#ifndef SI_MODEL_ADAPTIVE_H
#define SI_MODEL_ADAPTIVE_H


#include <si/model/model.h>
class Tree;


namespace sigraph
{
  class LearnConstParam;
  class Learner;
  /**	Elément adaptatif (classe abstraite).

	Réseau de neurones, combinaison de réseaux, régression, Radial basis 
	function ou n'importe quoi d'autre. Le truc est que ça doit pouvoir 
	apprendre et donner une réponse ainsi qu'une évaluation de sa propre 
	erreur
  */
  class Adaptive : public Model
  {
  public:
    virtual ~Adaptive();
    /// Duplication (fonction abstraite)
    virtual Model* clone() const = 0;

    Adaptive & operator = ( const Adaptive & ad );

    /**@name	Fonctions de base */
    //@{
    /** Apprentissage du modèle sur \p lrn et test avec \p tst
        @param  lrn	Ensemble d'apprentissage
        @param  tst     Ensemble de test
        @return Taux de reconnaissance du modèle appris sur la base de test
    */

    virtual void generateDataBase(Learner &learner, const std::string &prefix,
		const std::list<Clique *> *lrnCliques,
		const std::list<Clique *> *tstCliques,
		int cycles, int cycles_tst) = 0;
    virtual void trainDomain(const std::list<Clique *> &cliques);

    /**	Evaluation (confiance dans la réponse au point donné) 
	(fonction abstraite) */
    virtual double eval( const Clique * ) = 0;
    ///	Taux d'erreur d'apprentissage global
    virtual double errorRate() const = 0;
    ///	Taux d'erreur de généralisation
    virtual double genErrorRate() const = 0;
    virtual double relianceWeight() const = 0;
    ///	Initialisation des parties adaptatives (fonction abstraite)
    virtual void init() { _learnfinished = false; _ndata = 0; }
    /**	Le modèle est-il adaptatif ? Oui. \\
	Cette fonction retourne {\tt true} pour la classe Adaptive et ses 
	dérivées
    */
    bool isAdaptive() const;
    unsigned nbLearnData() const;
    void setNbLearnData( unsigned n );
    ///	regarde juste la variable interne
    virtual bool learnFinished() const { return( _learnfinished ); }
    ///	fonction non-const pcq elle peut positionner le flag
    virtual bool checkLearnFinished() { return( _learnfinished ); }
    void setLearnFinished( bool f ) { _learnfinished = f; }
    /**	fait ce qu'il faut pour "fermer" l'apprentissage (remet le dernier 
	mémo...) */
    virtual void forceLearnFinished() { setLearnFinished( true ); }

    //@}

    /**@name	Statistiques (pour normaliser les entrées) */
    //@{
    ///	Vide toutes les stats (fonction abstraite)
    virtual void resetStats() = 0;
    ///	Ajout à la base de statistiques (fonction abstraite)
    virtual void trainStats(Learner &learner,
				const std::list<Clique *> &cliques) = 0;
    //@}

    ///	Conversion en arbre (pour IO)
    virtual void buildTree( Tree & tr ) const;

  protected:
    Adaptive( Model* parent = 0 );
    Adaptive( const Adaptive & ad );

    ///	Nombre d'exemples appris
    unsigned	_ndata;
    bool	_learnfinished;

  private:

  };


  //	Fonctions

  inline Adaptive::Adaptive( Model* parent ) : Model( parent ), _ndata( 0 ), 
					       _learnfinished( false )
  {
  }


  inline Adaptive::Adaptive( const Adaptive & ad ) : Model( ad ), 
						     _ndata( ad._ndata ), _learnfinished( ad._learnfinished )
  {
  }


  inline Adaptive & Adaptive::operator = ( const Adaptive & ad )
  {
    Model::operator = ( *(Model *) &ad );
    _ndata = ad._ndata;
    _learnfinished = ad._learnfinished;
    return( *this );
  }


  inline bool Adaptive::isAdaptive() const
  {
    return( true );
  }


  inline unsigned Adaptive::nbLearnData() const
  {
    return( _ndata );
  }


  inline void Adaptive::setNbLearnData( unsigned n )
  {
    _ndata = n;
  }

}


#endif



