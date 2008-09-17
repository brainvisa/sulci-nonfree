
#include <neur/gauss/gaussnet.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

unsigned GaussNet::CyclePeriod = 1000;


struct GaussNet::Private
{
  Private();
  Private( const Private & );
  ~Private();
  Private & operator = ( const Private & );
  void clear();

  ///	Gaussiennes
  std::vector<Gaussian *>	gauss;
  ///	Poids des gaussiennes vers la sortie
  std::vector<double>		weights;
  ///	compteur pour certains apprentissages
  unsigned			count;
};


GaussNet::Private::Private() : count( 0 )
{
}


GaussNet::Private::Private( const GaussNet::Private & p ) : count( p.count )
{
  unsigned	i, n = p.gauss.size();
  for( i=0; i<n; ++i )
    gauss.push_back( new Gaussian( *p.gauss[i] ) );
  for( i=0, n=p.weights.size(); i<n; ++i )
    weights.push_back( p.weights[i] );
}


GaussNet::Private::~Private()
{
  unsigned	i, n = gauss.size();
  for( i=0; i<n; ++i )
    delete gauss[i];
}


void GaussNet::Private::clear()
{
  unsigned	i, n = gauss.size();
  for( i=0; i<n; ++i )
    delete gauss[i];
  gauss.clear();
  weights.clear();
}


GaussNet::Private & 
GaussNet::Private::operator = ( const GaussNet::Private & p ) 
{
  if( this == &p )
    return( *this );

  clear();
  unsigned	i, n = p.gauss.size();
  for( i=0; i<n; ++i )
    gauss.push_back( new Gaussian( *p.gauss[i] ) );
  for( i=0, n=p.weights.size(); i<n; ++i )
    weights.push_back( p.weights[i] );
  count = p.count;
  return *this;
}

// ----

GaussNet::GaussNet() 
  : randInit( &GaussNet::randInitAll ), learn( &GaussNet::learnMulSig ), 
    d( new Private ), _ninputs( 0 )
{
}


GaussNet::GaussNet( const GaussNet & gn ) : randInit( gn.randInit ), 
  learn( gn.learn ), d( new Private )
{
  operator = ( gn );
}


GaussNet::~GaussNet()
{
  clear();
}


GaussNet & GaussNet::operator = ( const GaussNet & gn )
{
  if( this != &gn )
    {
      clear();
      randInit = gn.randInit;
      learn = gn.learn;
      _ninputs = gn._ninputs;
      *d = *gn.d;
    }

  return( *this );
}


void GaussNet::clear()
{
  d->clear();
  _ninputs = 0;
}


void GaussNet::init( unsigned ninp, unsigned ngauss, bool samesigma )
{
  clear();

  _ninputs = ninp;
  unsigned     i;
  for( i=0; i<ngauss; ++i )
    {
      d->gauss.push_back( new Gaussian( ninp, samesigma ) );
      d->weights.push_back( 1. );
    }
}


void GaussNet::randInitAll( double cmax, double smin, double smax, 
			    double wmax )
{
  unsigned	i, n = d->gauss.size();

  for( i=0; i<n; ++i )
    {
      d->gauss[i]->randInit( -cmax, cmax, smin, smax );
      d->weights[i] = ((double) (rand() & 0x7fff)) * wmax * 2 / 0x7fff - wmax;
    }
  d->count = 0;
}


void GaussNet::randInitWeights( double, double, double, 
				double wmax )
{
  unsigned	i, n = d->gauss.size();

  for( i=0; i<n; ++i )
    {
      d->weights[i] = ((double) (rand() & 0x7fff)) * wmax * 2 / 0x7fff - wmax;
    }
  d->count = 0;
}


void GaussNet::randInitCenters( double, double smin, double smax, 
				double )
{
  unsigned	i, n = d->gauss.size();

  for( i=0; i<n; ++i )
    d->gauss[i]->randInitCenter( smin, smax );
  d->count = 0;
}


void GaussNet::randInitSigma( double, double smin, double smax, 
			      double )
{
  unsigned	i, n = d->gauss.size();

  for( i=0; i<n; ++i )
    d->gauss[i]->randInitSigma( smin, smax );
  d->count = 0;
}


