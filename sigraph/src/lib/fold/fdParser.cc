
#include <si/fold/fdParser.h>
#include <si/fold/foldDescr.h>
#include <si/fold/foldDescr3.h>
#include <si/fold/foldDescr5.h>
#include <si/fold/interFoldDescr.h>
#include <si/fold/interFoldDescr5.h>
#include <si/fold/brainJuncDescr.h>
#include <graph/tree/tree.h>
#include <si/fold/fattrib.h>
#include <si/gyrus/gyrusattrib.h>
#include <si/gyrus/gyrusdescr.h>
#include <si/roi/roiattrib.h>
#include <si/roi/roidescr.h>

using namespace sigraph;
using namespace carto;
using namespace std;


FDParser::FDParser() : DescrParser()
{
  _factories[ SIA_FOLD_DESCRIPTOR ] = buildFDescr;
  _factories[ SIA_FOLD_DESCRIPTOR2 ] = buildFDescr2;
  _factories[ SIA_FOLD_DESCRIPTOR3 ] = buildFDescr3;
  _factories[ SIA_FOLD_DESCRIPTOR4 ] = buildFDescr4;
  _factories[ SIA_FOLD_DESCRIPTOR5 ] = buildFDescr5;
  _factories[ SIA_INTER_FOLD_DESCRIPTOR ] = buildIFDescr;
  _factories[ SIA_INTER_FOLD_DESCRIPTOR2 ] = buildIFDescr2;
  _factories[ SIA_INTER_FOLD_DESCRIPTOR4 ] = buildIFDescr4;
  _factories[ SIA_INTER_FOLD_DESCRIPTOR5 ] = buildIFDescr5;
  _factories[ SIA_BRAIN_JUNCTION_DESCRIPTOR ] = buildBJDescr;
  _factories[ SIA_GYRUS_DESCRIPTOR ] = buildGyrusDescr;
  _factories[ SIA_ROI_DESCRIPTOR ] = buildRoiDescr;
}


TreePostParser::FactorySet FDParser::factories()
{
  return( _factories );
}


void FDParser::buildFDescr( AttributedObject* parent, Tree* t, 
			    const string & )
{
  FoldDescr	*fd = new FoldDescr;
  t->setProperty( SIA_POINTER, (CliqueDescr *) fd );

  float		ls;
  int		nnorm;
  vector<float>	nrm;

  if( t->getProperty( "nstats_normal", nnorm ) && 
      t->getProperty( SIA_NORMAL, nrm ) )
    {
      fd->setNStats( nnorm );
      fd->setNormal( nrm[0], nrm[1], nrm[2] );
    }

  if( t->getProperty( "limit_size", ls ) )
    fd->setLimitSize( ls );
  else fd->setLimitSize( 0 );

  DescrParser::parseDescr( parent, t, fd );
}


void FDParser::buildFDescr2( AttributedObject* parent, Tree* t, 
			     const string & )
{
  FoldDescr2	*fd = new FoldDescr2;
  t->setProperty( SIA_POINTER, (CliqueDescr *) fd );

  int		nnorm;
  vector<float>	nrm;

  if( t->getProperty( SIA_NSTATS_NORMAL, nnorm ) && 
      t->getProperty( SIA_NORMAL, nrm ) )
    {
      fd->setNStats( nnorm );
      fd->setNormal( nrm[0], nrm[1], nrm[2] );
    }
  if( t->getProperty( SIA_NSTATS_E1E2, nnorm ) 
      && t->getProperty( SIA_E1E2, nrm ) )
    {
      fd->setE12Stats( nnorm );
      fd->setE1E2( nrm[0], nrm[1], nrm[2] );
    }
  if( t->getProperty( SIA_NSTATS_DIR, nnorm ) 
      && t->getProperty( SIA_DIRECTION, nrm ) )
    {
      fd->setDirStats( nnorm );
      fd->setDirection( nrm[0], nrm[1], nrm[2] );
    }

  DescrParser::parseDescr( parent, t, fd );
}


void FDParser::buildFDescr3( AttributedObject* parent, Tree* t, 
			     const string & )
{
  FoldDescr3	*fd = new FoldDescr3;
  t->setProperty( SIA_POINTER, (CliqueDescr *) fd );

  int		nnorm;
  vector<float>	nrm;

  if( t->getProperty( SIA_NSTATS_NORMAL, nnorm ) && 
      t->getProperty( SIA_NORMAL, nrm ) )
    {
      fd->setNStats( nnorm );
      fd->setNormal( nrm[0], nrm[1], nrm[2] );
    }
  if( t->getProperty( SIA_NSTATS_E1E2, nnorm ) 
      && t->getProperty( SIA_E1E2, nrm ) )
    {
      fd->setE12Stats( nnorm );
      fd->setE1E2( nrm[0], nrm[1], nrm[2] );
    }
  if( t->getProperty( SIA_NSTATS_DIR, nnorm ) 
      && t->getProperty( SIA_DIRECTION, nrm ) )
    {
      fd->setDirStats( nnorm );
      fd->setDirection( nrm[0], nrm[1], nrm[2] );
    }

  DescrParser::parseDescr( parent, t, fd );
}


