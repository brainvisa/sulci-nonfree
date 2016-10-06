
#include <si/fold/domainRbf.h>
#include <neur/gauss/gaussnet.h>
#include <si/fold/fattrib.h>
#include <graph/tree/tree.h>
#include <graph/graph/graph.h>
#include <aims/vector/vector.h>
#include <cartobase/smart/rcptr.h>
#include <aims/bucket/bucket.h>

using namespace sigraph;
using namespace aims;
using namespace carto;
using namespace std;


struct DomainRBF::Private
{
  Private();
  Private( const Private & );

  GaussNet	gnet;
  float		threshold;
  float		learnThreshold;
  float		sigma;
};


DomainRBF::Private::Private()
  : threshold( 0.5 ), learnThreshold( 0.6 ), sigma( 3 )
{
  gnet.init( 3, 0 );
}


DomainRBF::Private::Private( const Private & p )
  : gnet( p.gnet ), threshold( p.threshold ), 
    learnThreshold( p.learnThreshold ), sigma( p.sigma )
{
}

// ------

DomainRBF::DomainRBF() : AdapDomain(), d( new Private )
{
}


DomainRBF::DomainRBF( const DomainRBF & dom ) 
  : AdapDomain( dom ), d( new Private( *dom.d ) )
{
}


void DomainRBF::reset()
{
  AdapDomain::reset();
  d->gnet.init( 3, 0 );
  //d->threshold = 0.5;
  //d->learnThreshold = 0.6;
  //d->sigma = 5;
}


void DomainRBF::buildTree( Tree & tr ) const
{
  AdapDomain::buildTree( tr );

  tr.setSyntax( SIA_DOMAIN_RBF );
  tr.setProperty( SIA_SIGMA, d->sigma );
  tr.setProperty( SIA_THRESHOLD, d->threshold );
  tr.setProperty( SIA_LEARNTHRESHOLD, d->learnThreshold );

  unsigned		i, n = d->gnet.nGauss();
  vector<float>		centers( n * 3 );
  const Gaussian	*gn;
  const double		*c;

  for( i=0; i<n; ++i )
    {
      gn = d->gnet.gauss( i );
      c = gn->center();
      centers[i*3] = c[0];
      centers[i*3+1] = c[1];
      centers[i*3+2] = c[2];
    }
  if( centers.size() > 0 )
    tr.setProperty( SIA_GAUSSCENTERS, centers );
}


float DomainRBF::sigma() const
{
  return d->sigma;
}


float DomainRBF::threshold() const
{
  return d->threshold;
}


float DomainRBF::learnThreshold() const
{
  return d->learnThreshold;
}


void DomainRBF::setSigma( float x )
{
  d->sigma = x;
}


void DomainRBF::setThreshold( float x )
{
  d->threshold = x;
}


void DomainRBF::setLearnThreshold( float x )
{
  d->learnThreshold = x;
}


unsigned DomainRBF::nGauss() const
{
  return d->gnet.nGauss();
}


const GaussNet & DomainRBF::gaussNet() const
{
  return d->gnet;
}


GaussNet & DomainRBF::gaussNet()
{
  return d->gnet;
}


void DomainRBF::buildDomRBF( Tree*, Tree* ao )
{
  if( ao->childrenSize() != 0 )
    {
      cerr << "warning : Domain Box with children\n";
    }

  DomainRBF	*drbf = new DomainRBF;
  int		ndata;
  float		sigma, threshold, learnThreshold;
  vector<float>	centers;

  ao->setProperty( SIA_POINTER, (Domain *) drbf );
  ao->getProperty( SIA_NDATA, ndata );
  drbf->setNData( ndata );
  ao->getProperty( SIA_SIGMA, sigma );
  ao->getProperty( SIA_THRESHOLD, threshold );
  ao->getProperty( SIA_LEARNTHRESHOLD, learnThreshold );
  ao->getProperty( SIA_GAUSSCENTERS, centers );
  drbf->setSigma( sigma );
  drbf->setThreshold( threshold );
  drbf->setLearnThreshold( learnThreshold );

  unsigned	i, n = centers.size() / 3;
  Gaussian	*gs;
  GaussNet	& gnet = drbf->gaussNet();

  gnet.init( 3, n );

  for( i=0; i<n; ++i )
    {
      gs = gnet.gauss( i );
      gs->setCenter( &centers[i*3] );
      gs->setSigma( 0, sigma );
    }
}