void GaussNet::randInitWtSig( double, double smin, double smax, 
			      double wmax )
{
  unsigned	i, n = d->gauss.size();

  for( i=0; i<n; ++i )
    {
      d->gauss[i]->randInitSigma( smin, smax );
      d->weights[i] = ((double) (rand() & 0x7fff)) * wmax * 2 / 0x7fff - wmax;
    }
  d->count = 0;
}


unsigned GaussNet::nGauss() const
{
  return d->gauss.size();
}


const Gaussian* GaussNet::gauss( unsigned n ) const
{
  return d->gauss[n];
}


Gaussian* GaussNet::gauss( unsigned n )
{
  return d->gauss[n];
}


double GaussNet::weight( unsigned n ) const
{
  return( d->weights[n] );
}


void GaussNet::setWeight( unsigned n, double w )
{
  d->weights[n] = w;
}


double GaussNet::learnAll( const vector<double> & vec, double d, double etaW, 
			   double etaC, double etaS )
{
  /**	GRADIENT :
	erreur: E = (s-d)^2 (carré de l'écart)
	Pour la variable v: dE/dv = 2(s-d)ds/dv (coord. sur v du gradient)
	Modif. de la variable v: Dv = -2 eta (s-d) ds/dv
  */

  unsigned		i, j, n = this->d->gauss.size();
  vector<double>	arg( n );
  double		*targ = &arg[0];
  vector<double>::const_iterator	iv = vec.begin();
  double		tmp, lf2, ts;
  Gaussian		**gs = &this->d->gauss[0];
  double		lrnf;
  double		expo, wt, sig, co, var;
  double		*c, *w = &this->d->weights[0];
  double		s = 0;	// sortie

  if( etaC == 0 )
    etaC = etaW;
  if( etaS == 0 )
    etaS = etaW;

  for( i=0; i<n; ++i, ++targ )
    {
      *targ = this->d->gauss[i]->arg( iv );
      s += w[i] * exp( -*targ );
    }

  lrnf = (d-s);	// à un facteur 2 près

  for( i=0, targ=&arg[0]; i<n; ++i, ++w, ++gs, ++targ )
    {
      expo = exp( -*targ );
      wt = *w;

      // variable: wi,		ds/dwi = exp( -garg )
      /*if( lrnf < 1e-10 && expo < 1e-280 )	// f...!@!! overflow !!
	*w += 1e-290;
	else*/
      lf2 = lrnf * expo;
      *w += lf2 * etaW;

      if( (*gs)->isSameSigma() )
	{
	  sig = (*gs)->sigmas()[0];
	  tmp = wt / (sig * sig) * lf2 * 2;

	  // variable: sig.i,	ds/dsig.i = 2 wi garg/sig.i^3 exp( -garg )
	  (*gs)->sigmas()[0] += tmp * *targ / sig * etaS;
	  if( (*gs)->sigmas()[0] <= 0 )
	    (*gs)->sigmas()[0] = 1e-30;

	  // variable: centre.ij	ds/dcij = 2 (vec.j-cij) wi/sig.i^2 expo
	  c = (*gs)->center();

	  for( j=0; j<_ninputs; ++j, ++c )
	    *c += tmp * (vec[j] - *c) * etaC;
	}
      else
	{
	  c = (*gs)->center();
	  tmp = wt * lf2 * 2;
	  for( j=0; j<_ninputs; ++j, ++c )
	    {
	      ts = (*gs)->sigmas()[j];
	      co = vec[j] - *c;

	      // variable: centre.ij   ds/dcij = 2 (vec.j-cij) wi/sig.ij^2 expo
	      var = tmp * co / (ts * ts);
	      *c += var * etaC;

	      // variable: sig.ij,
	      // ds/dsig.ij = 2 wi (xj-cij)^2 / sig.ij^3 exp( -garg )
	      (*gs)->sigmas()[j] += var * co / ts * etaS;
	      if( (*gs)->sigmas()[j] <= 0 )
		(*gs)->sigmas()[j] = 1e-30;
	    }
	}
    }

  return( s );
}


