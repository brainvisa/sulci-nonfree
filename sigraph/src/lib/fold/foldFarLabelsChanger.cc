
#include <si/fold/foldFarLabelsChanger.h>
#include <aims/math/random.h>

using namespace sigraph;
using namespace std;


unsigned FoldFarLabelsChanger::randomGen( unsigned n )
{
  double	a = 0.2;	// dp(0)
  double	x = UniformRandom();
  double	y = 1./(a+2)*( 3*x + 4*(a-1) 
			       * ( (x-0.5) * (x-0.5) * (x-0.5) + 0.125 ) );
  return( (unsigned) ( y * ( n + 0.99 ) ) );
}


double
FoldFarLabelsChanger::constrainedNoise( Clique* cl, double & outp, 
					const set<string> & significantLabels, 
					const string & voidLabel )
{
  double	r;
  unsigned	i;

  for( i=0; i<3; ++i )
    {
      r = FoldLabelsChanger::constrainedNoise( cl, outp, significantLabels, 
					       voidLabel );
      if( r >= 400 )
	return( r );
    }
  return( 0 );
}



