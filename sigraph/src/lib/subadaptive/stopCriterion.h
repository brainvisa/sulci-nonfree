
#ifndef SI_SUBADAPTIVE_STOPCRITERION_H
#define SI_SUBADAPTIVE_STOPCRITERION_H


namespace sigraph
{
  class SubAdaptive;

  /**	Critère d'arrêt pour les apprentissages
   */
  class LearnStopCriterion
  {
  public:
    LearnStopCriterion();
    virtual ~LearnStopCriterion() {}

    /**	Teste si on doit arrêter
	@param	sa	subAdaptive qui contient les taux d'erreurs
	@param	num	noumbre d'exemples appris par ce modèle
    */
    virtual bool stops( const SubAdaptive & sa, unsigned num ) const;
    ///	Première phase
    virtual bool stoppable( const SubAdaptive & sa, unsigned num ) const;

    /**	Seuil sur l'erreur d'apprentissage en dessous du quel le modèle passe 
	en mode "stoppable"*/
    double	MaxAppError;
    /**	Délai entre le dernier minimum global d'erreur de généralisation 
	(éventuellement sur les bons exemples seuls, selon le critère) et 
	l'arrêt complet de l'apprentissage (en nombre d'exemples testés) */
    unsigned	StopDelay;
    /* *  Seuil de dispersion de l'erreur locale en dessous duquel il est 
       possible d'arrêter 
       double maxErrorDispersion;
       **   Augmentation maxi de l'erreur par rapport à son minimum global à 
       partir de laquelle on arrête l'apprentissage. Doit être supérieur 
       à maxErrorDispersion pour être sûr que c'est une tendance générale et 
       pas une remontée locale.
       *
       double maxErrorIncrease;
       **   Seuil entre l'erreur max locale et l'erreur min globale en dessous 
       duquel on arrête l'apprentissage *
       double minDispersion;*/

    ///	Pointeur statique
    static LearnStopCriterion *theCriterion;
  protected:

  private:
  };

}

#endif


