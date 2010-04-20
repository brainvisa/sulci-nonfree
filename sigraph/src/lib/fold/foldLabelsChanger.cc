/*
 *  Copyright (C) 1998-2004 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 */

#include <si/fold/foldLabelsChanger.h>
#include <si/fold/foldCache.h>
#include <si/fold/foldDescr.h>
#include <si/fold/foldDescr2.h>
#include <si/fold/interFoldDescr.h>
#include <si/fold/interFoldCache.h>
#include <si/fold/fattrib.h>
#include <si/graph/vertexclique.h>
#include <cartobase/exception/assert.h>
#include <math.h>

using namespace sigraph;
using namespace std;


double FoldLabelsChanger::constrainedNoise( Clique* cl, double & outp, 
					    const set<string> & sl, 
					    const string & vl )
{
  string	mtype;
  Graph	*gr;
  cl->getProperty( SIA_GRAPH, gr );
  gr->getProperty( SIA_VERSION, _version );

  if( cl->getProperty( SIA_MODEL_TYPE, mtype ) && mtype == SIV_RANDOM_EDGE )
    {
      if( _version == SIV_VERSION_0_8 )
	return( noiseIFDescr( cl, outp, sl, vl ) );
      else
	return( noiseIFDescr2( cl, outp, sl, vl ) );
    }
  else
    {
      if( _version == SIV_VERSION_0_8 )
	return( noiseFoldDescr( cl, outp, sl, vl ) );
      else
	return( noiseFoldDescr2( cl, outp, sl, vl ) );
    }
}


double FoldLabelsChanger::noiseFoldDescr( Clique* cl, double & outp, 
					  const set<string> & sl, 
					  const string & vl )
{
  VertexClique	*vcl = (VertexClique *) cl;
  //cout << "FoldLabelsChanger\n";
  double	dist;
  unsigned	ccjold, i;
  CliqueCache	*cch = 0, *ccho = 0;

  cl->getProperty( SIA_ORIGINAL_CACHE, ccho );
  /*if( ccho ) cout << "original_cache trouv�\n";
    else cout << "Pas de original_cache\n";*/
  cl->getProperty( SIA_CACHE, cch );
  /*if( cch ) cout << "cache trouv�\n";
    else cout << "Pas de original_cache\n";*/

  FoldCache	*fco = 0, *fc = 0;

  _edge = false;

  //cout << "Random vertex\n";

  if( ccho )
    {
      fco = dynamic_cast<FoldCache *>( ccho );
      ASSERT( fco );
    }

  if( cch )
    {
      fc = dynamic_cast<FoldCache *>( cch );
      ASSERT( fc );
      //cout << "FoldCache\n";
      for( i=0; i<fco->subVecValid.size(); ++i )
	fco->subVecValid[i] = false;
    }
  //else cout << "cache pas bon\n";

  string	label;

  ASSERT( cl->getProperty( SIA_LABEL, label ) );

  if( fco && ( fco->vecValid 
      || ( fco->subVecValid.size() > FoldDescr::JUNC_CC && 
	   fco->subVecValid[ FoldDescr::JUNC_CC ] ) ) )
    ccjold = (unsigned) fco->inputVector[ FoldDescr::JUNC_CC ];
  else
    ccjold = vcl->connectivity( label, 0, SIA_JUNCTION_SYNTAX );

  //cout << "noise...\n";
  dist = LabelsChanger::constrainedNoise( cl, outp, sl, vl );
  //cout << "OK\n";

  unsigned	ccj = vcl->connectivity( label, 0, 
					SIA_JUNCTION_SYNTAX );
  if( ccj != ccjold )
    dist += 10000;	// distance prohibitive

  if( fc )
    {
      while( fc->inputVector.size() < FoldDescr::END )
	fc->inputVector.push_back( 0 );
      while( fc->subVecValid.size() < FoldDescr::END )
	fc->subVecValid.push_back( false );
      fc->inputVector[ FoldDescr::JUNC_CC ] = ccj;
      fc->subVecValid[ FoldDescr::JUNC_CC ] = true;
      /*cout << "sizeof( FoldCache ) : " << sizeof( *fc ) << ", " 
	<< sizeof( FoldCache ) << endl;
	cout << "cache mis � jour.\n";*/
    }
  //else cout << "cache non trouv�\n";

  dist = ( dist > 60 ? dist - 60 : 0 );

  //outp = output( outp, dist );

  return( dist );
}


