#include <cstdlib>
#include <si/fold/inertialDomainBox.h>
#include <aims/math/eigen.h>
#include <aims/bucket/bucket.h>
#include <si/fold/fattrib.h>
#include <graph/graph/graph.h>
#include <si/fold/fgraph.h>
#include <graph/tree/tree.h>
#include <cartobase/smart/rcptr.h>
#include <cartobase/exception/assert.h>

using namespace sigraph;
using namespace aims;
using namespace carto;
using namespace std;


InertialDomainBox::InertialDomainBox()
  : DomainBox(), _inertia( 3, 3 ), _gravity( 3 ), _rotation( 3, 3 ),
    _eigenValues( 3 ), _npoints( 0 ), _transfUpToDate( false ), _tolMargin( 0 )
{
  _inertia = 0;
  _gravity = 0;
}


bool InertialDomainBox::canBeFound( double x, double y, double z )
{
  if( !_transfUpToDate )
    diagonalize();

  changeRef( x, y, z );
  return( DomainBox::canBeFound( x, y, z ) );
}


void InertialDomainBox::firstPass()
{
  _transfUpToDate = false;
  _gravity[0] *= _npoints;
  _gravity[1] *= _npoints;
  _gravity[2] *= _npoints;
}


void InertialDomainBox::reset()
{
  DomainBox::reset();
  _transfUpToDate = false;
  _inertia = 0;
  _gravity = 0;
  _npoints = 0;
}


bool InertialDomainBox::canBeFound( const Vertex* v, const Graph* )
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

  if( !v->getProperty( _gcattrib, g ) )
    return( false );
  double	x = g[0], y = g[1], z = g[2];
  return( canBeFound( x, y, z ) );
}


void InertialDomainBox::learn( const Vertex* v, const Graph* g )
{
  if( _transfUpToDate )
    learnBucket( v, g );
  else	// apprends la matrice d'inertie
    {
      vector<float>	inert;
      int		pn;
      vector<float>	gc;

      ASSERT( v->getProperty( SIA_INERTIA, inert ) );
      ASSERT( v->getProperty( SIA_POINT_NUMBER, pn ) );
      ASSERT( v->getProperty( _gcattrib, gc ) );
      _inertia( 0, 0 ) += inert[0];
      _inertia( 0, 1 ) += inert[1];
      _inertia( 0, 2 ) += inert[2];
      _inertia( 1, 0 ) += inert[3];
      _inertia( 1, 1 ) += inert[4];
      _inertia( 1, 2 ) += inert[5];
      _inertia( 2, 0 ) += inert[6];
      _inertia( 2, 1 ) += inert[7];
      _inertia( 2, 2 ) += inert[8];
      _gravity[0] += gc[0] * pn;
      _gravity[1] += gc[1] * pn;
      _gravity[2] += gc[2] * pn;
      _npoints += pn;
    }
}


void InertialDomainBox::learnBucket( const Vertex* v, const Graph* g )
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

  // points extr�mes des hull_junction

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


