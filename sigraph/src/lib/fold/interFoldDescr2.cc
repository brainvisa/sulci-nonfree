#include <cstdlib>
#include <si/fold/interFoldDescr2.h>
#include <si/fold/foldDescr2.h>
#include <si/graph/vertexclique.h>
#include <si/model/model.h>
#include <si/fold/interFoldCache.h>
#include <si/fold/fattrib.h>
#include <graph/tree/tree.h>
#include <graph/graph/graph.h>
#include <iostream>
#include <math.h>
#include <si/fold/foldvertex.h>
#include <si/fold/interfoldedge.h>
#ifdef SIGRAPH_DEBUG
#include <typeinfo>
#endif

#define SI_USE_BUILTIN_FOLD

using namespace sigraph;
using namespace carto;
using namespace std;


/* taken from <ffmpeg/common.h>
   redefined just to avoid dirty warnings
 */
#define structoffset( T, F ) ((size_t)((char *)&((T *)8)->F)-8)


namespace
{

  // type tr� tr� local
  struct InterFoldDescr2PExtr
  {
#ifdef SI_USE_BUILTIN_FOLD
    FoldVertex	*v;
#else
    Vertex* v;
#endif
    bool f;
    vector<float> pt;
  };


  template <typename U, typename T> inline 
  U & structfield( T* x, int off )
  {
    return *reinterpret_cast<U *>( reinterpret_cast<char *>(x) + off );
  }

}


InterFoldDescr2::~InterFoldDescr2()
{
}


bool InterFoldDescr2::makeVector( const Clique* cl, vector<double> & vec, 
				  GenericObject* model )
{
  CliqueCache			*cch;
  InterFoldCache		*ifc = 0;

  if( cl->getProperty( SIA_CACHE, cch ) 
      || cl->getProperty( SIA_ORIGINAL_CACHE, cch ) )
    {
      ifc = dynamic_cast<InterFoldCache *>( cch );
      assert( ifc );
      if( ifc->vecValid )
	{
	  vec = ifc->inputVector;
	  if (!vec[0]) return false;
	  return true;
	}
    }
  bool x = makeVectorElements( cl, vec, model );
  if( x && ifc )
    {
      // update cache
      ifc->inputVector = vec;
      ifc->vecValid = true;
    }
  return x;
}


