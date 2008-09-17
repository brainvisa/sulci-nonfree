#include <si/fold/domainBox.h>
#include <graph/graph/graph.h>
#include <aims/bucket/bucket.h>
#include <aims/def/def_g.h>
#include <si/fold/fattrib.h>
#include <graph/tree/tree.h>

using namespace sigraph;
using namespace aims;
using namespace carto;
using namespace std;


DomainBox::DomainBox() 
  : AdapDomain(), _xmin( 0 ), _ymin( 0 ), _zmin( 0 ), 
    _xmax( 0 ), _ymax( 0 ), _zmax( 0 ), _gcattrib( SIA_REFGRAVITY_CENTER )
{
}


DomainBox::DomainBox( const DomainBox & dom ) 
  : AdapDomain( dom ), _xmin( dom._xmin ), _ymin( dom._ymin ), 
    _zmin( dom._zmin ), _xmax( dom._xmax ), _ymax( dom._ymax ), 
    _zmax( dom._zmax ), _gcattrib( SIA_REFGRAVITY_CENTER )
{
}


void DomainBox::learn( const Vertex* v, const Graph* g )
{
  if( !v->hasProperty( SIA_SS_BUCKET ) )
    return;

  rc_ptr<BucketMap<Void> >			bck;
  BucketMap<Void>::Bucket::const_iterator	ib, fb;
  vector< vector<float> >			mrot;
  vector<float>					scale, transl;
  vector<float>					vsz;
  float						x, y, z, xv, yv, zv;

  v->getProperty( SIA_SS_BUCKET, bck );

  if( g )
    {
      g->getProperty( SIA_0_8_TALAIRACH_M_ROTATION, mrot );
      g->getProperty( SIA_TALAIRACH_SCALE, scale );
      g->getProperty( SIA_TALAIRACH_TRANSLATION, transl );
      g->getProperty( SIA_VOXEL_SIZE, vsz );
    }

  if( mrot.size() != 3 || scale.size() != 3 || transl.size() != 3 )
    {
      cout << "No Talairach info. ";
      mrot.erase( mrot.begin(), mrot.end() );
      scale.erase( scale.begin(), scale.end() );
      transl.erase( transl.begin(), transl.end() );

      transl.push_back( 0. );
      transl.push_back( 0. );
      transl.push_back( 0. );

      scale.push_back( 1. );
      scale.push_back( 1. );
      scale.push_back( 1. );

      mrot.push_back( vector<float>() );
      mrot.push_back( vector<float>() );
      mrot.push_back( vector<float>() );

      mrot[0].push_back( 1. );
      mrot[0].push_back( 0. );
      mrot[0].push_back( 0. );

      mrot[1].push_back( 0. );
      mrot[1].push_back( 1. );
      mrot[1].push_back( 0. );

      mrot[2].push_back( 0. );
      mrot[2].push_back( 0. );
      mrot[2].push_back( 1. );
    }

  if( vsz.size() != 3 )
    {
      cerr << "Bad voxel_size vector size ! : " << vsz.size() 
	   << ", should be 3.\n";
      while( vsz.size() < 3 )
	vsz.push_back( 1. );
    }

  cout << "learning box from bucket of " << (*bck)[0].size() << " elements.\n";

  ib = (*bck)[0].begin();
  fb = (*bck)[0].end();

  if( ib != fb && _ndata == 0 )
    {
      const AimsVector<short,3> & loc = ib->first;

      //	multiplier les coord. par les voxel size
      xv = loc[0] * vsz[0] + transl[0];
      yv = loc[1] * vsz[1] + transl[1];
      zv = loc[2] * vsz[2] + transl[2];
      
      _xmin = _xmax = ( mrot[0][0] * xv + mrot[0][1] * yv 
			+ mrot[0][2] * zv ) * scale[0];
      _ymin = _ymax = ( mrot[1][0] * xv + mrot[1][1] * yv 
			+ mrot[1][2] * zv ) * scale[1];
      _zmin = _zmax = ( mrot[2][0] * xv + mrot[2][1] * yv 
			+ mrot[2][2] * zv ) * scale[2];

      ++ib;
    }

  for( ; ib!=fb; ++ib )
    {
      const AimsVector<short,3> & loc = ib->first;
      //	multiplier les coord. par les voxel size
      xv = loc[0] * vsz[0] + transl[0];
      yv = loc[1] * vsz[1] + transl[1];
      zv = loc[2] * vsz[2] + transl[2];

      x = ( mrot[0][0] * xv + mrot[0][1] * yv 
	    + mrot[0][2] * zv ) * scale[0];
      y = ( mrot[1][0] * xv + mrot[1][1] * yv 
	    + mrot[1][2] * zv ) * scale[1];
      z = ( mrot[2][0] * xv + mrot[2][1] * yv 
	    + mrot[2][2] * zv ) * scale[2];

      if( x < _xmin ) _xmin = x;
      if( x > _xmax ) _xmax = x;
      if( y < _ymin ) _ymin = y;
      if( y > _ymax ) _ymax = y;
      if( z < _zmin ) _zmin = z;
      if( z > _zmax ) _zmax = z;
    }

  ++_ndata;
}