double GaussNet::learnWeights( const vector<double> & vec, double d, 
			       double etaW, double, double )
{
  /**	GRADIENT :
	erreur: E = (s-d)^2 (carré de l'écart)
	Pour la variable v: dE/dv = 2(s-d)ds/dv (coord. sur v du gradient)
	Modif. de la variable v: Dv = -2 eta (s-d) ds/dv
  */

  unsigned		i, n = this->d->gauss.size();
  vector<double>	arg( n );
  vector<double>::iterator		targ = arg.begin();
  vector<double>::const_iterator	iv = vec.begin();
  Gaussian		**gs = &this->d->gauss[0];
  double		lrnf;
  double		*w = &this->d->weights[0];
  double		s = 0;	// sortie

  for( i=0; i<n; ++i, ++targ )
    {
      *targ = this->d->gauss[i]->arg( iv );
      s += w[i] * exp( -*targ );
    }

  lrnf = (d-s) * etaW;	// à un facteur 2 près

  for( i=0, targ=arg.begin(); i<n; ++i, ++w, ++gs, ++targ )
    {
      // variable: wi,		ds/dwi = exp( -garg )
      /*if( lrnf < 1e-10 && expo < 1e-280 )	// f...!@!! overflow !!
	*w += 1e-290;
	else*/
      *w += lrnf * exp( -*targ );
    }

  return( s );
}


double GaussNet::learnCenters( const vector<double> & vec, double d, 
			       double etaW, double etaC, double etaS )
{
  /**	GRADIENT :
	erreur: E = (s-d)^2 (carré de l'écart)
	Pour la variable v: dE/dv = 2(s-d)ds/dv (coord. sur v du gradient)
	Modif. de la variable v: Dv = -2 eta (s-d) ds/dv
  */

  unsigned		i, j, n = this->d->gauss.size();
  vector<double>	arg( n );
  double		*targ = &arg[0];
  vector<double>::const_iterator	iv = vec.begin();
  double		lf2;
  Gaussian		**gs = &this->d->gauss[0];
  double		lrnf;
  double		ts;
  double		*c, *w = &this->d->weights[0];
  double		s = 0;	// sortie

  if( etaC == 0 )
    etaC = etaW;
  if( etaS == 0 )
    etaS = etaW;

  for( i=0; i<n; ++i, ++targ )
    {
      *targ = this->d->gauss[i]->arg( iv );
      s += w[i] * exp( -*targ );
    }

  lrnf = (d-s) * 2;	// à un facteur 2 près

  for( i=0, targ=&arg[0]; i<n; ++i, ++w, ++gs, ++targ )
    {
      lf2 = lrnf * exp( -*targ ) * *w;
      c = (*gs)->center();

      for( j=0; j<_ninputs; ++j, ++c )
	{
	  ts = (*gs)->sigma(j);

	  // variable: centre.ij   ds/dcij = 2 (vec.j-cij) wi/sig.ij^2 expo
	  *c += lf2 * etaC * (vec[j] - *c) / (ts * ts);
	}
    }

  return( s );
}


double GaussNet::learnSigma( const vector<double> & vec, double d, 
			     double etaW, double, double etaS )
{
  /**	GRADIENT :
	erreur: E = (s-d)^2 (carré de l'écart)
	Pour la variable v: dE/dv = 2(s-d)ds/dv (coord. sur v du gradient)
	Modif. de la variable v: Dv = -2 eta (s-d) ds/dv
  */

  unsigned		i, j, n = this->d->gauss.size();
  vector<double>	arg( n );
  double		*targ = &arg[0];
  vector<double>::const_iterator	iv = vec.begin();
  double		tmp, lf2, ts;
  Gaussian		**gs = &this->d->gauss[0];
  double		lrnf;
  double		expo, wt, sig, co;
  double		*c, *w = &this->d->weights[0];
  double		s = 0;	// sortie

  if( etaS == 0 )
    etaS = etaW;

  for( i=0; i<n; ++i, ++targ )
    {
      *targ = this->d->gauss[i]->arg( iv );
      s += w[i] * exp( -*targ );
    }

  lrnf = (d-s) * etaS;	// à un facteur 2 près

  for( i=0, targ=&arg[0]; i<n; ++i, ++w, ++gs, ++targ )
    {
      expo = exp( -*targ );
      wt = *w;

      lf2 = lrnf * expo;

      if( (*gs)->isSameSigma() )
	{
	  sig = (*gs)->sigmas()[0];
	  tmp = wt / (sig * sig) * lf2 * 2;

	  // variable: sig.i,	ds/dsig.i = 2 wi garg/sig.i^3 exp( -garg )
	  (*gs)->sigmas()[0] += tmp * *targ / sig;
	  if( (*gs)->sigmas()[0] <= 0 )
	    (*gs)->sigmas()[0] = 1e-30;
	}
      else
	{
	  c = (*gs)->center();
	  tmp = wt * lf2 * 2;
	  for( j=0; j<_ninputs; ++j, ++c )
	    {
	      ts = (*gs)->sigmas()[j];
	      co = vec[j] - *c;

	      // variable: sig.ij,
	      // ds/dsig.ij = 2 wi (xj-cij)^2 / sig.ij^3 exp( -garg )
	      (*gs)->sigmas()[j] += tmp * co * co / (ts * ts * ts);
	      if( (*gs)->sigmas()[j] <= 0 )
		(*gs)->sigmas()[j] = 1e-30;
	    }
	}
    }

  return( s );
}