bool InterFoldDescr2::makeVectorElements( const Clique* cl, 
                                          vector<double> & vec, 
                                          GenericObject* )
{
  const VertexClique	*vcl = (const VertexClique *) cl;

#ifdef SIGRAPH_DEBUG
  cout << "InterFoldDescr2::makeVector\n";
#endif
  string			label1, label2;
  set<Edge *>			ed;
  set<Edge *>::const_iterator	ie, fe;

  assert( cl->getProperty( SIA_LABEL1, label1 ) );
  assert( cl->getProperty( SIA_LABEL2, label2 ) );

  set<Vertex *>	sl1 = vcl->getVerticesWith( SIA_LABEL, label1 );
  set<Vertex *> sl2 = vcl->getVerticesWith( SIA_LABEL, label2 );
  vector<float>			E11, E12, E21, E22;
  float				s, sum1, sum2;
  Vertex			*v1, *v2;
  bool				hashj1, hashj2;
#ifdef SI_USE_BUILTIN_FOLD
  Edge				*edg;
  CorticalEdge			*cort = 0, *cortedg;
  JunctionEdge			*juncedg;
  HullJunctionEdge		*hjedg;
  PliDePassageEdge		*ppedg;
#else
  Edge				*edg, *cort = 0;
#endif
  double			d1, d2, dmax1, dmax2;
  int				major, minor;

  if( sl1.size() == 0 || sl2.size() == 0 )
    {	// un des sillons n'est pas l� pas de relation
      while( vec.size() < END )
	vec.push_back( 0 );
      //cout << "rel absente " << label1 << " <-> " << label2 << endl;
      return( false );
    }

  //	initialisation

  FoldDescr2::checkDataGraphVersion( cl, major, minor );

  bool	normzd = _normalized == Normalized || _normalized == NormalizedBoth;
#ifdef SI_USE_BUILTIN_FOLD
  int	clenoff = corticalLengthOffset( normzd, cl, major, minor );
  int	clenVoff = corticalLengthValidOffset( normzd, cl, major, minor );
  int	cdistoff = corticalDistanceOffset( normzd, cl, major, minor );
  //int	cdistVoff = corticalDistanceValidOffset( normzd, cl, major, minor );
  int	css1nearoff = corticalSS1NearestOffset( normzd, cl, major, minor );
  /*int	css1nearVoff = corticalSS1NearestValidOffset( normzd, cl, major, 
    minor );*/
  int	css2nearoff = corticalSS2NearestOffset( normzd, cl, major, minor );
  /*int	css2nearVoff = corticalSS2NearestValidOffset( normzd, cl, major, 
    minor );*/
  int	jlenoff = junctionLengthOffset( normzd, cl, major, minor );
  int	jlenVoff = junctionLengthValidOffset( normzd, cl, major, minor );
  int	jdepthoff = junctionDepthOffset( normzd, cl, major, minor );
  //int	jdepthVoff = junctionDepthValidOffset( normzd, cl, major, minor );
  int	hjdiroff = hullJunctionDirectionOffset( normzd, cl, major, minor );
  int	hjdirVoff = hullJunctionDirectionValidOffset( normzd, cl, major, 
                                                      minor );
  int	hjextr1off = hullJunctionExtremity1Offset( normzd, cl, major, minor );
  /*int	hjextr1Voff = hullJunctionExtremity1ValidOffset( normzd, cl, major, 
    minor );*/
  int	hjextr2off = hullJunctionExtremity2Offset( normzd, cl, major, minor );
  /*int	hjextr2Voff = hullJunctionExtremity2ValidOffset( normzd, cl, major, 
    minor );*/
  int	ppdepthoff = pliDePassageDepthOffset( normzd, cl, major, minor );
  //int	ppdepthVoff = pliDePassageDepthValidOffset( normzd, cl, major, minor );
#else
  string clenatt = corticalLengthAttribute( normzd, cl, major, minor );
  string cdistatt = corticalDistanceAttribute( normzd, cl, major, minor );
  string css1nearatt = corticalSS1NearestAttribute( normzd, cl, major, minor );
  string css2nearatt = corticalSS2NearestAttribute( normzd, cl, major, minor );
  string hjdiratt = hullJunctionDirectionAttribute( normzd, cl, major, minor );
  string hjextr1att = hullJunctionExtremity1Attribute( normzd, cl, major, 
                                                       minor );
  string hjextr2att = hullJunctionExtremity2Attribute( normzd, cl, major, 
                                                       minor );
  string jlenatt = junctionLengthAttribute( normzd, cl, major, minor );
  string jdepthatt = junctionDepthAttribute( normzd, cl, major, minor );
  string ppdepthatt = pliDePassageDepthAttribute( normzd, cl, major, minor );
#endif

  //	R�um�sillon par sillon
  scanFold( sl1, sum1, E11, E12, dmax1, hashj1, cl, major, minor );
  scanFold( sl2, sum2, E21, E22, dmax2, hashj2, cl, major, minor );

  //	relations entre les 2 labels 
  string	synt;
  double	sc = 0, sj = 0, nj = 0, npp = 0, jdpt = 0, ppdpt, jdmax = 0;
  double	dmin = 0, pscalc, dminS1 = 0, dminS2 = 0, ds1, ds2, dm, dmo;
  float		d;
  bool		fstjdpth = true, fstppdpth = true, fstcort = true;
  bool		haspscal = true, takecort;
  vector<float>	nea1, nea2, dir, dir1, dir2, extr1, extr2, e1, e2, n1, n2, tv;
  Vertex::const_iterator	ie2, fe2;
  vector<float>	grel;

  dir.push_back( 0 );
  dir.push_back( 0 );
  dir.push_back( 0 );
  vcl->edgesBetween( sl1, sl2, ed );

  //	Connexit�
  set<CComponent *>	compc1, compc2, usedcc1, usedcc2;
  set<CComponent *>::iterator	ic, fc;
  unsigned		cc1, cc2;
  set<string>		syntl;
  Edge::const_iterator	iv;
  bool			invnodes;

  syntl.insert( SIA_JUNCTION_SYNTAX );
  syntl.insert( SIA_PLI_DE_PASSAGE_SYNTAX );
  cc1 = vcl->connectivity( sl1, &compc1, syntl );
  cc2 = vcl->connectivity( sl2, &compc2, syntl );

  //	Relations

  for( ie=ed.begin(), fe=ed.end(); ie!=fe; ++ie )
    {
      edg = *ie;
      synt = edg->getSyntax();
      //	noeuds correspondants
      iv = edg->begin();
      v1 = *iv;
      ++iv;
      v2 = *iv;
      if( sl1.find( v1 ) == sl1.end() )
	{	// mettre l'ordre: v1 pour label1 et v2 pour label2
	  v2 = v1;
	  v1 = *iv;
	  invnodes = true;
	}
      else
	invnodes = false;

      //	�quelle composante connexe appartient un noeud ?
      for( ic=compc1.begin(), fc=compc1.end(); ic!=fc; ++ic )
	if( (*ic)->find( v1 ) != (*ic)->end() )
	  {
	    usedcc1.insert( *ic );
	    break;
	  }
      if( ic == fc )
	cerr << "Noeud dans aucune composante S1 ! (BUG)\n";
      for( ic=compc2.begin(), fc=compc2.end(); ic!=fc; ++ic )
	if( (*ic)->find( v2 ) != (*ic)->end() )
	  {
	    usedcc2.insert( *ic );
	    break;
	  }
      if( ic == fc )
	cerr << "Noeud dans aucune composante S2 ! (BUG)\n";

      if( synt == SIA_CORTICAL_SYNTAX )
	{
	  //	taille rel. corticales
#ifdef SI_USE_BUILTIN_FOLD
          cortedg = static_cast<CorticalEdge *>( edg );
          if( clenVoff < 0 || structfield<bool>( cortedg, clenVoff ) )
            sc += structfield<float>( cortedg, clenoff );
	  //	dist min
          d = structfield<float>( cortedg, cdistoff );
#else
	  if( edg->getProperty( clenatt, s ) )
	    sc += s;
	  //	dist min
	  edg->getProperty( cdistatt, d );
#endif
	  if( fstcort || d < dmin )
	    {
	      dmin = d;
#ifdef SI_USE_BUILTIN_FOLD
	      cort = cortedg;	// garder trace de la relation
	      //	nearest points
              nea1 = structfield<vector<float> >( cort, css1nearoff );
              nea2 = structfield<vector<float> >( cort, css2nearoff );
#else
	      cort = edg;	// garder trace de la relation
	      //	points les plus proches
	      cort->getProperty( css1nearatt, nea1 );
	      cort->getProperty( css2nearatt, nea2 );
#endif
	      if( invnodes )
		{
		  tv = nea1;
		  nea1 = nea2;
		  nea2 = tv;
		}

	      // distance ES1-Pcontact
	      d1 = ( E11[0] - nea1[0] ) * ( E11[0] - nea1[0] ) 
		+ ( E11[1] - nea1[1] ) * ( E11[1] - nea1[1] ) 
		+ ( E11[2] - nea1[2] ) * ( E11[2] - nea1[2] );
	      d2 = ( E12[0] - nea1[0] ) * ( E12[0] - nea1[0] ) 
		+ ( E12[1] - nea1[1] ) * ( E12[1] - nea1[1] ) 
		+ ( E12[2] - nea1[2] ) * ( E12[2] - nea1[2] );
	      if( d1 < d2 )
		dminS1 = d1;
	      else
		dminS1 = d2;
	      // distance ES2-Pcontact
	      d1 = ( E21[0] - nea2[0] ) * ( E21[0] - nea2[0] ) 
		+ ( E21[1] - nea2[1] ) * ( E21[1] - nea2[1] ) 
		+ ( E21[2] - nea2[2] ) * ( E21[2] - nea2[2] );
	      d2 = ( E22[0] - nea2[0] ) * ( E22[0] - nea2[0] ) 
		+ ( E22[1] - nea2[1] ) * ( E22[1] - nea2[1] ) 
		+ ( E22[2] - nea2[2] ) * ( E22[2] - nea2[2] );
	      if( d1 < d2 )
		dminS2 = d1;
	      else
		dminS2 = d2;
	    }
	  else if( d == dmin )	// m�e distance min (0 par ex.)
	    {	// dans ce cas, crit�e de min de dminS1 / dminS2
	      //	points les plus proches
#ifdef SI_USE_BUILTIN_FOLD
              n1 = structfield<vector<float> >( cortedg, css1nearoff );
              n2 = structfield<vector<float> >( cortedg, css2nearoff );
#else
	      edg->getProperty( css1nearatt, n1 );
	      edg->getProperty( css2nearatt, n2 );
#endif
	      if( invnodes )
		{
		  tv = n1;
		  n1 = n2;
		  n2 = tv;
		}

	      // distance ES1-Pcontact
	      d1 = ( E11[0] - n1[0] ) * ( E11[0] - n1[0] ) 
		+ ( E11[1] - n1[1] ) * ( E11[1] - n1[1] ) 
		+ ( E11[2] - n1[2] ) * ( E11[2] - n1[2] );
	      d2 = ( E12[0] - n1[0] ) * ( E12[0] - n1[0] ) 
		+ ( E12[1] - n1[1] ) * ( E12[1] - n1[1] ) 
		+ ( E12[2] - n1[2] ) * ( E12[2] - n1[2] );
	      if( d1 < d2 )
		ds1 = d1;
	      else
		ds1 = d2;
	      // distance ES2-Pcontact
	      d1 = ( E21[0] - n2[0] ) * ( E21[0] - n2[0] ) 
		+ ( E21[1] - n2[1] ) * ( E21[1] - n2[1] ) 
		+ ( E21[2] - n2[2] ) * ( E21[2] - n2[2] );
	      d2 = ( E22[0] - n2[0] ) * ( E22[0] - n2[0] ) 
		+ ( E22[1] - n2[1] ) * ( E22[1] - n2[1] ) 
		+ ( E22[2] - n2[2] ) * ( E22[2] - n2[2] );
	      if( d1 < d2 )
		ds2 = d1;
	      else
		ds2 = d2;

	      if( ds1 < ds2 )		// min( ds1, ds2 )
		dm = ds1;
	      else
		dm = ds2;
	      if( dminS1 < dminS2 )	// min( dminS1, dminS2 )
		dmo = dminS1;
	      else
		dmo = dminS2;

	      if( dm < dmo )
		takecort = true;
	      else if( dm > dmo )
		takecort = false;
	      else	// m�e minimum: chercher plus loin une diff�ence
		{
		  if( ds1 == ds2 
		      || ( ds1 > dm && (ds1 < dminS1 || ds1 < dminS2) )
		      || ( ds2 > dm && (ds2 < dminS1 || ds2 < dminS2) ) )
		    takecort = true;
		  else
		    takecort = false;
		}

	      if( takecort )
		{
		  nea1 = n1;
		  nea2 = n2;
		  dminS1 = ds1;
		  dminS2 = ds2;
#ifdef SI_USE_BUILTIN_FOLD
		  cort = cortedg;
#else
		  cort = edg;
#endif
		}
	    }
	  fstcort = false;
	}
      else if( synt == SIA_JUNCTION_SYNTAX )
	{
	  //	taille jonc
#ifdef SI_USE_BUILTIN_FOLD
          juncedg = static_cast<JunctionEdge *>( edg );
          if( jlenVoff < 0 
             || structfield<bool>( juncedg, jlenVoff ) )
            sj += structfield<float>( juncedg, jlenoff );
#else
	  if( edg->getProperty( jlenatt, s ) )
	    sj += s;
#endif
	  //	nb jonc
	  ++nj;
	  //	depth min
#ifdef SI_USE_BUILTIN_FOLD
          s = structfield<float>( juncedg, jdepthoff );
#else
	  edg->getProperty( jdepthatt, s );
#endif
	  if( fstjdpth )
	    {
	      fstjdpth = false;
	      jdpt = s;
	      jdmax = s;
	    }
	  else
	    {
	      if( s < jdpt )
		jdpt = s;
	      else if( s > jdmax )
		jdmax = s;
	    }
	}
      else if( synt == SIA_PLI_DE_PASSAGE_SYNTAX )
	{
	  //	nb PP
	  ++npp;
	  //	depth
#ifdef SI_USE_BUILTIN_FOLD
          ppedg = static_cast<PliDePassageEdge *>( edg );
          s = structfield<float>( ppedg, ppdepthoff );
#else
	  edg->getProperty( ppdepthatt, s );
#endif
	  if( fstppdpth )
	    {
	      fstppdpth = false;
	      ppdpt = s;
	    }
	  else if( s > ppdpt )
	    ppdpt = s;
	}
    }

  //	post-traitements, normalisations etc

  //	rel corticale de distance min
  if( fstcort )
    {	// pas de rel. corticale...
      sc = 0;
      dmin = 100;		// ?...
      // dir est d���0
      dminS1 = dminS2 = 0;	// ?...
      pscalc = 2;
      grel.push_back( 0 );
      grel.push_back( 0 );
      grel.push_back( 0 );
    }
  else
    {
      //	direction
      dir[0] = nea2[0] - nea1[0];
      dir[1] = nea2[1] - nea1[1];
      dir[2] = nea2[2] - nea1[2];
      d = sqrt( dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2] );
      dir[0] /= d;
      dir[1] /= d;
      dir[2] /= d;
      grel.push_back( (nea1[0] + nea2[0] ) / 2 );
      grel.push_back( (nea1[1] + nea2[1] ) / 2 );
      grel.push_back( (nea1[2] + nea2[2] ) / 2 );

      //	noeuds correspondants
      iv = cort->begin();
      v1 = *iv;
      ++iv;
      v2 = *iv;
      if( sl1.find( v1 ) == sl1.end() )
	{	// mettre l'ordre: v1 pour label1 et v2 pour label2
	  v2 = v1;
	  v1 = *iv;
	  invnodes = true;
	}
      else
	invnodes = false;

      //	directions des hull_jonctions des morceaux correspdt
      // morceau 1
      for( ie2=v1->begin(), fe2=v1->end(); 
	   ie2!=fe2 && (*ie2)->getSyntax() != SIA_HULLJUNCTION_SYNTAX; 
	   ++ie2 ) {}
      assert( ie2 != fe2 );	// il y en a toujours pcq il n'y a pas de
      // rel. corticales entre les sillons pas externes
      edg = *ie2;
#ifdef SI_USE_BUILTIN_FOLD
      hjedg = static_cast<HullJunctionEdge *>( edg );
      if( !structfield<bool>( hjedg, hjdirVoff ) )
#else
      if( !edg->getProperty( hjdiratt, dir1 ) )
#endif
	{	// pas de refdirection, essayer les extr�it�
#ifdef SI_USE_BUILTIN_FOLD
          const vector<float> & extr1 
            = structfield<vector<float> >( hjedg, hjextr1off );
          extr2 = structfield<vector<float> >( hjedg, hjextr2off );
#else
	  edg->getProperty( hjextr1att, extr1 );
	  edg->getProperty( hjextr2att, extr2 );
#endif
	  extr2[0] -= extr1[0];
	  extr2[1] -= extr1[1];
	  extr2[2] -= extr1[2];
	  d1 = extr2[0] * extr2[0] + extr2[1] * extr2[1] 
	    + extr2[2] * extr2[2];
	  if( d1 == 0 )	// extr. confondues: pas de direction calculable
	    haspscal = false;
	  else
	    {
	      dir1.push_back( extr2[0] / d1 );
	      dir1.push_back( extr2[1] / d1 );
	      dir1.push_back( extr2[2] / d1 );
	    }
	}
#ifdef SI_USE_BUILTIN_FOLD
      else
        dir1 = structfield<vector<float> >( hjedg, hjdiroff );
#endif
      if( haspscal )
	{
	  // morceau 2
	  for( ie2=v2->begin(), fe2=v2->end(); 
	       ie2!=fe2 && (*ie2)->getSyntax() != SIA_HULLJUNCTION_SYNTAX; 
	       ++ie2 ) {}
	  assert( ie2 != fe2 );
	  edg = *ie2;
#ifdef SI_USE_BUILTIN_FOLD
          hjedg = static_cast<HullJunctionEdge *>( edg );
          if( !structfield<bool>( hjedg, hjdirVoff ) )
#else
	  if( !edg->getProperty( hjdiratt, dir2 ) )
#endif
	    {
#ifdef SI_USE_BUILTIN_FOLD
              const vector<float> & extr1 
                = structfield<vector<float> >( hjedg, hjextr1off );
              extr2 = structfield<vector<float> >( hjedg, hjextr2off );
#else
	      edg->getProperty( hjextr1att, extr1 );
	      edg->getProperty( hjextr2att, extr2 );
#endif
	      extr2[0] -= extr1[0];
	      extr2[1] -= extr1[1];
	      extr2[2] -= extr1[2];
	      d1 = extr2[0] * extr2[0] + extr2[1] * extr2[1] 
		+ extr2[2] * extr2[2];
	      if( d1 == 0 )
		haspscal = false;
	      else
		{
		  dir2.push_back( extr2[0] / d1 );
		  dir2.push_back( extr2[1] / d1 );
		  dir2.push_back( extr2[2] / d1 );
		}
	    }
#ifdef SI_USE_BUILTIN_FOLD
          dir2 = structfield<vector<float> >( hjedg, hjdiroff );
#endif
	}
      // produit scalaire
      if( haspscal )
	pscalc = fabs( dir1[0] * dir2[0] + dir1[1] * dir2[1] 
		       + dir1[2] * dir2[2] );
      else
	pscalc = 2.;	// valeur out of range
    }

  if( fstppdpth )	// pas de PP
    {
      if( !fstjdpth )	// existe des jonctions
	ppdpt = jdmax;	// alors depth pp = max depth jonctions (trop profond)
      else		// pas de jonction: corticale pure? (pas assez profond)
	ppdpt = 0;
    }

  vec.push_back( 1 );				// rel. valide

  vec.push_back( sum1 );			// taille 1er sillon
  vec.push_back( cc1 );				// nb CC S1 sans cort.
  vec.push_back( usedcc1.size() );		// nb CC S1 impliqu�s
  vec.push_back( sum2 );			// taille 2e sillon
  vec.push_back( cc2 );				// nb CC S2 sans cort.
  vec.push_back( usedcc2.size() );		// nb CC S2 impliqu�s

  vec.push_back( sc );		// taille des relations corticales
  vec.push_back( sqrt( dmin ) );
  vec.push_back( dir[0] );
  vec.push_back( dir[1] );
  vec.push_back( dir[2] );
  vec.push_back( sqrt( dminS1 ) );
  vec.push_back( sqrt( dminS2 ) );
  vec.push_back( grel[0] );
  vec.push_back( grel[1] );
  vec.push_back( grel[2] );
  //cout << "dminS1 : " << dminS1 << ", dminS2 : " << dminS2 << endl;
  vec.push_back( pscalc );	// abs( dir1 . dir2 ) : 

  vec.push_back( nj );		// nb jonctions
  vec.push_back( jdpt );	// min depth jonctions
  vec.push_back( sj );		// taille des jonctions

  vec.push_back( npp );		// nb plis de passage
  vec.push_back( ppdpt );	// prof. max pp

  //	Effacer les comp. connexes

  CComponent	*cc;

  while( !compc1.empty() )
    {
      cc = *compc1.begin();
      delete cc;
      compc1.erase( compc1.begin() );
    }
  while( !compc2.empty() )
    {
      cc = *compc2.begin();
      delete cc;
      compc2.erase( compc2.begin() );
    }

  /*cout << "edge : pscal : " << pscalc << ", npp : " << npp << ", dirx : " 
    << dir[0] << endl; */

  return true;
}


