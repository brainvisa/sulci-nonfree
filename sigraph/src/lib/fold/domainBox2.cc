#include <si/fold/domainBox2.h>
#include <si/fold/fattrib.h>
#include <graph/tree/tree.h>
#include <graph/graph/graph.h>
#include <vector>
#include <iostream>

using namespace sigraph;
using namespace std;


void DomainBox2::learn( const Vertex* v, const Graph* )
{
  vector<float>	bmin, bmax;

  if( v->getProperty( SIA_TAL_BOUNDINGBOX_MIN, bmin ) 
      && v->getProperty( SIA_TAL_BOUNDINGBOX_MAX, bmax ) )
    {
      if( _ndata == 0 )
	{
	  _xmin = bmin[0];
	  _xmax = bmax[0];
	  _ymin = bmin[1];
	  _ymax = bmax[1];
	  _zmin = bmin[2];
	  _zmax = bmax[2];
	}
      else
	{
	  if( _xmin > bmin[0] )
	    _xmin = bmin[0];
	  if( _ymin > bmin[1] )
	    _ymin = bmin[1];
	  if( _zmin > bmin[2] )
	    _zmin = bmin[2];
	  if( _xmax < bmax[0] )
	    _xmax = bmax[0];
	  if( _ymax < bmax[1] )
	    _ymax = bmax[1];
	  if( _zmax < bmax[2] )
	    _zmax = bmax[2];
	}
    }
  ++_ndata;
}


void DomainBox2::buildTree( Tree & tr ) const
{
  DomainBox::buildTree( tr );

  tr.setSyntax( SIA_DOMAIN_BOX2 );
}


void DomainBox2::buildDomBox2( Tree*, Tree* ao )
{
  if( ao->childrenSize() != 0 )
    {
      cerr << "warning : Domain Box with children\n";
    }

  DomainBox2	*fdb = new DomainBox2;
  float		xmin, xmax, ymin, ymax, zmin, zmax;
  int		ndata;

  ao->setProperty( SIA_POINTER, (Domain *) fdb );
  ao->getProperty( SIA_XMIN, xmin );
  ao->getProperty( SIA_XMAX, xmax );
  ao->getProperty( SIA_YMIN, ymin );
  ao->getProperty( SIA_YMAX, ymax );
  ao->getProperty( SIA_ZMIN, zmin );
  ao->getProperty( SIA_ZMAX, zmax );
  ao->getProperty( SIA_NDATA, ndata );
  fdb->setNData( ndata );
  fdb->setDims( xmin, ymin, zmin, xmax, ymax, zmax );
}