double FoldLabelsChanger::noiseFoldDescr2( Clique* cl, double & outp, 
					   const set<string> & sl, 
					   const string & vl )
{
  const VertexClique	*vcl = (const VertexClique *) cl;
  double	dist;
  unsigned	ccold, ccncold, i;
  CliqueCache	*cch = 0, *ccho = 0;

  cl->getProperty( SIA_ORIGINAL_CACHE, ccho );
  cl->getProperty( SIA_CACHE, cch );

  FoldCache	*fco = 0, *fc = 0;

  _edge = false;

  if( ccho )
    {
      fco = dynamic_cast<FoldCache *>( ccho );
      ASSERT( fco );
    }

  if( cch )
    {
      fc = dynamic_cast<FoldCache *>( cch );
      ASSERT( fc );
      for( i=0; i<fco->subVecValid.size(); ++i )
	fco->subVecValid[i] = false;
    }

  string	label;
  cl->getProperty( SIA_LABEL, label );
  set<Vertex *>	sv = vcl->getVerticesWith( SIA_LABEL, label );
  set<string>	csynt;

  csynt.insert( SIA_JUNCTION_SYNTAX );
  csynt.insert( SIA_PLI_DE_PASSAGE_SYNTAX );

  ASSERT( cl->getProperty( SIA_LABEL, label ) );

  if( fco && ( fco->vecValid 
               || ( fco->subVecValid.size() > FoldDescr2::NCC_NOT_CORTICAL 
                    && fco->subVecValid[ FoldDescr2::NCC ] 
                    && fco->subVecValid[ FoldDescr2::NCC_NOT_CORTICAL ] ) ) )
    {
      ccold = (unsigned) fco->inputVector[ FoldDescr2::NCC ];
      ccncold = (unsigned) fco->inputVector[ FoldDescr2::NCC_NOT_CORTICAL ];
    }
  else
    {
      ccold = vcl->connectivity( sv );
      ccncold = vcl->connectivity( sv, 0, csynt );
    }

  dist = LabelsChanger::constrainedNoise( cl, outp, sl, vl );

  //	nouveaux noeuds avec le bon label
  sv = vcl->getVerticesWith( SIA_LABEL, label );

  //	nouvelles CC
  unsigned	cc = vcl->connectivity( sv );
  if( cc != ccold )
    dist += 10000;	// distance prohibitive
  unsigned	ccnc = vcl->connectivity( sv, 0, csynt );
  if( ccnc != ccncold )
    dist += 10000;

  if( fc )
    {
      while( fc->inputVector.size() < FoldDescr2::END )
	fc->inputVector.push_back( 0 );
      while( fc->subVecValid.size() < FoldDescr2::END )
	fc->subVecValid.push_back( false );
      fc->inputVector[ FoldDescr2::NCC ] = cc;
      fc->subVecValid[ FoldDescr2::NCC ] = true;
      fc->inputVector[ FoldDescr2::NCC_NOT_CORTICAL ] = cc;
      fc->subVecValid[ FoldDescr2::NCC_NOT_CORTICAL ] = true;
    }

  //dist = ( dist > 60 ? dist - 60 : 0 );

  outp = output( outp, dist );

  return( dist );
}