bool InterFoldDescr2::makeLearnVector( const Clique* cl, vector<double> & vec, 
				      GenericObject* model )
{
  return( makeVector( cl, vec, model ) );	// provisoire
}


void InterFoldDescr2::buildTree( Tree & t )
{
  t.setSyntax( SIA_INTER_FOLD_DESCRIPTOR2 );
}


bool InterFoldDescr2::hasChanged( const Clique* cl, 
				  const map<Vertex*, string> & changes, 
				  const GenericObject* model ) const
{
  const VertexClique	*vcl = (const VertexClique *) cl;
  Model			*mod;
  TopModel		*tm = 0;

  if( !model || !model->getProperty( "model", mod ) || !(tm=mod->topModel()) )
    return( true );	// manque des trucs: recalcule tout

  VertexClique::const_iterator		iv, fv=vcl->end();
  map<Vertex *, string>::const_iterator	im, fm=changes.end();
  string				label2;

  set<string>	& sl = tm->significantLabels();
  string	vl = tm->voidLabel();

  if( sl.size() == 0 || vl.size() == 0 )
    return( true );	// labels significatifs pourris

  set<string>::const_iterator		fs = sl.end();

  for( iv=vcl->begin(); iv!=fv; ++iv )
    if( (im=changes.find( *iv )) != fm )
      {
	const string	&label1 = (*im).second;
	if( label1 != vl && sl.find( label1 ) != fs )	// ancien label
	  return( true );				// non-void
#ifdef SI_USE_BUILTIN_FOLD
        label2 = static_cast<const FoldVertex *>( *iv )->label;
#else
	(*iv)->getProperty( SIA_LABEL, label2 );
#endif
	if( label2 != vl && sl.find( label2 ) != fs )	// nouveau label
	  return true;					// non-void
      }

  return false;	// si rien n'a chang� on ne recalcule pas
}


