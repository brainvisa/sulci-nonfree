
#ifndef NEUR_GAUSS_GAUSSIAN_H
#define NEUR_GAUSS_GAUSSIAN_H

#include <iostream>
#include <math.h>


/**	Gaussienne, avec centre et écart-type
 */
class Gaussian
{
public:
  Gaussian();
  Gaussian( unsigned ninputs, bool samesigma = true );
  Gaussian( const Gaussian & g );
  ~Gaussian();

  /**@name	Initialisation */
  //@{
  void clear();
  void init( unsigned ninputs, bool samesigma = true );
  //@}

  /**@name	Fonctionnement */
  //@{
  bool isSameSigma() const { return( _sameSigma ); }
  const double *center() const { return( _center ); }
  double *center() { return( _center ); }
  double centerCoord( unsigned n ) const { return( _center[n] ); }
  void setCenterCoord( unsigned n, double c ) { _center[n] = c; }
  template<class InputIterator> 
  void setCenter( const InputIterator & start );
  const double *sigmas() const { return( _sigma ); }
  double *sigmas() { return( _sigma ); }
  double sigma( unsigned n = 0 ) const 
    { if( _sameSigma ) return( _sigma[0] ); else return( _sigma[n] ); }
  void setSigma( unsigned n, double s )
    { if( _sameSigma ) _sigma[0] = s; else _sigma[n] = s; }
  unsigned nInputs() const { return _ninputs; }
  template<class InputIterator> 
  double value( const InputIterator & start ) const;
  /**	Initialise avec des valeurs aléatoires.
	@param	cmin	min des coordonnées des centres
	@param	cmax	max des coordonnées des centres
	@param	smax	max des écarts-types
  */
  void randInit( double cmin = -1, double cmax = -1, double smin = 0.3, 
		 double smax = 5 );
  ///	Initialise seulement les écarts-types
  void randInitSigma( double smin = 0.3, double smax = 5 );
  ///	Initialise seulement les centres
  void randInitCenter( double cmin = -1, double cmax = 1 );
  ///	Calcule l'argument de l'exponentielle
  template<class InputIterator> 
  double arg( const InputIterator & start ) const;
  //@}

  Gaussian & operator = ( const Gaussian & g );

protected:

private:
  unsigned	_ninputs;
  double	*_center;
  double	*_sigma;
  bool		_sameSigma;
};


//	inline

template<class InputIterator>
inline 
double Gaussian::value( const InputIterator & start ) const
{
  double x = arg( start );
  return( exp( -( x < 700 ? x : 700 ) ) );
}


template<class InputIterator>
inline 
double Gaussian::arg( const InputIterator & start ) const
{
  double	x = 0, tmp;
  InputIterator	it = start;
  unsigned	i;
  double	*center = _center;
  double	*sigma = _sigma;

  if( _sameSigma )
    {
      double	sig = *sigma;
      for( i=0; i<_ninputs; ++i, ++it, ++center )
	{
	  tmp = (*center) - (*it);
	  x += tmp * tmp;
	}
      if( sig != 0 )
	x /= 2 * sig * sig;
      else
	std::cerr << "Gaussian : Pas d'écart-type\n";
    }
  else
    {
      for( i=0; i<_ninputs; ++i, ++it, ++center, ++sigma )
	{
	  tmp = (*center) - (*it);
	  x += ( tmp * tmp ) / ( 2 * (*sigma) * (*sigma) );
	}
    }
  return( x );
}


template<class InputIterator>
inline void Gaussian::setCenter( const InputIterator & start )
{
  InputIterator	it = start;
  unsigned	i;
  double	*c = _center;

  for( i=0; i<_ninputs; ++i, ++it, ++c )
    *c = *it;
}



#endif