double FoldLabelsChanger::noiseIFDescr( Clique* cl, double & outp, 
					const set<string> & sl, 
					const string & vl )
{
  //cout << "FoldLabelsChanger\n";
  double	dist;
  unsigned	i;
  CliqueCache	*cch = 0, *ccho = 0;

  cl->getProperty( SIA_ORIGINAL_CACHE, ccho );
  /*if( ccho ) cout << "original_cache trouv�\n";
    else cout << "Pas de original_cache\n";*/
  cl->getProperty( SIA_CACHE, cch );
  /*if( cch ) cout << "cache trouv�\n";
    else cout << "Pas de original_cache\n";*/

  InterFoldCache	*ifco = 0, *ifc = 0;

  //cout << "Random edge\n";

  _edge = true;
  cl->getProperty( SIA_LABEL1, _label1 );
  cl->getProperty( SIA_LABEL2, _label2 );
  dist = 0;

  if( ccho )
    {
      ifco = dynamic_cast<InterFoldCache *>( ccho );
      ASSERT( ifco );
    }

  if( cch )
    {
      ifc = dynamic_cast<InterFoldCache *>( cch );
      ASSERT( ifc );
      //cout << "InterFoldCache\n";
      for( i=0; i<ifc->subVecValid.size(); ++i )
	ifc->subVecValid[i] = false;
    }

  //cout << "noise...\n";
  for( int i=0; i<3 && dist==0; ++i ) // 3 essais avant de laisser tomber
    dist = LabelsChanger::constrainedNoise( cl, outp, sl, vl );
  //if( dist == 0 )
  //cout << "dist rel 0" << endl;
  //cout << "OK. dist = " << dist << endl;

  if( ifc )
    {
      while( ifc->inputVector.size() < InterFoldDescr::END )
	ifc->inputVector.push_back( 0 );
      while( ifc->subVecValid.size() < InterFoldDescr::END )
	ifc->subVecValid.push_back( false );
      //cout << "cache mis � jour.\n";
    }
  //else cout << "cache pas trouv�\n";
  return( dist );
}


double FoldLabelsChanger::noiseIFDescr2( Clique* cl, double & outp, 
					 const set<string> & sl, 
					 const string & vl )
{
  double	dist;
  unsigned	i;
  CliqueCache	*cch = 0, *ccho = 0;

  cl->getProperty( SIA_ORIGINAL_CACHE, ccho );
  cl->getProperty( SIA_CACHE, cch );

  InterFoldCache	*ifco = 0, *ifc = 0;

  _edge = true;
  cl->getProperty( SIA_LABEL1, _label1 );
  cl->getProperty( SIA_LABEL2, _label2 );
  dist = 0;

  if( ccho )
    {
      ifco = dynamic_cast<InterFoldCache *>( ccho );
      ASSERT( ifco );
    }

  if( cch )
    {
      ifc = dynamic_cast<InterFoldCache *>( cch );
      ASSERT( ifc );
      for( i=0; i<ifc->subVecValid.size(); ++i )
	ifc->subVecValid[i] = false;
    }

  for( int i=0; i<3 && dist==0; ++i ) // 3 essais avant de laisser tomber
    dist = LabelsChanger::constrainedNoise( cl, outp, sl, vl );

  if( ifc )
    {
      while( ifc->inputVector.size() < InterFoldDescr::END )
	ifc->inputVector.push_back( 0 );
      while( ifc->subVecValid.size() < InterFoldDescr::END )
	ifc->subVecValid.push_back( false );
    }

  outp = output( outp, dist );

  return( dist );
}


double FoldLabelsChanger::distance( Vertex* v, const string & oldlabel, 
				    const string & newlabel, CliqueCache* cc )
{
  if( _edge )
    return( edgeDist( v, oldlabel, newlabel, cc ) );

  float	size = 0;	// le =0 c'est juste pour eviter un warning...

  if( v->getProperty( SIA_SIZE, size ) )
    {
      FoldCache	*fc = dynamic_cast<FoldCache *>( cc );

      if( fc )
	{
	  unsigned	iend, isize;

	  if( _version == SIV_VERSION_0_8 )
	    {
	      iend = FoldDescr::END;
	      isize = FoldDescr::SIZE;
	    }
	  else
	    {
	      iend = FoldDescr2::END;
	      isize = FoldDescr2::SIZE;
	    }

	  fc->vecValid = false;
	  if( fc->inputVector.size() != iend )
	    for( unsigned i=fc->inputVector.size(); i<iend; ++i )
	      fc->inputVector.push_back( 0 );
	  if( fc->subVecValid.size() != iend )
	    for( unsigned i=fc->subVecValid.size(); i<iend; ++i )
	      fc->subVecValid.push_back( false );
	  if( newlabel == SIV_VOID_LABEL )
	    fc->inputVector[ isize ] += size;
	  else
	    fc->inputVector[ isize ] -= size;
	  fc->subVecValid[ isize ] = true;
	}

      return( size );
    }
  else
    return( 0 );
}