void InterFoldDescr2::scanFold( const set<Vertex *> & sv, float & size, 
				vector<float> & extr1, vector<float> & extr2, 
				double & dmax, bool & hashj, const Clique *cl, 
                                int major, int minor )
{
  set<Vertex *>::const_iterator	iv, fv=sv.end();
#ifdef SI_USE_BUILTIN_FOLD
  FoldVertex			*v;
  HullJunctionEdge		*edg;
#else
  Vertex			*v;
  Edge				*edg;
  float				s;
#endif
  Vertex::const_iterator	ie, fe;
  vector<float>			e1, e2;
  //double			d1, d2;
  //	liste des points extr�es des noeuds
  vector<InterFoldDescr2PExtr>	lextr;

  bool	normzd = _normalized == Normalized || _normalized == NormalizedBoth;
#ifdef SI_USE_BUILTIN_FOLD
  int	surfoff = foldSurfaceOffset( normzd, cl, major, minor );
  int	hjextr1off = hullJunctionExtremity1Offset( normzd, cl, major, minor );
  int	hjextr2off = hullJunctionExtremity2Offset( normzd, cl, major, minor );
#else
  string surfatt = foldSurfaceAttribute( normzd, cl, major, minor );
  string hjextr1att = hullJunctionExtremity1Attribute( normzd, cl, major, 
                                                       minor );
  string hjextr2att = hullJunctionExtremity2Attribute( normzd, cl, major, 
                                                       minor );
#endif

  dmax = 0;
  size = 0;
  hashj = false;

  for( iv=sv.begin(); iv!=fv; ++iv )
    {
#ifdef SI_USE_BUILTIN_FOLD
      v = static_cast<FoldVertex *>( *iv );
      size += structfield<float>( v, surfoff );
#else
      v = *iv;
      v->getProperty( surfatt, s );
      size += s;
#endif

      //	recherche hull_junction
      for( ie=v->begin(), fe=v->end(); 
	   ie!=fe && (*ie)->getSyntax() != SIA_HULLJUNCTION_SYNTAX; ++ie ) {}
      if( ie != fe )
	{
	  hashj = true;

	  //	E1 et E2
#ifdef SI_USE_BUILTIN_FOLD
          edg = static_cast<HullJunctionEdge *>( *ie );
          const vector<float>	& e1 
            = structfield<vector<float> >( edg, hjextr1off );
          const vector<float>	& e2 
            = structfield<vector<float> >( edg, hjextr2off );
#else
	  edg = *ie;
	  edg->getProperty( hjextr1att, e1 );
	  edg->getProperty( hjextr2att, e2 );
#endif
          InterFoldDescr2PExtr tmp1 = { v, false, e1 };
	  lextr.push_back( tmp1 );
          InterFoldDescr2PExtr tmp2 = { v, true, e2 };
	  lextr.push_back( tmp2 );
	}
    }

  //	Extr�it�

  if( lextr.empty() )
    {
      extr1.erase( extr1.begin(), extr1.end() );
      extr2.erase( extr2.begin(), extr2.end() );
      extr1.push_back( 0 );
      extr1.push_back( 0 );
      extr1.push_back( 0 );
      extr2.push_back( 0 );
      extr2.push_back( 0 );
      extr2.push_back( 0 );
      dmax = 0;
    }
  else
    {
      vector<InterFoldDescr2PExtr>::const_iterator	il, jl, fl=lextr.end();
      unsigned	indmax1 = 0, indmax2 = 0, i = 0, j;
      double	ds, tx, ty, tz;
      bool	accept;
      int	id1, id2, id3, id4;

      dmax = -1;

      for( il=lextr.begin(); il!=fl; ++il, ++i )
	{
	  for( jl=il, ++jl, j=i+1; jl!=fl; ++jl, ++j )
	    {
	      //cout << "i : " << i << ", j : " << j << endl;
	      tx = (*il).pt[0] - (*jl).pt[0];
	      ty = (*il).pt[1] - (*jl).pt[1];
	      tz = (*il).pt[2] - (*jl).pt[2];
	      ds = tx * tx + ty * ty + tz * tz;
	      accept = false;
	      if( ds > dmax )
		{
		  accept = true;
		}
	      else if( ds == dmax )
		{
		  // crit�e de choix
		  // ID des 4 noeuds
#ifdef SI_USE_BUILTIN_FOLD
                  id1 = lextr[indmax1].v->index;
                  id2 = lextr[indmax2].v->index;
                  id3 = (*il).v->index;
                  id4 = (*jl).v->index;
#else
		  lextr[indmax1].v->getProperty( SIA_INDEX, id1 );
		  lextr[indmax2].v->getProperty( SIA_INDEX, id2 );
		  (*il).v->getProperty( SIA_INDEX, id3 );
		  (*jl).v->getProperty( SIA_INDEX, id4 );
#endif
		  // nombres compl�ement artificiels
		  id1 = ( id1*2 + ( lextr[indmax1].f ? 1 : 0 ) ) * 100000
		    + id2*2 + ( lextr[indmax2].f ? 1 : 0 );
		  id2 = ( id3*2 + ( (*il).f ? 1 : 0 ) ) * 100000
		    + id4*2 + ( (*jl).f ? 1 : 0 );
		  assert( id1 != id2 );	// � ne doit JAMAIS �re �al
		  if( id2 < id1 )
		    accept = true;
		}
	      if( accept )
		{
		  indmax1 = i;
		  indmax2 = j;
		  dmax = ds;
		}
	    }
	}
      extr1 = lextr[indmax1].pt;
      extr2 = lextr[indmax2].pt;
    }
}