double GaussNet::learnWtSig( const vector<double> & vec, double d, 
			     double etaW, double, double etaS )
{
  /**	GRADIENT :
	erreur: E = (s-d)^2 (carré de l'écart)
	Pour la variable v: dE/dv = 2(s-d)ds/dv (coord. sur v du gradient)
	Modif. de la variable v: Dv = -2 eta (s-d) ds/dv
  */

  unsigned		i, j, n = this->d->gauss.size();
  vector<double>	arg( n );
  double		*targ = &arg[0];
  vector<double>::const_iterator	iv = vec.begin();
  double		tmp, lf2, ts;
  Gaussian		**gs = &this->d->gauss[0];
  double		lrnf;
  double		expo, wt, sig, co;
  double		*c, *w = &this->d->weights[0];
  double		s = 0;	// sortie

  if( etaS == 0 )
    etaS = etaW;

  for( i=0; i<n; ++i, ++targ )
    {
      *targ = this->d->gauss[i]->arg( iv );
      s += w[i] * exp( -*targ );
    }

  lrnf = (d-s);	// à un facteur 2 près

  for( i=0, targ=&arg[0]; i<n; ++i, ++w, ++gs, ++targ )
    {
      expo = exp( -*targ );
      wt = *w;

      // variable: wi,		ds/dwi = exp( -garg )
      /*if( lrnf < 1e-10 && expo < 1e-280 )	// f...!@!! overflow !!
	*w += 1e-290;
	else*/
      lf2 = lrnf * expo;
      *w += lf2 * etaW;

      if( (*gs)->isSameSigma() )
	{
	  sig = (*gs)->sigmas()[0];
	  tmp = wt / (sig * sig) * lf2 * 2 * etaS;

	  // variable: sig.i,	ds/dsig.i = 2 wi garg/sig.i^3 exp( -garg )
	  (*gs)->sigmas()[0] += tmp * *targ / sig;
	  if( (*gs)->sigmas()[0] <= 0 )
	    (*gs)->sigmas()[0] = 1e-30;
	}
      else
	{
	  c = (*gs)->center();
	  tmp = wt * lf2 * 2 * etaS;
	  for( j=0; j<_ninputs; ++j, ++c )
	    {
	      ts = (*gs)->sigmas()[j];
	      co = vec[j] - *c;

	      // variable: sig.ij,
	      // ds/dsig.ij = 2 wi (xj-cij)^2 / sig.ij^3 exp( -garg )
	      (*gs)->sigmas()[j] += tmp * co *co / (ts * ts * ts);
	      if( (*gs)->sigmas()[j] <= 0 )
		(*gs)->sigmas()[j] = 1e-30;
	    }
	}
    }

  return( s );
}