void DomainBox::reset()
{
  AdapDomain::reset();

  _xmin = _ymin = _zmin = _xmax = _ymax = _zmax = 0;
}


bool DomainBox::canBeFound( const Vertex* v, const Graph* )
{
  vector<float>	g;

  if( !v->getProperty( _gcattrib, g ) )
    return( false );

  return( g[0] >= _xmin && g[0] <= _xmax && g[1] >= _ymin && g[1] <= _ymax 
	  && g[2] >= _zmin && g[2] <= _zmax );
}


void DomainBox::talairach(  float & v1, float & v2, float & v3, 
			    const Graph* g ) const
{
  vector< vector<float> >		mrot;
  vector<float>				scale, transl, vsz;
  float					x, y, z, xv, yv, zv;

  if( g )
    {
      g->getProperty( SIA_0_8_TALAIRACH_M_ROTATION, mrot );
      g->getProperty( SIA_TALAIRACH_SCALE, scale );
      g->getProperty( SIA_TALAIRACH_TRANSLATION, transl );
      g->getProperty( SIA_VOXEL_SIZE, vsz );
    }

  if( vsz.size() != 3 )
    {
      cerr << "Bad voxel_size vector size ! : " << vsz.size() 
	   << ", should be 3.\n";
      while( vsz.size() < 3 )
	vsz.push_back( 1. );
    }

  if( mrot.size() != 3 || scale.size() != 3 || transl.size() != 3 )
    cout << "No Talairach info. ";
  else
    {
      //	multiplier les coord. par les voxel size
      xv = v1 * vsz[0];
      yv = v2 * vsz[1];
      zv = v3 * vsz[2];
      
      x = ( mrot[0][0] * xv + mrot[0][1] * yv + mrot[0][2] * zv + transl[0] ) 
	* scale[0];
      y = ( mrot[1][0] * xv + mrot[1][1] * yv + mrot[1][2] * zv + transl[1] ) 
	* scale[1];
      z = ( mrot[2][0] * xv + mrot[2][1] * yv + mrot[2][2] * zv + transl[2] ) 
	* scale[2];

      v1 = x;
      v2 = y;
      v3 = z;
    }
}


void DomainBox::buildTree( Tree & tr ) const
{
  AdapDomain::buildTree( tr );

  tr.setSyntax( SIA_DOMAIN_BOX );
  tr.setProperty( SIA_XMIN, (float) _xmin );
  tr.setProperty( SIA_XMAX, (float) _xmax );
  tr.setProperty( SIA_YMIN, (float) _ymin );
  tr.setProperty( SIA_YMAX, (float) _ymax );
  tr.setProperty( SIA_ZMIN, (float) _zmin );
  tr.setProperty( SIA_ZMAX, (float) _zmax );
  if( _gcattrib != SIA_REFGRAVITY_CENTER )
    tr.setProperty( SIA_GRAVITYCENTER_ATTRIBUTE, _gcattrib );
}


void DomainBox::buildDomBox( Tree*, Tree* ao )
{
  if( ao->size() != 0 )
    {
      cerr << "warning : Domain Box with children\n";
    }

  DomainBox	*fdb = new DomainBox;
  float		xmin, xmax, ymin, ymax, zmin, zmax;
  int		ndata;
  string	gcatt = SIA_REFGRAVITY_CENTER;

  ao->setProperty( SIA_POINTER, (Domain *) fdb );
  ao->getProperty( SIA_XMIN, xmin );
  ao->getProperty( SIA_XMAX, xmax );
  ao->getProperty( SIA_YMIN, ymin );
  ao->getProperty( SIA_YMAX, ymax );
  ao->getProperty( SIA_ZMIN, zmin );
  ao->getProperty( SIA_ZMAX, zmax );
  ao->getProperty( SIA_NDATA, ndata );
  ao->getProperty( SIA_GRAVITYCENTER_ATTRIBUTE, gcatt );
  fdb->setNData( ndata );
  fdb->setDims( xmin, ymin, zmin, xmax, ymax, zmax );
  fdb->setGravityCenterAttribute( gcatt );
}


void DomainBox::cubeTalairach( vector<vector<double> > & pts ) const
{
  vector<double>	pt;
  pt.push_back( _xmin );
  pt.push_back( _ymin );
  pt.push_back( _zmin );
  pts.push_back( pt );
  pt[0] = _xmax;
  pts.push_back( pt );
  pt[2] = _zmax;
  pts.push_back( pt );
  pt[0] = _xmin;
  pts.push_back( pt );
  pt[1] = _ymax;
  pts.push_back( pt );
  pt[2] = _zmin;
  pts.push_back( pt );
  pt[0] = _xmax;
  pts.push_back( pt );
  pt[2] = _zmax;
  pts.push_back( pt );
}


void DomainBox::setGravityCenterAttribute( const string & att )
{
  _gcattrib = att;
}