vector<string> InterFoldDescr2::descriptorsNames() const
{
  static vector<string>	names;
  if( names.empty() )
    {
      names.reserve( 23 );

      names.push_back( "valid" );
      names.push_back( "volumeS1" );
      names.push_back( "connectedComponentsS1" );
      names.push_back( "connectedComponentsS1Rel" );
      names.push_back( "volumeS2" );
      names.push_back( "connectedComponentsS2" );
      names.push_back( "connectedComponentsS2Rel" );
      names.push_back( "corticalSize" );
      names.push_back( "distanceMin" );
      names.push_back( "direction_x" );

      names.push_back( "direction_y" );
      names.push_back( "direction_z" );
      names.push_back( "distanceToExtremity1" );
      names.push_back( "distanceToExtremity2" );
      names.push_back( "gravityCenter_x" );
      names.push_back( "gravityCenter_y" );
      names.push_back( "gravityCenter_z" );
      names.push_back( "corticalDotProduct" );
      names.push_back( "junctions" );
      names.push_back( "junctionDepthMin" );

      names.push_back( "junctionsSize" );
      names.push_back( "plisDePassage" );
      names.push_back( "plisDePassageDepthMax" );
    }

  return names;
}


string InterFoldDescr2::name() const
{
  return SIA_INTER_FOLD_DESCRIPTOR2;
}


