
#ifndef NEUR_GAUSS_GAUSSNET_H
#define NEUR_GAUSS_GAUSSNET_H


#include <neur/gauss/gaussian.h>
#include <vector>


/**	Réseau de gaussiennes, en 2 couches, avec 1 sortie.
	Ref. Bishop 95
*/
class GaussNet
{
public:
  /**@name	Types */
  //@{
  /**	Type de fonction d'apprentissage
	@param	vec	exemple d'apprentissage
	@param	d	sortie désirée
	@param	etaW	coef. du gradient pour modifs sur les poids
	@param	etaC	coef. du gradient pour modifs sur les centres
	@param	etaS	coef. du gradient pour modifs sur les écarts-types
	@return	sortie du réseau AVANT apprentissage
  */
  typedef double (GaussNet::*LearnFunc)( const std::vector<double> & vec, 
					 double d, 
					 double etaW, double etaC, 
					 double etaS);
  /**	Type de fonction d'initialisation aléatoire des paramètres du réseau
	@param	cmax	max en valeur absolue des coordonnées des centres
	@param	smax	max des écarts-types
	@param	wmax	max (V.A) des poids entre les gaussiennes et la sortie
  */
  typedef void (GaussNet::*RandInitFunc)( double cmax, double smin, 
					  double smax, double wmax );
  //@}

  /**@name	Statique */
  //@{
  ///	Nombre de passes entre changements de type d'apprentissage
  static unsigned	CyclePeriod;
  //@}

  /**@name	Construction - destruction */
  //@{
  ///
  GaussNet();
  ///	Copie
  GaussNet( const GaussNet & gn );
  ///
  virtual ~GaussNet();
  //@}

  /**@name	Opérateurs */
  //@{
  ///
  GaussNet & operator = ( const GaussNet & gn );
  //@}

  /**@name	Initialisation */
  //@{
  ///	Efface tout
  virtual void clear();
  ///	Crée le réseau avec ninp entrées et ngauss gaussiennes
  virtual void init( unsigned ninp, unsigned ngauss, bool samesigma = true );
  ///	Fonction d'initialisation
  RandInitFunc	randInit;
  /**	Initialise avec des valeurs aléatoires.
	@param	cmax	max en valeur absolue des coordonnées des centres
	@param	smax	max des écarts-types
	@param	wmax	max (V.A) des poids entre les gaussiennes et la sortie
  */
  void randInitAll( double cmax = 1, double smin = 0.3, 
		    double smax = 5, double wmax = 1 );
  ///	Initialise seulement les poids
  void randInitWeights( double cmax = 0, double smin = 0.3, 
			double smax = 5, double wmax = 1 );
  ///	Initialise seulement les centres
  void randInitCenters( double cmax = 0, double smin = 0.3, 
			double smax = 5, double wmax = 1 );
  ///	Initialise seulement les écarts-types
  void randInitSigma( double cmax = 1, double smin = 0.5, 
		      double smax = 5, double wmax = 1 );
  ///	Initialise les poids et les écarts-types
  void randInitWtSig( double cmax = 1, double smin = 0.5, 
		      double smax = 5, double wmax = 1 );
  //@}

  /**@name	Fonctionnement */
  //@{
  unsigned nInputs() const { return( _ninputs ); }
  unsigned nGauss() const;
  const Gaussian* gauss( unsigned n ) const;
  Gaussian* gauss( unsigned n );
  double weight( unsigned n ) const;
  void setWeight( unsigned n, double w );
  template<class InputIterator> 
  void setCenter( unsigned n, const InputIterator & start );
  Gaussian* addGaussian();
  void removeGaussian( unsigned num );
  template<class InputIterator> 
  double prop( const InputIterator & start ) const;
  ///	Fonction d'apprentissage
  LearnFunc	learn;
  /**	Apprentissage par gradient
	@param	vec	exemple d'apprentissage
	@param	d	sortie désirée
	@param	eta	coef. du gradient
	@return	sortie du réseau AVANT apprentissage
  */
  double learnAll( const std::vector<double> & vec, double d, double etaW, 
		   double etaC = 0, double etaS = 0 );
  ///	Apprentissage des coefs seuls
  double learnWeights( const std::vector<double> & vec, double d, double etaW, 
		       double etaC = 0, double etaS = 0 );
  ///	Apprentissage des centres seuls
  double learnCenters( const std::vector<double> & vec, double d, double etaW, 
		       double etaC = 0, double etaS = 0 );
  ///	Apprentissage des écarts-types seuls
  double learnSigma( const std::vector<double> & vec, double d, double etaW, 
		     double etaC = 0, double etaS = 0 );
  ///	Apprentissage des coefs et des écarts-types
  double learnWtSig( const std::vector<double> & vec, double d, double etaW, 
		     double etaC = 0, double etaS = 0 );
  ///	Aprentissage avec modifs multiplicatives sur les sigmas
  double learnMulSig( const std::vector<double> & vec, double d, double etaW, 
		      double etaC = 0, double etaS = 0 );
  ///	Apprentissage alterné Wt - Center - Sigma
  double learnCycle( const std::vector<double> & vec, double d, double etaW, 
		     double etaC = 0, double etaS = 0 );
  //@}

protected:

private:
  struct Private;
  Private	*d;
  ///	Nombre d'entrées
  unsigned	_ninputs;
};


//	inline


template<class InputIterator>
inline double GaussNet::prop( const InputIterator & start ) const
{
  unsigned	i, n = nGauss();
  double	sum = 0;

  for( i=0; i<n; ++i )
    sum += gauss( i )->value( start ) * weight( i );

  return( sum );
}


template<class InputIterator>
inline void GaussNet::setCenter( unsigned n, const InputIterator & start )
{
  gauss(n)->setCenter( start );
}


#endif


