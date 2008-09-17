
#include <si/subadaptive/subAdLogGauss.h>
#include <graph/tree/tree.h>
#include <iomanip>
#include <cmath>

using namespace sigraph;
using namespace std;


SubAdLogGauss::SubAdLogGauss( const string & name ) 
  : SubAdGauss( name ) 
{
}


SubAdLogGauss::SubAdLogGauss( const SubAdLogGauss & g ) 
	: SubAdGauss( g )
{
}


SubAdLogGauss::~SubAdLogGauss()
{
}


double SubAdLogGauss::prop( const vector<double> & vec )
{
  vector<double>	*nrmv = NULL;
  double		res;

  nrmv = normalizeSelected(&(vec[0]), vec.size());
  // we consider only one Gaussian here!
  res = _gnet.gauss(0)->arg( nrmv->begin()) + log(_gnet.weight(0)) + _defVal;
  delete nrmv;
  return res;
}

double SubAdLogGauss::learn(const GaussVectorLearnable &)
{
	std::cout << "SubAdGauss::learn : not implemented!" << std::endl;
	return 0.;
}


void SubAdLogGauss::buildTree( Tree & tr ) const
{
  SubAdGauss::buildTree( tr );
  tr.setSyntax( "sub_ad_loggauss" );
}