string InterFoldDescr2::foldSurfaceAttribute( bool, const Clique*, 
                                              int, int ) const
{
  return SIA_SIZE;
}


string InterFoldDescr2::corticalLengthAttribute( bool, const Clique*, 
                                                 int, int ) const
{
  return SIA_SIZE;
}


string InterFoldDescr2::corticalDistanceAttribute( bool, const Clique*, int, 
                                                   int ) const
{
  return SIA_DIST;
}


string InterFoldDescr2::corticalSS1NearestAttribute( bool, 
                                                     const Clique*, int, 
                                                     int ) const
{
  return SIA_REFSS1NEAREST;
}


string InterFoldDescr2::corticalSS2NearestAttribute( bool, 
                                                     const Clique*, int, 
                                                     int ) const
{
  return SIA_REFSS2NEAREST;
}


string InterFoldDescr2::hullJunctionDirectionAttribute( bool, 
                                                        const Clique*, int, 
                                                        int ) const
{
  return SIA_REFDIRECTION;
}


string InterFoldDescr2::hullJunctionExtremity1Attribute( bool, 
                                                         const Clique*, int, 
                                                         int ) const
{
  return SIA_REFEXTREMITY1;
}


string InterFoldDescr2::hullJunctionExtremity2Attribute( bool, 
                                                         const Clique*, int, 
                                                         int ) const
{
  return SIA_REFEXTREMITY2;
}


