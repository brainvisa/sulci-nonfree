
#include <cstdlib>
#include <si/subadaptive/subAdGauss.h>
#include <graph/tree/tree.h>
#include <iomanip>

using namespace sigraph;
using namespace std;


SubAdGauss::SubAdGauss( const string & name ) 
  : IncrementalSubAdaptive( name ), _etaW( 0.01 ), _etaC( 0.01 ), _etaS( 0.99 ), 
    _defVal( 0 )
{
}


SubAdGauss::SubAdGauss( const SubAdGauss & g ) 
	: IncrementalSubAdaptive( g ), _gnet( g._gnet ), _etaW( g._etaW ),
	_etaC( g._etaC ), _etaS( g._etaS ), _defVal( g._defVal )
{
}


SubAdGauss::~SubAdGauss()
{
}


void SubAdGauss::init()
{
  SubAdaptive::init();
}

void SubAdGauss::randinit()
{
  SubAdaptive::init();
  (_gnet.*_gnet.randInit)( 1, 0.3, 5, 1 );
}

double SubAdGauss::prop( const vector<double> & vec )
{
  vector<double>	*nrmv = NULL;
  double		res;

  nrmv = normalizeSelected(&(vec[0]), vec.size());
  res = _gnet.prop( nrmv->begin() ) + _defVal;
  delete nrmv;
  return res;
}


double SubAdGauss::learn(AdaptiveLeaf &,
			const SiDBLearnable &,
			const SiDBLearnable &)
{
	std::cout << "SubAdGauss::learn : not implemented!" << std::endl;
	return 0.;
};

SubAdResponse	*SubAdGauss::train(AdaptiveLeaf &,
				const SiDBLearnable &,
				const SiDBLearnable &)
{
	std::cout << "SubAdGauss::train : not implemented!" << std::endl;
	return NULL;
};



double SubAdGauss::learn(const GaussVectorLearnable &vl)
{
  std::vector<double>	vec = vl.X();
  vector<double>	*nrmv = NULL;
  double		outp = vl.y();

  nrmv = normalizeSelected(&(vec[0]), vl.sizeX());

  //	moins précis (résultat du coup d'avant) mais + rapide...
  double res = (_gnet.*_gnet.learn)( *nrmv, outp - _defVal, _etaW, _etaC, 
				     _etaS ) + _defVal;

  //	résultat exact, mais nécessite une propagation de plus...
  //double res = _gnet.prop( nrmv->begin() ) + _defVal;
  double err = fabs( res - outp );

  //	taux d'erreur
  setErrorRate( (1. - ForgetFactor) * errorRate() + ForgetFactor * err );
  if( outp < 0 )
    setAppGoodErrorRate( (1. - ForgetFactor) * appGoodErrorRate() 
			 + ForgetFactor * res );

  if( fileOpened() )
    {
      for( unsigned i=0; i<_gnet.nInputs(); ++i )
	*_stream << (*nrmv)[i] << "\t";
      *_stream << res << "\t" 
	       << outp << "\t" 
	       << err << "\t" << errorRate() << endl;
    }

  delete nrmv;
  return( err );
}


void SubAdGauss::buildTree( Tree & tr ) const
{
  tr.setSyntax( "sub_ad_gauss" );
  SubAdaptive::buildTree( tr );

  tr.setProperty( "etaW", (float) _etaW );
  tr.setProperty( "etaC", (float) _etaC );
  tr.setProperty( "etaS", (float) _etaS );
  tr.setProperty( "default_value", (float) _defVal );

  //	réseau de gaussiennes (temporairement ici)
  //  tr.setProperty( "ninputs", (int) _gnet.nInputs() );
  //  tr.setProperty( "ngauss", (int) _gnet.nGauss() );

  vector<float>		wt, sg;
  vector<float>		ct;
  unsigned		i, j, n = _gnet.nGauss(), m = _gnet.nInputs();
  const Gaussian	*gs;
  const double		*c;
  int			samesigma = 1;

  if( n > 0 && !_gnet.gauss( 0 )->isSameSigma() )
    samesigma = 0;

  for( i=0; i<n; ++i )
    {
      wt.push_back( _gnet.weight( i ) );
      gs = _gnet.gauss( i );
      c = gs->center();
      if( samesigma )
	sg.push_back( gs->sigma() );
      else
	for( j=0; j<m; ++j )
	  sg.push_back( gs->sigma( j ) );
      for( j=0; j<m; ++j )
	ct.push_back( c[j] );
    }

  tr.setProperty( "gauss_weights", wt );
  tr.setProperty( "gauss_sigma", sg );
  tr.setProperty( "gauss_centers", ct );
  tr.setProperty( "gauss_samesigma", samesigma );

  if( (void (GaussNet::*)()) _gnet.learn == 
      (void (GaussNet::*)()) &GaussNet::learnAll )
    tr.setProperty( "gauss_learn_func", (string) "gradient" );
  else if( (void (GaussNet::*)()) _gnet.learn == 
	   (void (GaussNet::*)()) &GaussNet::learnWeights )
    tr.setProperty( "gauss_learn_func", (string) "gradient_weights" );
  else if( (void (GaussNet::*)()) _gnet.learn == 
	   (void (GaussNet::*)()) &GaussNet::learnCenters )
    tr.setProperty( "gauss_learn_func", (string) "gradient_centers" );
  else if( (void (GaussNet::*)()) _gnet.learn == 
	   (void (GaussNet::*)()) &GaussNet::learnSigma )
    tr.setProperty( "gauss_learn_func", (string) "gradient_sigma" );
  else if( (void (GaussNet::*)()) _gnet.learn == 
	   (void (GaussNet::*)()) &GaussNet::learnWtSig )
    tr.setProperty( "gauss_learn_func", (string) "gradient_wtsig" );
  else if( (void (GaussNet::*)()) _gnet.learn == 
	   (void (GaussNet::*)()) &GaussNet::learnMulSig )
    tr.setProperty( "gauss_learn_func", (string) "gradient_mulsig" );
  else if( (void (GaussNet::*)()) _gnet.learn == 
	   (void (GaussNet::*)()) &GaussNet::learnCycle )
    tr.setProperty( "gauss_learn_func", (string) "cycle" );
}

