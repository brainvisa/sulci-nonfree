/*
 *  Copyright (C) 1998-2003 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 */

#include <si/fold/foldFakeRel.h>
#include <si/graph/mgraph.h>
#include <si/graph/cgraph.h>
#include <si/fold/fattrib.h>
#include <graph/tree/tree.h>
#include <iostream>

using namespace sigraph;
using namespace carto;
using namespace std;


FoldFakeRel::FoldFakeRel( Model* parent ) 
  : Model( parent ),  _mgraph( 0 )
{
}


FoldFakeRel::~FoldFakeRel()
{
  clear();
}


FoldFakeRel::Relmap FoldFakeRel::allocGraph( const CGraph* )
{
  unsigned			i = 0, n = _mgraph->order() - 1;
  Relmap			rmap = new Reldescr*[ n ];

  for( i=0; i<n; ++i )
    rmap[i] = new Reldescr[ n-i-1 ];

  //	liste des labels
  MGraph::const_iterator		iv, fv=_mgraph->end();
  string				label;
  map<string, int>::const_iterator	ii, fi=_ltoi.end();

  //	valeurs hors-table à ne pas comptabiliser
  _ltoi[ SIV_VOID_LABEL ] = -1;
  _ltoi[ SIV_BRAIN_HULL ] = -1;

  for( iv=_mgraph->begin(); iv!=fv; ++iv )
    if( (*iv)->getProperty( SIA_LABEL, label ) )
      {
	ii = _ltoi.find( label );
	if( ii == fi )
	  {
	    i = _ltoi.size() - 2;
	    _ltoi[ label ] = i;
	  }
      }

  return( rmap );
}


void FoldFakeRel::deleteGraph( Relmap rmap )
{
  if( rmap )
    {
      for( unsigned i=0; i<_mgraph->order()-1; ++i )
	delete[] rmap[i];
      delete[] rmap;
    }
}


FoldFakeRel::Relmap FoldFakeRel::init( const CGraph & cg )
{
  Relmap			& rel = _rels[ &cg ];

  deleteGraph( rel );
  rel = allocGraph( &cg );
  update( cg, rel );
  return( rel );
}


double FoldFakeRel::update( const CGraph & cg, Relmap & rel )
{
  Edge::const_iterator		iv2;
  string			label1, label2;
  unsigned			j1, j2, tmp, n=_mgraph->order() - 1, count = 0;

  //	on commence par tout vider
  for( j1=0; j1<n; ++j1 )
    for( j2=0; j2<n-j1-1; ++j2 )
	rel[j1][j2] = Reldescr();

  //	remplir d'abord ceux qui ont un vrai expert
  //map<string, int>::const_iterator	ii, fi=_ltoi.end();
  const set<Edge *>			& se = _mgraph->edges();
  set<Edge *>::const_iterator		ime, fme = se.end();

  for( ime=se.begin(); ime!=fme; ++ime )
    if( (*ime)->getSyntax() != SIA_FAKEREL_SYNTAX )	// sauter les fakeRels
      {
	iv2 = (*ime)->begin();
	(*iv2)->getProperty( SIA_LABEL, label1 );
	++iv2;
	(*iv2)->getProperty( SIA_LABEL, label2 );

	if( label1 != SIV_VOID_LABEL && label2 != SIV_VOID_LABEL )
	  {
	    /*ii = _ltoi.find( label1 );
	    assert( ii != fi );
	    j1 = (*ii).second;*/
	    j1 = _ltoi[ label1 ];
	    // j1 est l'indice du label1
	    /*ii = _ltoi.find( label2 );
	    assert( ii != fi );
	    j2 = (*ii).second;*/
	    j2 = _ltoi[ label2 ];
	    // j2 est l'indice du label1

	    if( j1 < j2 )
	      rel[ j1 ][ j2-j1-1 ].hasModel = true;
	    else
	      rel[ j2 ][ j1-j2-1 ].hasModel = true;
	  }
      }

  //	ensuite les relations trouvées dans le graphe exemple
  const set<Edge *>		& ed = cg.edges();
  set<Edge *>::const_iterator	ie, fe=ed.end();
  Vertex			*v1, *v2;

  for( ie=ed.begin(); ie!=fe; ++ie )
    {
      iv2 = (*ie)->begin();
      v1 = *iv2;
      ++iv2;
      v2 = *iv2;
      if( v1->getProperty( SIA_LABEL, label1 ) 
	  && label1 != SIV_VOID_LABEL && label1 != SIV_BRAIN_HULL
	  && v2->getProperty( SIA_LABEL, label2 ) && label1 != label2 
	  && label2 != SIV_VOID_LABEL && label2 != SIV_BRAIN_HULL )
	{
	  /*ii = _ltoi.find( label1 );
	  assert( ii != fi );
	  j1 = (*ii).second;*/
	  j1 = _ltoi[ label1 ];
	  /*ii = _ltoi.find( label2 );
	  assert( ii != fi );
	  j2 = (*ii).second;*/
	  j2 = _ltoi[ label2 ];

	  // on est sûr que j1 != j2 pcq label1 != label2
	  if( j2 < j1 )
	    {
	      tmp = j2;
	      j2 = j1;
	      j1 = tmp;
	    }

	  Reldescr & rd = rel[j1][j2-j1-1];

	  if( !rd.hasModel )
	    {
	      if( rd.num == 0 )
		++count;	// compte seulement ceux qui passent de 0 à 1
	      ++rd.num;
	      /*cout << "rel entre " << label1 << " et " << label2 
		<< endl;*/
	    }
	}
    }

  return( (double) count );
}