string InterFoldDescr2::junctionLengthAttribute( bool, const Clique*, 
                                                 int, int ) const
{
  return SIA_SIZE;
}


string InterFoldDescr2::junctionDepthAttribute( bool, const Clique*, 
                                                int, int ) const
{
  return SIA_MAXDEPTH;
}


string InterFoldDescr2::pliDePassageDepthAttribute( bool, const Clique*, int, 
                                                    int ) const
{
  return SIA_DEPTH;
}


int InterFoldDescr2::foldSurfaceOffset( bool, const Clique*, 
                                            int, int ) const
{
  return structoffset( FoldVertex, size );
}


int InterFoldDescr2::foldSurfaceValidOffset( bool, const Clique*, 
                                             int, int ) const
{
  return 0;
}


int InterFoldDescr2::corticalLengthOffset( bool , const Clique*, 
                                           int, int ) const
{
  return structoffset( CorticalEdge, size );
}


int InterFoldDescr2::corticalLengthValidOffset( bool, const Clique*, 
                                                int, int ) const
{
  return -1;
}


int InterFoldDescr2::corticalDistanceOffset( bool, const Clique*, 
                                             int, int ) const
{
  return structoffset( CorticalEdge, dist );
}


int InterFoldDescr2::corticalDistanceValidOffset( bool, const Clique*, 
                                                  int, int ) const
{
  return structoffset( CorticalEdge, dist_valid );
}


