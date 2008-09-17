
#ifndef SI_SUBADAPTIVE_STOPCRITERION_H
#define SI_SUBADAPTIVE_STOPCRITERION_H


namespace sigraph
{
  class SubAdaptive;

  /**	Crit�re d'arr�t pour les apprentissages
   */
  class LearnStopCriterion
  {
  public:
    LearnStopCriterion();
    virtual ~LearnStopCriterion() {}

    /**	Teste si on doit arr�ter
	@param	sa	subAdaptive qui contient les taux d'erreurs
	@param	num	noumbre d'exemples appris par ce mod�le
    */
    virtual bool stops( const SubAdaptive & sa, unsigned num ) const;
    ///	Premi�re phase
    virtual bool stoppable( const SubAdaptive & sa, unsigned num ) const;

    /**	Seuil sur l'erreur d'apprentissage en dessous du quel le mod�le passe 
	en mode "stoppable"*/
    double	MaxAppError;
    /**	D�lai entre le dernier minimum global d'erreur de g�n�ralisation 
	(�ventuellement sur les bons exemples seuls, selon le crit�re) et 
	l'arr�t complet de l'apprentissage (en nombre d'exemples test�s) */
    unsigned	StopDelay;
    /* *  Seuil de dispersion de l'erreur locale en dessous duquel il est 
       possible d'arr�ter 
       double maxErrorDispersion;
       **   Augmentation maxi de l'erreur par rapport � son minimum global � 
       partir de laquelle on arr�te l'apprentissage. Doit �tre sup�rieur 
       � maxErrorDispersion pour �tre s�r que c'est une tendance g�n�rale et 
       pas une remont�e locale.
       *
       double maxErrorIncrease;
       **   Seuil entre l'erreur max locale et l'erreur min globale en dessous 
       duquel on arr�te l'apprentissage *
       double minDispersion;*/

    ///	Pointeur statique
    static LearnStopCriterion *theCriterion;
  protected:

  private:
  };

}

#endif