void FoldFakeRel::clear()
{
  FakeRels::iterator	ir, fr=_rels.end();

  for( ir=_rels.begin(); ir!=fr; ++ir )
    deleteGraph( (*ir).second );
  _rels.erase( _rels.begin(), _rels.end() );
}


FoldFakeRel::Reldescr* FoldFakeRel::relDescr( const CGraph* cg, 
					      const string & label1, 
					      const string & label2 )
{
  int	j1, j2;

  j1 = _ltoi[ label1 ];
  if( j1 < 0 )
    return( 0 );
  j2 = _ltoi[ label2 ];
  if( j2 < 0 )
    return( 0 );

  if( j2 == j1 )
    return( 0 );

  FakeRels::iterator	ifr = _rels.find( cg );
  if( ifr == _rels.end() )
    {
      init( *cg );
      ifr = _rels.find( cg );
      assert( ifr != _rels.end() );
    }

  if( j2 < j1 )
    return( &(*ifr).second[ j2 ][ j1-j2-1 ] );
  else
    return( &(*ifr).second[ j1 ][ j2-j1-1 ] );
}


double FoldFakeRel::prop( const Clique* cl )
{
  return( update( cl ) );
}


double FoldFakeRel::prop( const Clique* cl, 
			  const map<Vertex*, string> & changes )
{
  Graph		*g;
  CGraph	*cg;

  if( !cl->getProperty( SIA_GRAPH, g ) )
    {
      cerr << "warning : clique FAKE sans attribut graph (BUG)\n";
      return( 0 );
    }
  assert( ( cg = dynamic_cast<CGraph *>( g ) ) );

  Relmap				& rmap = _rels[ cg ];
  if( !rmap )
    rmap = init( *cg );

  //	copier la table
  Relmap	copy = allocGraph( cg );
  unsigned	i, j, n = _mgraph->order() - 1;

  for( i=0; i<n; ++i )
    for( j=0; j<n-i-1; ++j )
      copy[i][j] = rmap[i][j];

  double	res = update( cl, changes );
  double	oldpot = 0;

  assert( cl->getProperty( SIA_POTENTIAL, oldpot ) );
  // debug: le potentiel de cette clique est toujours positif
  /*assert( oldpot >= 0 );
  assert( oldpot + res >= 0 );
  // debug: comparer avec le vrai calcul
  double toto = update( *cg, rmap );
  if( oldpot + res != toto )
    {
      cout << "incohérence dans FakeRel: oldpot : " << oldpot << ", chgt : " 
	   << res << ", newpot : " << toto << endl;
      cout << "changements : " << changes.size() << endl;
      map<Vertex*, string>::const_iterator	iv, fv=changes.end();
      string					nlab;
      for( iv=changes.begin(); iv!=fv; ++iv )
	{
	  (*iv).first->getProperty( SIA_LABEL, nlab );
	  cout << (*iv).second << " -> " << nlab << endl;
	  (*iv).first->setProperty( SIA_LABEL, (*iv).second );
	  (*iv).second = nlab;
	}
      double totobis = update( *cg, rmap );
      cout << "recalcul d'avant : " << totobis << endl;
      // comparaison avec état avant
      for( i=0; i<n; ++i )
	for( j=0; j<n-i-1; ++j )
	  {
	    Reldescr	& rm = rmap[i][j];
	    Reldescr	& cp = copy[i][j];
	    assert( rm.hasModel == cp.hasModel 
		    && rm.num == cp.num );
	  }
      for( iv=changes.begin(); iv!=fv; ++iv )
	{
	  (*iv).first->getProperty( SIA_LABEL, nlab );
	  (*iv).first->setProperty( SIA_LABEL, (*iv).second );
	  (*iv).second = nlab;
	}
      //assert( oldpot + res == toto );
      }*/

  //	remettre la table originale
  for( i=0; i<n; ++i )
    for( j=0; j<n-i-1; ++j )
      rmap[i][j] = copy[i][j];
  deleteGraph( copy );

  // debug
  /*if( oldpot + res != toto )
    {
      double tutu = update( cl, changes );
      assert( oldpot + res == toto );
      }*/

  return( oldpot + res );
}