int InterFoldDescr2::corticalSS1NearestOffset( bool, const Clique*, 
                                               int, int ) const
{
  return structoffset( CorticalEdge, refSS1nearest );
}


int InterFoldDescr2::corticalSS1NearestValidOffset( bool, 
                                                    const Clique*, 
                                                    int, int ) const
{
  return structoffset( CorticalEdge, refSS1nearest_valid );
}


int InterFoldDescr2::corticalSS2NearestOffset( bool, const Clique*, 
                                               int, int ) const
{
  return structoffset( CorticalEdge, refSS2nearest );
}


int InterFoldDescr2::corticalSS2NearestValidOffset( bool, 
                                                    const Clique*, 
                                                    int, int ) const
{
  return structoffset( CorticalEdge, refSS2nearest_valid );
}


int InterFoldDescr2::junctionLengthOffset( bool, const Clique*, 
                                           int, int ) const
{
  return structoffset( JunctionEdge, size );
}


int InterFoldDescr2::junctionLengthValidOffset( bool, const Clique*, 
                                                int, int ) const
{
  return -1;
}


int InterFoldDescr2::junctionDepthOffset( bool, const Clique*, 
                                          int, int ) const
{
  return structoffset( JunctionEdge, maxdepth );
}


int InterFoldDescr2::junctionDepthValidOffset( bool, const Clique*, 
                                               int, int ) const
{
  return structoffset( JunctionEdge, maxdepth_valid );
}


int InterFoldDescr2::hullJunctionDirectionOffset( bool, 
                                                  const Clique*, int, 
                                                  int ) const
{
  return structoffset( HullJunctionEdge, refdirection );
}


int InterFoldDescr2::hullJunctionDirectionValidOffset( bool, 
                                                       const Clique*, int, 
                                                       int ) const
{
  return structoffset( HullJunctionEdge, refdirection_valid );
}


int InterFoldDescr2::hullJunctionExtremity1Offset( bool, 
                                                   const Clique*, int, 
                                                   int ) const
{
  return structoffset( HullJunctionEdge, refextremity1 );
}


int InterFoldDescr2::hullJunctionExtremity1ValidOffset( bool, 
                                                        const Clique*, int, 
                                                        int ) const
{
  return structoffset( HullJunctionEdge, refextremity1_valid );
}


int InterFoldDescr2::hullJunctionExtremity2Offset( bool, 
                                                   const Clique*, int, 
                                                   int ) const
{
  return structoffset( HullJunctionEdge, refextremity2 );
}


int InterFoldDescr2::hullJunctionExtremity2ValidOffset( bool, 
                                                        const Clique*, int, 
                                                        int ) const
{
  return structoffset( HullJunctionEdge, refextremity2_valid );
}


int InterFoldDescr2::pliDePassageDepthOffset( bool, const Clique*, 
                                              int, int ) const
{
  return structoffset( PliDePassageEdge, depth );
}


int InterFoldDescr2::pliDePassageDepthValidOffset( bool, const Clique*, 
                                                   int, int ) const
{
  return structoffset( PliDePassageEdge, depth_valid );
}