double GaussNet::learnMulSig( const vector<double> & vec, double d, 
			      double etaW, double etaC, double etaS )
{
  /**	GRADIENT :
	erreur: E = (s-d)^2 (carré de l'écart)
	Pour la variable v: dE/dv = 2(s-d)ds/dv (coord. sur v du gradient)
	Modif. de la variable v: Dv = -2 eta (s-d) ds/dv
  */

  unsigned		i, j, n = this->d->gauss.size();
  vector<double>	arg( n );
  vector<double>::iterator		targ = arg.begin();
  vector<double>::const_iterator	iv = vec.begin();
  double		tmp, lf2, ts;
  Gaussian		**gs = &this->d->gauss[0];
  double		lrnf, mfac;
  double		expo, wt, sig, co, var;
  double		*c, *w = &this->d->weights[0];
  double		s = 0;	// sortie

  if( etaC == 0 )
    etaC = etaW;
  if( etaS == 0 )
    etaS = etaW;

  for( i=0; i<n; ++i, ++targ )
    {
      *targ = this->d->gauss[i]->arg( iv );
      s += w[i] * exp( -*targ );
    }

  lrnf = (d-s);	// à un facteur 2 près

  if( lrnf == 0 )	// pas besoin d'apprendre dans ce cas
    return( s );
  mfac = etaS * lrnf;	// eta * (d-s)

  for( i=0, targ=arg.begin(); i<n; ++i, ++w, ++gs, ++targ )
    {
      expo = exp( -*targ );
      wt = *w;

      // variable: wi,		ds/dwi = exp( -garg )
      /*if( lrnf < 1e-10 && expo < 1e-280 )	// f...!@!! overflow !!
	*w += 1e-290;
	else*/
      lf2 = lrnf * expo;	// (d-s)*expo
      *w += lf2 * etaW;

      if( (*gs)->isSameSigma() )
	{
	  sig = (*gs)->sigmas()[0];
	  tmp = wt / (sig * sig) * expo * 2;

	  // variable: sig.i,	ds/dsig.i = 2 wi garg/sig.i^3 exp( -garg )
	  // modif multiplicative: sig *= exp( -eta sig/E dE/dsig )
	  //				= exp( 2 eta sig/(d-s) ds/dsig )
	  // (vient de: d(ln E)/d(ln sig) )
	  (*gs)->sigmas()[0] *= exp( mfac * tmp * *targ / sig );

	  // variable: centre.ij	ds/dcij = 2 (vec.j-cij) wi/sig.i^2 expo
	  c = (*gs)->center();

	  tmp *= lrnf;	// tmp = 2 (d-s) w/sig^2 * expo

	  for( j=0; j<_ninputs; ++j, ++c )
	    *c += tmp * (vec[j] - *c) * etaC;
	}
      else
	{
	  c = (*gs)->center();
	  tmp = wt * expo * 2;
	  for( j=0; j<_ninputs; ++j, ++c )
	    {
	      ts = (*gs)->sigmas()[j];
	      co = vec[j] - *c;

	      // variable: centre.ij   ds/dcij = 2 (vec.j-cij) wi/sig.ij^2 expo
	      var = tmp * co / (ts * ts);	// 2 w.expo (xj-cij)/sig^2
	      *c += var * etaC * lrnf;

	      // variable: sig.ij,
	      // ds/dsig.ij = 2 wi (xj-cij)^2 / sig.ij^3 exp( -garg )
	      // modif multiplicative: sig *= exp( -eta sig/E dE/dsig )
	      //			    = exp( 2 eta sig/(d-s) ds/dsig )
	      // (vient de: d(ln E)/d(ln sig) )
	      (*gs)->sigmas()[j] *= exp( mfac * var * co );
	    }
	}
    }

  return( s );
}


double GaussNet::learnCycle( const vector<double> & vec, double d, 
			     double etaW, double etaC, double etaS )
{
  if( this->d->count < CyclePeriod )
    {
      if( this->d->count == 0 )
	this->d->count = 3 * CyclePeriod - 1;
      else
	--this->d->count;
      return( learnWeights( vec, d, etaW, etaC, etaS ) );
    }
  else if( this->d->count < 2 * CyclePeriod )
    {
      --this->d->count;
      return( learnCenters( vec, d, etaW, etaC, etaS ) );
    }
  else
    {
      --this->d->count;
      return( learnSigma( vec, d, etaW, etaC, etaS ) );
    }
}


Gaussian* GaussNet::addGaussian()
{
  bool samesigma = true;
  if( !d->gauss.empty() )
    samesigma = d->gauss[0]->isSameSigma();
  Gaussian	*g = new Gaussian( _ninputs, samesigma );
  d->gauss.push_back( g );
  d->weights.push_back( 1. );
  return g;
}


void GaussNet::removeGaussian( unsigned num )
{
  if( d->gauss.size() > num )
    {
      delete d->gauss[num];
      d->gauss.erase( d->gauss.begin() + num );
    }
  if( d->weights.size() > num )
    d->weights.erase( d->weights.begin() + num );
}