double FoldLabelsChanger::edgeDist( Vertex* v, const string & oldlabel, 
				    const string & newlabel, CliqueCache* cc )
{
  Vertex::const_iterator	ie, fe = v->end();
  Edge::const_iterator		iv;
  string			label, label1, label2 = "";
  float				size = 0;
  double			sc1 = 0, sc2 = 0, sj1 = 0, sj2 = 0;
  double			dist=0;
  //	"Facteur de Mangin" ou M-Factor (constante universelle)
  //  const double			ManginFactor = 10.;
  InterFoldCache		*ifc = 0;
  enum				Type { ADD, REMOVE, CHANGE };
  Type				t;
  unsigned			first;

  if( cc )
    ifc = dynamic_cast<InterFoldCache *>( cc );

  v->getProperty( SIA_SIZE, size );

  if( oldlabel == _label1 )
    {
      first = 0;
      if( newlabel == _label2 )
	{
	  t = CHANGE;
	  label1 = _label1;
	  label2 = _label2;
	  if( ifc )
	    {
	      ifc->cSize1 -= size;
	      ifc->cSize2 += size;
	    }
	}
      else
	{
	  t = REMOVE;
	  label1 = _label2;
	  label2 = _label1;
	  if( ifc )
	    ifc->cSize1 -= size;
	}
    }
  else if( oldlabel == _label2 )
    {
      first = 1;
      if( newlabel == _label1 )
	{
	  t = CHANGE;
	  label1 = _label2;
	  label2 = _label1;
	  if( ifc )
	    {
	      ifc->cSize1 += size;
	      ifc->cSize2 -= size;
	    }
	}
      else
	{
	  t = REMOVE;
	  label1 = _label1;
	  label2 = _label2;
	  if( ifc )
	    ifc->cSize2 -= size;
	}
    }
  else	// ajout: old = void
    {
      t = ADD;
      if( newlabel == label1 )
	{
	  first = 0;
	  label1 = _label2;
	  label2 = _label1;
	  if( ifc )
	    ifc->cSize1 += size;
	}
      else	// normalement label2 != void
	{
	  first = 1;
	  label1 = _label1;
	  label2 = _label2;
	  if( ifc )
	    ifc->cSize2 += size;
	}
    }

  for( ie=v->begin(); ie!=fe; ++ie )
    {
      iv = (*ie)->begin();
      if( *iv == v )
	++iv;

      if( (*iv)->getProperty( SIA_LABEL, label ) )
	{
	  if( label==label1 )
	    {
	      (*ie)->getProperty( SIA_SIZE, size );
	      if( (*ie)->getSyntax() == SIA_JUNCTION_SYNTAX )
		sj1 += size;
	      else
		sc1 += size;
	    }
	  else if( label2.size() > 0 && label==label2 )
	    {
	      (*ie)->getProperty( SIA_SIZE, size );
	      if( (*ie)->getSyntax() == SIA_JUNCTION_SYNTAX )
		sj2 += size;
	      else
		sc2 += size;
	    }
	}
    }

  if( ifc )
    {
      switch( t )
	{
	case ADD:
	  ifc->cSzCort += sc1;
	  ifc->cSzJunc += sj1;
	  break;
	case REMOVE:
	  ifc->cSzCort -= sc1;
	  ifc->cSzJunc -= sj1;
	  break;
	default:	// change
	  sc1 -= sc2;
	  sj1 -= sj2;
	  ifc->cSzCort -= sc1;
	  ifc->cSzJunc -= sj1;
	}
    }

  dist = sc1; // + ManginFactor * sj1;

  return( dist );
}


double FoldLabelsChanger::output( double, double dist )
{
  //  return( 4. / (1. + exp( -( dist <7000 ? dist : 7000 )/100 )) -3. );
  // celui-ci r�pond entre 0 et 1
  return( 2. / ( 1. + exp( - dist / 100 ) ) - 1. );
  // si besoin de couper avant underflow...
  // return( 2. / ( 1. + exp( -( dist < 10000 ? dist : 10000 ) / 100 )) - 1. );
}
