
#include <cstdlib>
#include <neur/gauss/gaussian.h>
#include <stdlib.h>


Gaussian::Gaussian() : _ninputs( 0 ), _center( 0 ), _sigma( 0 ), 
  _sameSigma( true )
{
}


Gaussian::Gaussian( unsigned ninputs, bool samesigma ) 
  : _center( 0 ), _sigma( 0 )
{
  init( ninputs, samesigma );
}


Gaussian::Gaussian( const Gaussian & g ) : _center( 0 ), _sigma( 0 )
{
  unsigned	i, n;

  init( g._ninputs, g._sameSigma );

  for( i=0; i<_ninputs; ++i )
    _center[i] = g._center[i];

  if( _sameSigma )
    n = 1;
  else
    n = _ninputs;
  for( i=0; i<n; ++i )
    _sigma[i] = g._sigma[i];
}


Gaussian::~Gaussian()
{
  clear();
}


Gaussian & Gaussian::operator = ( const Gaussian & g )
{
  if( this != &g )
    {
      clear();
      init( g._ninputs, g._sameSigma );

      unsigned	i, n;

      for( i=0; i<_ninputs; ++i )
	_center[i] = g._center[i];

      if( _sameSigma )
	n = 1;
      else
	n = _ninputs;
      for( i=0; i<n; ++i )
	_sigma[i] = g._sigma[i];
    }
  return( *this );
}


void Gaussian::clear()
{
  delete[] _center;
  delete[] _sigma;
  _ninputs = 0;
  _center = 0;
  _sigma = 0;
}


void Gaussian::init( unsigned ninputs, bool samesigma )
{
  clear();

  _center = new double[ ninputs ];
  _sameSigma = samesigma;
  if( samesigma )
    _sigma = new double[ 1 ];
  else
    _sigma = new double[ ninputs ];
  _ninputs = ninputs;
}


void Gaussian::randInit( double cmin, double cmax, double smin, double smax )
{
  if( _center )
    {
      unsigned	i, n;

      for( i=0; i<_ninputs; ++i )
	_center[i] = ((double) (rand() & 0x7fff)) * (cmax-cmin) / 0x7fff 
	  + cmin;
      if( _sameSigma )
	n = 1;
      else
	n = _ninputs;
      for( i=0; i<n; ++i )
	_sigma[i] = ((double) (rand() & 0x7fff)) * (smax-smin) / 0x7fff + smin;
    }
}


void Gaussian::randInitSigma( double smin, double smax )
{
  if( _sigma )
    {
      unsigned	i, n;

      if( _sameSigma )
	n = 1;
      else
	n = _ninputs;
      for( i=0; i<n; ++i )
	_sigma[i] = ((double) (rand() & 0x7fff)) * (smax-smin) / 0x7fff + smin;
    }
}


void Gaussian::randInitCenter( double cmin, double cmax )
{
  if( _center )
    {
      unsigned	i;

      for( i=0; i<_ninputs; ++i )
	_center[i] = ((double) (rand() & 0x7fff)) * (cmax-cmin) / 0x7fff 
	  + cmin;
    }
}