void FoldFakeRel::buildTree( Tree & tr ) const
{
  tr.setSyntax( SIA_FAKEREL_SYNTAX );
}


double FoldFakeRel::update( const Clique* cl, 
			    const map<Vertex*, string> & changes )
{
  Graph		*g;
  CGraph	*cg;

  if( !cl->getProperty( SIA_GRAPH, g ) )
    {
      cerr << "warning : clique FAKE sans attribut graph (BUG)\n";
      return( 0 );
    }
  assert( ( cg = dynamic_cast<CGraph *>( g ) ) );

  Relmap				& rmap = _rels[ cg ];
  if( !rmap )
    rmap = init( *cg );

  map<Vertex*, string>::const_iterator	iv, fv=changes.end(), ivo;
  Vertex				*v1, *v2;
  int					j1o, j1n, j2o, j1, j2;
  string				label1, label2;
  int					nc = 0;
  set<Edge *>				done;
  set<Edge *>::const_iterator		df = done.end();
  Vertex::const_iterator		ie, fe;
  Edge					*edg;
  Edge::const_iterator			iv2;

  //	debug
  //unsigned	nl = _ltoi.size();

  for( iv=changes.begin(); iv!=fv; ++iv )
    {
      v1 = (*iv).first;
      const string	& lold = (*iv).second;
      assert( v1->getProperty( SIA_LABEL, label1 ) );
      j1o = _ltoi[ lold ];	// ancien indice
      j1n = _ltoi[ label1 ];	// nouveau

      for( ie=v1->begin(), fe=v1->end(); ie!=fe; ++ie )
	if( done.find( edg = *ie ) == df )	// seulement si pas déjà fait
	  {
	    done.insert( edg );

	    //	autre bout de la relation
	    iv2 = edg->begin();
	    if( *iv2 == v1 )
	      ++iv2;
	    v2 = *iv2;
	    assert( v2->getProperty( SIA_LABEL, label2 ) );

	    //	décrémenter l'ancien compteur
	    ivo = changes.find( v2 );
	    if( ivo != fv )			// autre bout changé aussi
	      j2o = _ltoi[ (*ivo).second ];	// prendre l'ancien label
	    else
	      j2o = _ltoi[ label2 ];
	    if( j1o >= 0 && j2o >= 0 && j1o != j2o )
	      {
		if( j2o < j1o )
		  {
		    j1 = j2o;
		    j2 = j1o;
		  }
		else
		  {
		    j1 = j1o;
		    j2 = j2o;
		  }
		Reldescr	& rd = rmap[ j1 ][ j2-j1-1 ];
		if( rd.num > 0 )
		  {
		    --rd.num;
		    if( rd.num == 0 )
		      --nc;
		  }
	      }

	    //	incrémenter le nouveau compteur
	    if( j1n >= 0 )
	      {
		if( ivo != fv )
		  j2o = _ltoi[ label2 ];
		if( j2o >= 0 && j1n != j2o )
		  {
		    if( j2o < j1n )
		      {
			j1 = j2o;
			j2 = j1n;
		      }
		    else
		      {
			j1 = j1n;
			j2 = j2o;
		      }
		    Reldescr	& rd2 = rmap[ j1 ][ j2-j1-1 ];
		    if( !rd2.hasModel )
		      {
			if( rd2.num == 0 )
			  ++nc;
			++rd2.num;
		      }
		  }
	      }
	  }
    }

  //	debug
  /*if( _ltoi.size() != nl )
    {
      cerr << "FFake rel: nombre de labels changé. " << nl << " -> " 
	   << _ltoi.size() << endl;
      assert( false );
      }*/

  return( (double) nc );
}


double FoldFakeRel::update( const Clique* cl )
{
  Graph		*g;
  CGraph	*cg;

  if( !cl->getProperty( SIA_GRAPH, g ) )
    {
      cerr << "warning : clique FAKE sans attribut graph (BUG)\n";
      return( 0 );
    }
  assert( ( cg = dynamic_cast<CGraph *>( g ) ) );

  Relmap				& rmap = _rels[ cg ];
  if( !rmap )
    rmap = init( *cg );
  return( update( *cg, rmap ) );
}


#include <cartobase/object/object_d.h>
INSTANTIATE_GENERIC_OBJECT_TYPE( FoldFakeRel * )