void InertialDomainBox::learnVoxel( const vector<float> & rot,
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


void InertialDomainBox::learnTalVoxel( double x2, double y2, double z2 )
{
  // Rep�re d'inertie
  changeRef( x2, y2, z2 );

  if( _npoints == 0 )
    setDims( x2 - _tolMargin, y2 - _tolMargin, z2 - _tolMargin,
	     x2 + _tolMargin, y2 + _tolMargin, z2 + _tolMargin );
  else
    {
      if( x2 < _xmin + _tolMargin )
	_xmin = x2 - _tolMargin;
      else if( x2 > _xmax - _tolMargin )
	_xmax = x2 + _tolMargin;
      if( y2 < _ymin + _tolMargin )
	_ymin = y2 - _tolMargin;
      else if( y2 > _ymax - _tolMargin )
	_ymax = y2 + _tolMargin;
      if( z2 < _zmin + _tolMargin)
	_zmin = z2 - _tolMargin;
      else if( z2 > _zmax - _tolMargin)
	_zmax = z2 + _tolMargin;
    }
  ++_npoints;
}


void InertialDomainBox::diagonalize()
{
  _rotation = _inertia.clone();
  AimsEigen<float>	eigen;
  //eigen.setSymmetricMatrix();
  AimsData<float> m_eigenValues = eigen.doit( _rotation );
  _eigenValues = AimsData<float>( 3 );
  _eigenValues(0) = m_eigenValues(0,0);
  _eigenValues(1) = m_eigenValues(1,1);
  _eigenValues(2) = m_eigenValues(2,2);
  // check that the referential is direct
  AimsData<float>	pvec(3);
  pvec[0] = _rotation( 1, 0 ) *  _rotation( 2, 1 )
    - _rotation( 2, 0 ) *  _rotation( 1, 1 );
  pvec[1] = _rotation( 2, 0 ) * _rotation( 0, 1 )
    - _rotation( 0, 0 ) * _rotation( 2, 1 );
  pvec[2] = _rotation( 0, 0 ) * _rotation( 1, 1 )
    - _rotation( 1, 0 ) * _rotation( 0, 1 );
  if( fabs( pvec[0] - _rotation( 0, 2 ) ) > 1e-5
      || fabs( pvec[1] - _rotation( 1, 2 ) ) > 1e-5
      || fabs( pvec[2] - _rotation( 2, 2 ) ) > 1e-5 )
  {
    cout << "indirect referential\n";
    if( fabs( pvec[0] + _rotation( 0, 2 ) ) > 1e-5
      || fabs( pvec[1] + _rotation( 1, 2 ) ) > 1e-5
      || fabs( pvec[2] + _rotation( 2, 2 ) ) > 1e-5 )
      cout << "La transformation ne marche pas : " << _rotation( 0, 2 )
        << ", " << _rotation( 1, 2 ) << ", " << _rotation( 2, 2 )
        << " au lieu de " << pvec[0] << ", " << pvec[1] << ", "
        << pvec[2] << ".\n";
    else
    {
      /*_rotation( 0, 2 ) *= -1;
      _rotation( 1, 2 ) *= -1;
      _rotation( 2, 2 ) *= -1;*/
      // swap de vecteurs
      float	t = _rotation( 0, 0 );
      _rotation( 0, 0 ) = _rotation( 0, 2 );
      _rotation( 0, 2 ) = t;
      t = _rotation( 1, 0 );
      _rotation( 1, 0 ) = _rotation( 1, 2 );
      _rotation( 1, 2 ) = t;
      t = _rotation( 2, 0 );
      _rotation( 2, 0 ) = _rotation( 2, 2 );
      _rotation( 2, 2 ) = t;
      //	et des valeurs propres associ�es
      t = _eigenValues(0);
      _eigenValues(0) = _eigenValues(2);
      _eigenValues(2) = t;
    }
  }
  else cout << "direct referential\n";

  if( _npoints )
    {
      _gravity[0] /= _npoints;
      _gravity[1] /= _npoints;
      _gravity[2] /= _npoints;
    }
  _transfUpToDate = true;
}


void InertialDomainBox::changeRef( double & x, double & y, double & z )
{
  double x2 = x - _gravity[0], y2 = y - _gravity[1], z2 = z - _gravity[2];

  /*x = _rotation( 0, 0 ) * x2 + _rotation( 0, 1 ) * y2 + _rotation( 0, 2 ) * z2;
  y = _rotation( 1, 0 ) * x2 + _rotation( 1, 1 ) * y2 + _rotation( 1, 2 ) * z2;
  z = _rotation( 2, 0 ) * x2 + _rotation( 2, 1 ) * y2 + _rotation( 2, 2 ) * z2;*/
  x = _rotation( 0, 0 ) * x2 + _rotation( 1, 0 ) * y2 + _rotation( 2, 0 ) * z2;
  y = _rotation( 0, 1 ) * x2 + _rotation( 1, 1 ) * y2 + _rotation( 2, 1 ) * z2;
  z = _rotation( 0, 2 ) * x2 + _rotation( 1, 2 ) * y2 + _rotation( 2, 2 ) * z2;
}


void InertialDomainBox::buildTree( Tree & tr ) const
{
  DomainBox::buildTree( tr );

  vector<float>	vf;

  vf.push_back( _inertia( 0, 0 ) );
  vf.push_back( _inertia( 0, 1 ) );
  vf.push_back( _inertia( 0, 2 ) );
  vf.push_back( _inertia( 1, 0 ) );
  vf.push_back( _inertia( 1, 1 ) );
  vf.push_back( _inertia( 1, 2 ) );
  vf.push_back( _inertia( 2, 0 ) );
  vf.push_back( _inertia( 2, 1 ) );
  vf.push_back( _inertia( 2, 2 ) );

  tr.setSyntax( SIA_INERTIAL_DOMAIN_BOX );
  tr.setProperty( SIA_DOM_INERTIA, vf );

  vf.erase( vf.begin(), vf.end() );
  tr.setProperty( SIA_POINT_NUMBER, (int) _npoints );

  if( _tolMargin != 0 )
    tr.setProperty( SIA_TOLERENCE_MARGIN, _tolMargin );
  if( _gcattrib != SIA_REFGRAVITY_CENTER )
    tr.setProperty( SIA_GRAVITYCENTER_ATTRIBUTE, _gcattrib );

  if( _transfUpToDate )
    {
      vf.push_back( _gravity[ 0 ] );
      vf.push_back( _gravity[ 1 ] );
      vf.push_back( _gravity[ 2 ] );
      tr.setProperty( SIA_REFGRAVITY_CENTER, vf );
      vf.erase( vf.begin(), vf.end() );
      vf.push_back( _rotation( 0, 0 ) );
      vf.push_back( _rotation( 0, 1 ) );
      vf.push_back( _rotation( 0, 2 ) );
      vf.push_back( _rotation( 1, 0 ) );
      vf.push_back( _rotation( 1, 1 ) );
      vf.push_back( _rotation( 1, 2 ) );
      vf.push_back( _rotation( 2, 0 ) );
      vf.push_back( _rotation( 2, 1 ) );
      vf.push_back( _rotation( 2, 2 ) );
      tr.setProperty( SIA_ROTATION, vf );
      vf.erase( vf.begin(), vf.end() );
      vf.push_back( _eigenValues[ 0 ] );
      vf.push_back( _eigenValues[ 1 ] );
      vf.push_back( _eigenValues[ 2 ] );
      tr.setProperty( SIA_EIGENVALUES, vf );
    }
  else
    {
      unsigned	np = _npoints;
      if( np == 0 )
	np = 1;
      vf.push_back( _gravity[ 0 ] / np );
      vf.push_back( _gravity[ 1 ] / np );
      vf.push_back( _gravity[ 2 ] / np );
      tr.setProperty( SIA_REFGRAVITY_CENTER, vf );
    }
}


void InertialDomainBox::buildInertialDomBox( Tree*, Tree* tr )
{
  InertialDomainBox	*idb = new InertialDomainBox;

  vector<float>	vf;

  float		xmin, xmax, ymin, ymax, zmin, zmax, tm = 0;
  int		ndata;
  string	gcatt = SIA_REFGRAVITY_CENTER;

  //	partie DomainBox
  tr->setProperty( SIA_POINTER, (Domain *) idb );
  tr->getProperty( SIA_XMIN, xmin );
  tr->getProperty( SIA_XMAX, xmax );
  tr->getProperty( SIA_YMIN, ymin );
  tr->getProperty( SIA_YMAX, ymax );
  tr->getProperty( SIA_ZMIN, zmin );
  tr->getProperty( SIA_ZMAX, zmax );
  tr->getProperty( SIA_NDATA, ndata );
  tr->getProperty( SIA_TOLERENCE_MARGIN, tm );
  tr->getProperty( SIA_GRAVITYCENTER_ATTRIBUTE, gcatt );
  idb->setNData( ndata );
  idb->setDims( xmin, ymin, zmin, xmax, ymax, zmax );
  idb->setTolerenceMargin( tm );
  idb->setGravityCenterAttribute( gcatt );

  //	partie Inertial
  tr->getProperty( SIA_DOM_INERTIA, vf );
  idb->_inertia( 0, 0 ) = vf[0];
  idb->_inertia( 0, 1 ) = vf[1];
  idb->_inertia( 0, 2 ) = vf[2];
  idb->_inertia( 1, 0 ) = vf[3];
  idb->_inertia( 1, 1 ) = vf[4];
  idb->_inertia( 1, 2 ) = vf[5];
  idb->_inertia( 2, 0 ) = vf[6];
  idb->_inertia( 2, 1 ) = vf[7];
  idb->_inertia( 2, 2 ) = vf[8];

  tr->getProperty( SIA_REFGRAVITY_CENTER, vf );
  idb->_gravity[0] = vf[0];
  idb->_gravity[1] = vf[1];
  idb->_gravity[2] = vf[2];

  int	pn;

  tr->getProperty( SIA_POINT_NUMBER, pn );
  idb->_npoints = pn;

  if( tr->getProperty( SIA_ROTATION, vf ) )
    {
      idb->_rotation( 0, 0 ) = vf[0];
      idb->_rotation( 0, 1 ) = vf[1];
      idb->_rotation( 0, 2 ) = vf[2];
      idb->_rotation( 1, 0 ) = vf[3];
      idb->_rotation( 1, 1 ) = vf[4];
      idb->_rotation( 1, 2 ) = vf[5];
      idb->_rotation( 2, 0 ) = vf[6];
      idb->_rotation( 2, 1 ) = vf[7];
      idb->_rotation( 2, 2 ) = vf[8];

      tr->getProperty( SIA_EIGENVALUES, vf );
      idb->_eigenValues[0] = vf[0];
      idb->_eigenValues[1] = vf[1];
      idb->_eigenValues[2] = vf[2];

      idb->_transfUpToDate = true;
    }
  else
    {
      idb->_transfUpToDate = false;
      idb->_gravity[0] *= idb->_npoints;
      idb->_gravity[1] *= idb->_npoints;
      idb->_gravity[2] *= idb->_npoints;
    }
}


void InertialDomainBox::cubeTalairach( vector<vector<double> > & pts ) const
{
  vector<double>	pt;

  pts.erase( pts.begin(), pts.end() );
  pt.push_back( _rotation( 0, 0 ) * _xmin + _rotation( 0, 1 ) * _ymin
		+ _rotation( 0, 2 ) * _zmin + _gravity[0] );
  pt.push_back( _rotation( 1, 0 ) * _xmin + _rotation( 1, 1 ) * _ymin
		+ _rotation( 1, 2 ) * _zmin + _gravity[1] );
  pt.push_back( _rotation( 2, 0 ) * _xmin + _rotation( 2, 1 ) * _ymin
		+ _rotation( 2, 2 ) * _zmin + _gravity[2] );
  pts.push_back( pt );

  pt[0] = _rotation( 0, 0 ) * _xmax + _rotation( 0, 1 ) * _ymin
    + _rotation( 0, 2 ) * _zmin + _gravity[0];
  pt[1] = _rotation( 1, 0 ) * _xmax + _rotation( 1, 1 ) * _ymin
    + _rotation( 1, 2 ) * _zmin + _gravity[1];
  pt[2] = _rotation( 2, 0 ) * _xmax + _rotation( 2, 1 ) * _ymin
    + _rotation( 2, 2 ) * _zmin + _gravity[2];
  pts.push_back( pt );

  pt[0] = _rotation( 0, 0 ) * _xmax + _rotation( 0, 1 ) * _ymin
    + _rotation( 0, 2 ) * _zmax + _gravity[0];
  pt[1] = _rotation( 1, 0 ) * _xmax + _rotation( 1, 1 ) * _ymin
    + _rotation( 1, 2 ) * _zmax + _gravity[1];
  pt[2] = _rotation( 2, 0 ) * _xmax + _rotation( 2, 1 ) * _ymin
    + _rotation( 2, 2 ) * _zmax + _gravity[2];
  pts.push_back( pt );

  pt[0] = _rotation( 0, 0 ) * _xmin + _rotation( 0, 1 ) * _ymin
    + _rotation( 0, 2 ) * _zmax + _gravity[0];
  pt[1] = _rotation( 1, 0 ) * _xmin + _rotation( 1, 1 ) * _ymin
    + _rotation( 1, 2 ) * _zmax + _gravity[1];
  pt[2] = _rotation( 2, 0 ) * _xmin + _rotation( 2, 1 ) * _ymin
    + _rotation( 2, 2 ) * _zmax + _gravity[2];
  pts.push_back( pt );

  pt[0] = _rotation( 0, 0 ) * _xmin + _rotation( 0, 1 ) * _ymax
    + _rotation( 0, 2 ) * _zmax + _gravity[0];
  pt[1] = _rotation( 1, 0 ) * _xmin + _rotation( 1, 1 ) * _ymax
    + _rotation( 1, 2 ) * _zmax + _gravity[1];
  pt[2] = _rotation( 2, 0 ) * _xmin + _rotation( 2, 1 ) * _ymax
    + _rotation( 2, 2 ) * _zmax + _gravity[2];
  pts.push_back( pt );

  pt[0] = _rotation( 0, 0 ) * _xmin + _rotation( 0, 1 ) * _ymax
    + _rotation( 0, 2 ) * _zmin + _gravity[0];
  pt[1] = _rotation( 1, 0 ) * _xmin + _rotation( 1, 1 ) * _ymax
    + _rotation( 1, 2 ) * _zmin + _gravity[1];
  pt[2] = _rotation( 2, 0 ) * _xmin + _rotation( 2, 1 ) * _ymax
    + _rotation( 2, 2 ) * _zmin + _gravity[2];
  pts.push_back( pt );

  pt[0] = _rotation( 0, 0 ) * _xmax + _rotation( 0, 1 ) * _ymax
    + _rotation( 0, 2 ) * _zmin + _gravity[0];
  pt[1] = _rotation( 1, 0 ) * _xmax + _rotation( 1, 1 ) * _ymax
    + _rotation( 1, 2 ) * _zmin + _gravity[1];
  pt[2] = _rotation( 2, 0 ) * _xmax + _rotation( 2, 1 ) * _ymax
    + _rotation( 2, 2 ) * _zmin + _gravity[2];
  pts.push_back( pt );

  pt[0] = _rotation( 0, 0 ) * _xmax + _rotation( 0, 1 ) * _ymax
    + _rotation( 0, 2 ) * _zmax + _gravity[0];
  pt[1] = _rotation( 1, 0 ) * _xmax + _rotation( 1, 1 ) * _ymax
    + _rotation( 1, 2 ) * _zmax + _gravity[1];
  pt[2] = _rotation( 2, 0 ) * _xmax + _rotation( 2, 1 ) * _ymax
    + _rotation( 2, 2 ) * _zmax + _gravity[2];
    pts.push_back( pt );



    /*  pts.erase( pts.begin(), pts.end() );
  pt.push_back( _rotation( 0, 0 ) * _xmin + _rotation( 1, 0 ) * _ymin
		+ _rotation( 2, 0 ) * _zmin + _gravity[0] );
  pt.push_back( _rotation( 0, 1 ) * _xmin + _rotation( 1, 1 ) * _ymin
		+ _rotation( 2, 1 ) * _zmin + _gravity[1] );
  pt.push_back( _rotation( 0, 2 ) * _xmin + _rotation( 1, 2 ) * _ymin
		+ _rotation( 2, 2 ) * _zmin + _gravity[2] );
  pts.push_back( pt );

  pt[0] = _rotation( 0, 0 ) * _xmax + _rotation( 1, 0 ) * _ymin
    + _rotation( 2, 0 ) * _zmin + _gravity[0];
  pt[1] = _rotation( 0, 1 ) * _xmax + _rotation( 1, 1 ) * _ymin
    + _rotation( 2, 1 ) * _zmin + _gravity[1];
  pt[2] = _rotation( 0, 2 ) * _xmax + _rotation( 1, 2 ) * _ymin
    + _rotation( 2, 2 ) * _zmin + _gravity[2];
  pts.push_back( pt );

  pt[0] = _rotation( 0, 0 ) * _xmax + _rotation( 1, 0 ) * _ymin
    + _rotation( 2, 0 ) * _zmax + _gravity[0];
  pt[1] = _rotation( 0, 1 ) * _xmax + _rotation( 1, 1 ) * _ymin
    + _rotation( 2, 1 ) * _zmax + _gravity[1];
  pt[2] = _rotation( 0, 2 ) * _xmax + _rotation( 1, 2 ) * _ymin
    + _rotation( 2, 2 ) * _zmax + _gravity[2];
  pts.push_back( pt );

  pt[0] = _rotation( 0, 0 ) * _xmin + _rotation( 1, 0 ) * _ymin
    + _rotation( 2, 0 ) * _zmax + _gravity[0];
  pt[1] = _rotation( 0, 1 ) * _xmin + _rotation( 1, 1 ) * _ymin
    + _rotation( 2, 1 ) * _zmax + _gravity[1];
  pt[2] = _rotation( 0, 2 ) * _xmin + _rotation( 1, 2 ) * _ymin
    + _rotation( 2, 2 ) * _zmax + _gravity[2];
  pts.push_back( pt );

  pt[0] = _rotation( 0, 0 ) * _xmin + _rotation( 1, 0 ) * _ymax
    + _rotation( 2, 0 ) * _zmax + _gravity[0];
  pt[1] = _rotation( 0, 1 ) * _xmin + _rotation( 1, 1 ) * _ymax
    + _rotation( 2, 1 ) * _zmax + _gravity[1];
  pt[2] = _rotation( 0, 2 ) * _xmin + _rotation( 1, 2 ) * _ymax
    + _rotation( 2, 2 ) * _zmax + _gravity[2];
  pts.push_back( pt );

  pt[0] = _rotation( 0, 0 ) * _xmin + _rotation( 1, 0 ) * _ymax
    + _rotation( 2, 0 ) * _zmin + _gravity[0];
  pt[1] = _rotation( 0, 1 ) * _xmin + _rotation( 1, 1 ) * _ymax
    + _rotation( 2, 1 ) * _zmin + _gravity[1];
  pt[2] = _rotation( 0, 2 ) * _xmin + _rotation( 1, 2 ) * _ymax
    + _rotation( 2, 2 ) * _zmin + _gravity[2];
  pts.push_back( pt );

  pt[0] = _rotation( 0, 0 ) * _xmax + _rotation( 1, 0 ) * _ymax
    + _rotation( 2, 0 ) * _zmin + _gravity[0];
  pt[1] = _rotation( 0, 1 ) * _xmax + _rotation( 1, 1 ) * _ymax
    + _rotation( 2, 1 ) * _zmin + _gravity[1];
  pt[2] = _rotation( 0, 2 ) * _xmax + _rotation( 1, 2 ) * _ymax
    + _rotation( 2, 2 ) * _zmin + _gravity[2];
  pts.push_back( pt );

  pt[0] = _rotation( 0, 0 ) * _xmax + _rotation( 1, 0 ) * _ymax
    + _rotation( 2, 0 ) * _zmax + _gravity[0];
  pt[1] = _rotation( 0, 1 ) * _xmax + _rotation( 1, 1 ) * _ymax
    + _rotation( 2, 1 ) * _zmax + _gravity[1];
  pt[2] = _rotation( 0, 2 ) * _xmax + _rotation( 1, 2 ) * _ymax
    + _rotation( 2, 2 ) * _zmax + _gravity[2];
    pts.push_back( pt );*/
}