void FDParser::buildFDescr4( AttributedObject* parent, Tree* t, 
			     const string & )
{
  FoldDescr4	*fd = new FoldDescr4;
  t->setProperty( SIA_POINTER, (CliqueDescr *) fd );

  int		nnorm;
  vector<float>	nrm;

  if( t->getProperty( SIA_NSTATS_NORMAL, nnorm ) && 
      t->getProperty( SIA_NORMAL, nrm ) )
    {
      fd->setNStats( nnorm );
      fd->setNormal( nrm[0], nrm[1], nrm[2] );
    }
  if( t->getProperty( SIA_NSTATS_E1E2, nnorm ) 
      && t->getProperty( SIA_E1E2, nrm ) )
    {
      fd->setE12Stats( nnorm );
      fd->setE1E2( nrm[0], nrm[1], nrm[2] );
    }
  if( t->getProperty( SIA_NSTATS_DIR, nnorm ) 
      && t->getProperty( SIA_DIRECTION, nrm ) )
    {
      fd->setDirStats( nnorm );
      fd->setDirection( nrm[0], nrm[1], nrm[2] );
    }

  DescrParser::parseDescr( parent, t, fd );
}


void FDParser::buildFDescr5( AttributedObject* parent, Tree* t, 
			     const string & )
{
  FoldDescr4	*fd = new FoldDescr5;
  t->setProperty( SIA_POINTER, (CliqueDescr *) fd );

  int		nnorm;
  vector<float>	nrm;

  if( t->getProperty( SIA_NSTATS_NORMAL, nnorm ) && 
      t->getProperty( SIA_NORMAL, nrm ) )
    {
      fd->setNStats( nnorm );
      fd->setNormal( nrm[0], nrm[1], nrm[2] );
    }
  if( t->getProperty( SIA_NSTATS_E1E2, nnorm ) 
      && t->getProperty( SIA_E1E2, nrm ) )
    {
      fd->setE12Stats( nnorm );
      fd->setE1E2( nrm[0], nrm[1], nrm[2] );
    }
  if( t->getProperty( SIA_NSTATS_DIR, nnorm ) 
      && t->getProperty( SIA_DIRECTION, nrm ) )
    {
      fd->setDirStats( nnorm );
      fd->setDirection( nrm[0], nrm[1], nrm[2] );
    }

  DescrParser::parseDescr( parent, t, fd );
}


void FDParser::buildIFDescr( AttributedObject* parent, Tree* t, 
			     const string & )
{
  InterFoldDescr	*fd = new InterFoldDescr;
  t->setProperty( SIA_POINTER, (CliqueDescr *) fd );
  parseDescr( parent, t, fd );
}


void FDParser::buildIFDescr2( AttributedObject* parent, Tree* t, 
			      const string & )
{
  InterFoldDescr2	*fd = new InterFoldDescr2;
  t->setProperty( SIA_POINTER, (CliqueDescr *) fd );
  parseDescr( parent, t, fd );
}


void FDParser::buildIFDescr4( AttributedObject* parent, Tree* t, 
			      const string & )
{
  InterFoldDescr4	*fd = new InterFoldDescr4;
  t->setProperty( SIA_POINTER, (CliqueDescr *) fd );
  parseDescr( parent, t, fd );
}


void FDParser::buildIFDescr5( AttributedObject* parent, Tree* t, 
			      const string & )
{
  // ### TODO ###
  InterFoldDescr5	*fd = new InterFoldDescr5;
  t->setProperty( SIA_POINTER, (CliqueDescr *) fd );
  parseDescr( parent, t, fd );
}


void FDParser::buildBJDescr( AttributedObject* parent, Tree* t, 
			     const string & )
{
  BrainJuncDescr		*bd = new BrainJuncDescr;
  t->setProperty( SIA_POINTER, (CliqueDescr *) bd );
  parseDescr( parent, t, bd );
}


void FDParser::buildGyrusDescr( AttributedObject* parent, Tree* t, 
                                const string & )
{
  GyrusDescr		*bd = new GyrusDescr;
  t->setProperty( SIA_POINTER, (CliqueDescr *) bd );
  parseDescr( parent, t, bd );
}


void FDParser::buildRoiDescr( AttributedObject* parent, Tree* t, 
                              const string & )
{
  RoiDescr		*bd = new RoiDescr;
  t->setProperty( SIA_POINTER, (CliqueDescr *) bd );
  FDParser::parseDescr( parent, t, bd );
}


void FDParser::registerFactory( const string & name, const Factory & fac )
{
  _factories[ name ] = fac;
}