bool DomainRBF::canBeFound( const Vertex* v, const Graph* )
{
  Vertex::const_iterator	ie, fe=v->end();
  Edge				*e;
  vector<float>			e1, e2;

  for( ie=v->begin(); ie!=fe; ++ie )
    if( (e=*ie)->getSyntax() == SIA_HULLJUNCTION_SYNTAX )
      {
	if( e->getProperty( SIA_REFEXTREMITY1, e1 ) 
	    && e->getProperty( SIA_REFEXTREMITY2, e2 ) )
	  if( !canBeFound( e1[0], e1[1], e1[2] ) 
	      || !canBeFound( e2[0], e2[1], e2[2] ) )
	    return( false );
	break;
      }

  vector<float>	g;

  if( !v->getProperty( SIA_REFGRAVITY_CENTER, g ) )
    return( false );

  return( canBeFound( g[0], g[1], g[2] ) );
}


bool DomainRBF::canBeFound( double x, double y, double z )
{
  float	g[3];

  g[0] = x;
  g[1] = y;
  g[2] = z;

  return( d->gnet.prop( &g[0] ) >= d->threshold );
}


void DomainRBF::learn( const Vertex* v, const Graph* g )
{
  learnBucket( v, g );
}


void DomainRBF::learnBucket( const Vertex* v, const Graph* g )
{
  rc_ptr<BucketMap<Void> >			ssbck, bbck, obck;
  BucketMap<Void>::Bucket::const_iterator	ib, fb;
  vector<float>					rot, scale, transl, vsz;

  if( !g || !g->getProperty( SIA_TALAIRACH_ROTATION, rot ) 
      || !g->getProperty( SIA_TALAIRACH_SCALE, scale ) 
      || !g->getProperty( SIA_TALAIRACH_TRANSLATION, transl ) 
      || !g->getProperty( SIA_VOXEL_SIZE, vsz ) )
    return;
  if( !v->getProperty( SIA_SS_BUCKET, ssbck ) )
    return;
  v->getProperty( SIA_BOTTOM_BUCKET, bbck );
  v->getProperty( SIA_OTHER_BUCKET, obck );

  for( ib = (*ssbck)[0].begin(), fb = (*ssbck)[0].end(); ib!=fb; ++ib )
    learnVoxel( rot, scale, transl, vsz, ib->first );
  if( bbck.get() )
    for( ib = (*bbck)[0].begin(), fb = (*bbck)[0].end(); ib!=fb; ++ib )
      learnVoxel( rot, scale, transl, vsz, ib->first );
  if( obck.get() )
    for( ib = (*obck)[0].begin(), fb = (*obck)[0].end(); ib!=fb; ++ib )
      learnVoxel( rot, scale, transl, vsz, ib->first );

  // points extrêmes des hull_junction

  Vertex::const_iterator	ie, fe=v->end();
  Edge				*e;
  vector<float>			e1, e2;

  for( ie=v->begin(); ie!=fe; ++ie )
    if( (e=*ie)->getSyntax() == SIA_HULLJUNCTION_SYNTAX )
      {
	if( e->getProperty( SIA_REFEXTREMITY1, e1 ) 
	    && e->getProperty( SIA_REFEXTREMITY2, e2 ) )
	  {
	    learnTalVoxel( e1[0], e1[1], e1[2] );
	    learnTalVoxel( e2[0], e2[1], e2[2] );
	  }
	break;
      }

  ++_ndata;
}


void DomainRBF::learnVoxel( const vector<float> & rot, 
			    const vector<float> & scale, 
			    const vector<float> & transl, 
			    const vector<float> & vsz, 
			    const AimsVector<short, 3> & pt )
{
  // Talairach
  double	x = vsz[0] * pt[0] + transl[0];
  double	y = vsz[1] * pt[1] + transl[1];
  double	z = vsz[2] * pt[2] + transl[2];
  double	x2 = ( rot[0] * x + rot[1] * y + rot[2] * z ) * scale[0];
  double	y2 = ( rot[3] * x + rot[4] * y + rot[5] * z ) * scale[1];
  double	z2 = ( rot[6] * x + rot[7] * y + rot[8] * z ) * scale[2];

  learnTalVoxel( x2, y2, z2 );
}


void DomainRBF::learnTalVoxel( double x2, double y2, double z2 )
{
  float		pos[3];
  pos[0] = x2;
  pos[1] = y2;
  pos[2] = z2;

  double	prop = d->gnet.prop( &pos[0] );
  if( prop >= d->learnThreshold )
    return;	// no need to learn

  // add a gaussian
  Gaussian	*gs = d->gnet.addGaussian();
  gs->setCenter( &pos[0] );
  gs->setSigma( 0, d->sigma );
}


