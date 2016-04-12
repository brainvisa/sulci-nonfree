/***********************************************
 * File : rand.cc
 * Prog : Fonctions de tirage au sort
 ***********************************************/



#include <cstdlib>
#include <stdlib.h>
#include <cmath>
#include <time.h>
#include <neur/rand/rand.h>
#include <float.h>


#define EPS 1.2E-7
#define RNMX (1.0 - EPS)
#define NTAB 32
#define IA 16807
#define IM 2147483647
#define AM (1.0/IM)
#define IQ 127773
#define IR 2836
#define NDIV (1+(IM-1)/NTAB)

static long *_idum = NULL;
static bool _reset_srand = true;


double ran1()
{
  int j;
  long k;
  static long iy = 0;
  static long iv[NTAB];
  double temp;

  if( _idum == NULL )
    {
      _idum = new long;
      if( _reset_srand )
        srand( time( NULL ) );
      *_idum = -rand();
      iy = 0;
    }

  if( *_idum <= 0 || !iy )
    {
      if( -(*_idum)<1 ) *_idum = 1;
      else *_idum = -(*_idum);
      for( j=NTAB+7; j>=0; j-- )
	{
	  k = (*_idum) / IQ;
	  *_idum = IA * (*_idum - k*IQ) - IR*k;
	  if( *_idum < 0 ) *_idum += IM;
	  if( j < NTAB ) iv[j] = *_idum;
	}
      iy = iv[0];
    }
  k = (*_idum) / IQ;
  *_idum = IA * (*_idum - k*IQ) - IR*k;
  if( *_idum < 0 ) *_idum += IM;
  j = iy / NDIV;
  iy = iv[j];
  iv[j] = *_idum;
  if( (temp=AM*iy) > RNMX ) return( RNMX );
  else return( temp );
}


void setRandSeed( long seed )
{
  delete _idum;
  _idum = 0;
  _reset_srand = false;
  srand( seed );
}


double GaussRand( double mean, double stdev )
{
  static int iset = 0;
  static double gset;
  double fac, rsq, v1,v2;

  if( iset == 0 )
    {
      do
	{
	  v1 = 2. * ran1() -1.;
	  v2 = 2. * ran1() -1.;
	  rsq = v1*v1 + v2*v2;
	} while( rsq >=1. || rsq == 0. );
      fac = sqrt( -2.*log( rsq )/rsq );
      gset = v1 * fac;
      iset = 1;
      return( mean + v2 * fac * stdev );
    }
  else
    {
      iset = 0;
      return( mean + gset*stdev );
    }
}


double OwnRand( double P(double ), double st, double mi, double ma )
{
  double	y = ran1();
  //  cout << "<but:" << y << "> ";
  const	double	eps = 1e-5;
  double	yy,x,dp;
  //  char	a[100];

  x = st;

  while( fabs(P(x)-y) > eps )
    {
      yy = P( x );
      //      cout << "<" << x << ":" << yy << "> ";
      //      cin >> a;
      dp = (yy - P( x-eps )) / eps;
      if( dp == 0 ) dp = 1.e-5;
      //      cout << "<dp:" << dp << ", dy:" << y-yy << "> ";
      x += 0.9 * (y-yy) / dp;
      if( x<mi ) x=mi;
      else if( x>ma ) x=ma;
    }
  return(x);
}


void randOrder( int *list, int n )
{
  int	i, j;
  int	cla[n+1];

  for( i=0; i<n; i++ ) list[i] = rand();

//	Tri

  cla[0] = -1;
  for( i=0; i<n; i++)
    {
      for( j=0; cla[j]!=-1; j=cla[j]+1 )
	if( list[i] < list[cla[j]] ) break;
      cla[i+1] = cla[j];
      cla[j] = i;
    }

//	Reecriture de la chaine d'indices dans l'ordre

  i = 0;
  for( j=0; cla[j]!=-1; j=cla[j]+1 ) list[i++] = cla[j];
}



float EnergyFunc( float x )
{
  static float y, z;
  return( (y=(z=x*x)*z) / ( 1. + 15./16 * y ) );
}


float angle( float x, float y )
{
  if( x == 0. )
    {
      if( y < 0 ) return( -M_PI/2 );
      else return( M_PI/2 );
    }

  float a = atan( y/x );

  if( x >= 0 ) return( a );
  if( y >= 0 ) return( a+M_PI );
  return( a-M_PI );
}




