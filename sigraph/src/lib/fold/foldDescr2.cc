
#include <si/fold/foldDescr2.h>
#include <si/fold/foldCache.h>
#include <si/model/adaptive.h>
#include <si/graph/vertexclique.h>
#include <si/fold/fattrib.h>
#include <si/fold/foldvertex.h>
#include <si/fold/interfoldedge.h>
#include <graph/tree/tree.h>
#include <iostream>
#include <math.h>

#define SI_USE_BUILTIN_FOLD

using namespace sigraph;
using namespace carto;
using namespace std;


/* taken from <ffmpeg/common.h>
   redefined just to avoid dirty warnings
 */
#define structoffset( T, F ) ((size_t)((char *)&((T *)8)->F)-8)

// type très très local
struct FoldDescr2PExtr
{
#ifdef SI_USE_BUILTIN_FOLD
  FoldVertex	*v;
#else
  Vertex	*v;
#endif
  bool		f;
  vector<float>	pt;
};



FoldDescr2::FoldDescr2() 
  : AdapDescr(), _nnorm( 0 ), _nx( 0 ), _ny( 0 ), _nz( 0 ), _nE1E2( 0 ), 
    _e12x( 0 ), _e12y( 0 ), _e12z( 0 ), _nDirHJ( 0 ), _dHJx( 0 ), _dHJy( 0 ), 
    _dHJz( 0 ), _normalized( Normalized )
{
}


FoldDescr2::FoldDescr2( const FoldDescr2 & f )
  : AdapDescr( f ), _nnorm( f._nnorm ), _nx( f._nx ), _ny( f._ny ), 
    _nz( f._nz ), _nE1E2( f._nE1E2 ), _e12x( f._e12x ), _e12y( f._e12y ), 
    _e12z( f._e12z ), _nDirHJ( f._nDirHJ ), _dHJx( f._dHJx ), 
    _dHJy( f._dHJy ), _dHJz( f._dHJz ), _normalized( f._normalized )
{
}


FoldDescr2::~FoldDescr2()
{
}


namespace
{
  template <typename U, typename T> inline 
  U & structfield( T* x, int off )
  {
    return *reinterpret_cast<U *>( reinterpret_cast<char *>(x) + off );
  }
}


bool FoldDescr2::makeVector( const Clique* cl, vector<double> & vec, 
                             GenericObject* model )
{
  CliqueCache		*cch;
  FoldCache		*fc = 0;

  //cout << "FoldDescr::makeVector\n";

  if( cl->getProperty( SIA_CACHE, cch ) 
      || cl->getProperty( SIA_ORIGINAL_CACHE, cch ) )
    {
      fc = dynamic_cast<FoldCache *>( cch );
      assert( fc );
      //cout << "cache. sizeof() = " << sizeof( *fc ) << ", " 
      //   << sizeof( FoldCache ) << "\n";
      if( fc->vecValid )
	{
	  //cout << "cache: vecteur valide\n";
	  vec = fc->inputVector;
	  if (!vec[0]) return false;
	  return true;
	}
    }
  // else cout << "no cache\n";

  bool x = makeVectorElements( cl, vec, model );

  if( x && fc )	// cache
    {
      //cout << "remplissage du cache\n";
      fc->inputVector = vec;
      fc->vecValid = true;
    }

  return x;
}


bool FoldDescr2::makeVectorElements( const Clique* cl, vector<double> & vec, 
                                     GenericObject* )
{
  CliqueCache		*cch;
  FoldCache		*fc = 0;

  //cout << "FoldDescr::makeVector\n";

  if( cl->getProperty( SIA_CACHE, cch ) 
      || cl->getProperty( SIA_ORIGINAL_CACHE, cch ) )
    {
      fc = dynamic_cast<FoldCache *>( cch );
      assert( fc );
      //cout << "cache. sizeof() = " << sizeof( *fc ) << ", " 
      //   << sizeof( FoldCache ) << "\n";
      if( fc->vecValid )
	{
	  //cout << "cache: vecteur valide\n";
	  vec = fc->inputVector;
	  if (!vec[0]) return false;
	  return true;
	}
    }
  // else cout << "no cache\n";

  const VertexClique	*vcl = (const VertexClique *) cl;

  //cout << "FoldDescr2::makeVector\n";

  // Calcul des "attributs synthétisés"

  string			label, labelV, synt;
  VertexClique::iterator	iv, fv=vcl->end();
#ifdef SI_USE_BUILTIN_FOLD
  FoldVertex			*v;
  HullJunctionEdge		*hjedg;
#else
  Vertex			*v;
#endif
  float				size = 0, s, x = 0, y = 0, z = 0, depth = 0, d;
  float				nrmx = 0, nrmy = 0, nrmz = 0, snrm = 0;
  float				mindepth = 0, meandepth = 0;
  set<Vertex *>			vertices;
  unsigned			n = 0, vec_valid = 0, norm_valid = 0;
  vector<float>			g;
  Vertex::const_iterator	ie, fe;
  set<Edge *>			pe;
  Edge::const_iterator		iv2;
#ifndef SI_USE_BUILTIN_FOLD
  float				surf;
#endif
  float				hjs = 0, /*d12 = 0, d1, d2,*/ dmaxCC = 0;
  vector<float>			e1, e2, extr1, extr2, dir, dirtmp;
  Edge				*edg;
  unsigned			ncort = 0, nplis = 0, e1e2_valid = 0;
  unsigned			ncc = 0, nccnc = 0;
  //	liste des points extrêmes des noeuds
  vector<FoldDescr2PExtr>	lextr;
  int				major, minor, np = 0, tnp;

  //	initialisation

  checkDataGraphVersion( cl, major, minor );

  bool	normzd = _normalized == Normalized || _normalized == NormalizedBoth;
#ifdef SI_USE_BUILTIN_FOLD
  int	surfaceoff = surfaceOffset( normzd, cl, major, minor );
  int	surfaceVoff = surfaceValidOffset( normzd, cl, major, minor );
  int	gravityoff = gravityCenterOffset( normzd, cl, major, minor );
  int	gravityVoff = gravityCenterValidOffset( normzd, cl, major, minor );
  int	normaloff = normalOffset( normzd, cl, major, minor );
  int	normalVoff = normalValidOffset( normzd, cl, major, minor );
  int	mindepthoff = minDepthOffset( normzd, cl, major, minor );
  //int	mindepthVoff = minDepthValidOffset( normzd, cl, major, minor );
  int	maxdepthoff = maxDepthOffset( normzd, cl, major, minor );
  int	maxdepthVoff = maxDepthValidOffset( normzd, cl, major, minor );
  int	meandepthoff = meanDepthOffset( normzd, cl, major, minor );
  int	meandepthVoff = meanDepthValidOffset( normzd, cl, major, minor );
  int	hjlenoff = hullJunctionLengthOffset( normzd, cl, major, minor );
  int	hjlenVoff = hullJunctionLengthValidOffset( normzd, cl, major, minor );
  int	hjextr1off = hullJunctionExtremity1Offset( normzd, cl, major, minor );
  /*int	hjextr1Voff = hullJunctionExtremity1ValidOffset( normzd, cl, major, 
    minor );*/
  int	hjextr2off = hullJunctionExtremity2Offset( normzd, cl, major, minor );
  /*int	hjextr2Voff = hullJunctionExtremity2ValidOffset( normzd, cl, major, 
    minor );*/
  int	hjdiroff = hullJunctionDirectionOffset( normzd, cl, major, minor );
  int	hjdirVoff = hullJunctionDirectionValidOffset( normzd, cl, major, 
                                                      minor );
  int	cdistoff = corticalDistanceOffset( normzd, cl, major, minor );
#else
  string surfaceatt = surfaceAttribute( normzd, cl, major, minor );
  string gravityatt = gravityCenterAttribute( normzd, cl, major, minor );
  string normalatt = normalAttribute( normzd, cl, major, minor );
  string mindepthatt = minDepthAttribute( normzd, cl, major, minor );
  string maxdepthatt = maxDepthAttribute( normzd, cl, major, minor );
  string meandepthatt = meanDepthAttribute( normzd, cl, major, minor );
  string hjlenatt = hullJunctionLengthAttribute( normzd, cl, major, minor );
  string hjextr1att = hullJunctionExtremity1Attribute( normzd, cl, major, 
                                                       minor );
  string hjextr2att = hullJunctionExtremity2Attribute( normzd, cl, major, 
                                                       minor );
  string hjdiratt = hullJunctionDirectionAttribute( normzd, cl, major, minor );
  string cdistatt = corticalDistanceAttribute( normzd, cl, major, minor );
#endif

  dir.push_back( 0 );
  dir.push_back( 0 );
  dir.push_back( 0 );

  //	noeuds concernés

  //cout << "avant passe sur les noeuds\n";
  cl->getProperty( SIA_LABEL, label );
  //cout << "clique: " << vcl->size() << " nodes\n";
  for( iv=vcl->begin(); iv!=fv; ++iv )
    {
#ifdef SI_USE_BUILTIN_FOLD
      v = static_cast<FoldVertex *>( *iv );
      labelV = v->label;
      if( surfaceVoff < 0 || structfield<bool>( v, surfaceVoff ) )
        s = structfield<float>( v, surfaceoff );
      else
        s = 0;
      if( label == labelV && s > 0 )
#else
      v = *iv;
      v->getProperty( SIA_LABEL, labelV );
      if( label == labelV && v->getProperty( surfaceatt, s ) )
#endif
	{
	  /*cout << "vertex " << v << ", synt: " << v->getSyntax() 
	    << ", label: " << labelV << endl;*/
	  vertices.insert( v );
	  vec_valid = 1;

	  //	taille
	  size += s;

	  //	centre gravité
#ifdef SI_USE_BUILTIN_FOLD
          if( structfield<bool>( v, gravityVoff ) )
	    {
              const vector<float> & g 
                = structfield<vector<float> >( v, gravityoff );
#else
	  if( v->getProperty( gravityatt, g ) )
	    {
#endif
	      x += g[0] * s;
	      y += g[1] * s;
	      z += g[2] * s;
	      ++n;
	    }

	  //	normale
#ifdef SI_USE_BUILTIN_FOLD
          if( structfield<bool>( v, normalVoff ) )
	    {
              const vector<float> & g 
                = structfield<vector<float> >( v, normaloff );
#else
	  if( v->getProperty( normalatt, g ) )
            {
#endif
              float	inv = 1.F;
	      if( _nnorm > 0 )	// faut le remettre dans le bon sens
		{
		  if( g[0] * _nx + g[1] * _ny + g[2] * _nz < 0 )
                    inv = -1.F;
		}
	      else if( snrm > 0 
		       && g[0] * nrmx + g[1] * nrmy + g[2] * nrmz < 0 )
                inv = -1.F;
	      snrm += s;
	      nrmx += g[0] * s * inv;
	      nrmy += g[1] * s * inv;
	      nrmz += g[2] * s * inv;
	      norm_valid = 1;
	    }

	  //	profondeur
#ifdef SI_USE_BUILTIN_FOLD
          d = structfield<float>( v, mindepthoff );
#else
	  v->getProperty( mindepthatt, d );
#endif
	  if( depth == 0 || mindepth > d )
	    mindepth = d;
#ifdef SI_USE_BUILTIN_FOLD
          if( structfield<bool>( v, maxdepthVoff ) 
              && ( d = structfield<float>( v, maxdepthoff ) ) > depth )
            depth = d;
#else
	  if( v->getProperty( maxdepthatt, d ) && d > depth )
	    depth = d;
#endif
#ifdef SI_USE_BUILTIN_FOLD
          if( meandepthoff >= 0
              && ( meandepthVoff < 0
                   || structfield<bool>( v, meandepthVoff ) )
              && ( d = structfield<float>( v, meandepthoff ) ) )
            {
              tnp = v->bottom_point_number;
#else
          if( !meandepthatt.empty() && v->getProperty( meandepthatt, d ) 
              && v->getProperty( SIA_BOTTOM_POINT_NUMBER, tnp ) )
            {
#endif
              meandepth += d * tnp;
              np += tnp;
            }

	  //	relations...
	  for( ie=v->begin(), fe=v->end(); ie!=fe; ++ie )
	    {
	      edg = *ie;
	      synt = edg->getSyntax();

	      //	hull_junction
	      if( synt == SIA_HULLJUNCTION_SYNTAX )
		{
		  //	taille HJ
#ifdef SI_USE_BUILTIN_FOLD
                  hjedg = static_cast<HullJunctionEdge *>( edg );
                  if( hjlenVoff < 0 || structfield<bool>( hjedg, hjlenVoff ) )
                    hjs += structfield<float>( hjedg, hjlenoff );
		  //	E1 and E2
                  const vector<float> & e1 
                    = structfield<vector<float> >( hjedg, hjextr1off );
                  const vector<float> & e2 
                    = structfield<vector<float> >( hjedg, hjextr2off );
#else
		  if( edg->getProperty( hjlenatt, surf ) )
		    hjs += surf;

		  //	E1 et E2
		  edg->getProperty( hjextr1att, e1 );
		  edg->getProperty( hjextr2att, e2 );
#endif
                  FoldDescr2PExtr tmp1 = { v, false, e1 };
                  lextr.push_back( tmp1 );
                  FoldDescr2PExtr tmp2 = { v, true, e2 };
                  lextr.push_back( tmp2 );
		  e1e2_valid = 1;

		  //	Direction
#ifdef SI_USE_BUILTIN_FOLD
                  if( structfield<bool>( hjedg, hjdirVoff ) )
                    {
                      const vector<float>	& dirtmp 
                        = structfield<vector<float> >( hjedg, hjdiroff );
#else
		  if( edg->getProperty( hjdiratt, dirtmp ) )
		    {
#endif
                      float	sgn = 1.F;
		      if( _nDirHJ > 0 )	// remettre dans le bon sens
			{
			  if( dirtmp[0] * _dHJx + dirtmp[1] * _dHJy 
			      + dirtmp[2] * _dHJz < 0 )
                            sgn = -1.F;
			}
		      else if( hjs > 0 
			       && dirtmp[0] * dir[0] + dirtmp[1] * dir[1] 
			       + dirtmp[2] * dir[2] < 0 )
                        sgn = -1.F;
		      dir[0] += s * dirtmp[0] * sgn;
		      dir[1] += s * dirtmp[1] * sgn;
		      dir[2] += s * dirtmp[2] * sgn;
		    }
		}

	      //	autres types de relations
	      else if( pe.find( edg ) == pe.end() )
		{
		  iv2 = edg->begin();
		  if( *iv2 == v )
		    ++iv2;
#ifdef SI_USE_BUILTIN_FOLD
                  labelV = static_cast<const FoldVertex *>( *iv2 )->label;
                  if( static_cast<const FoldVertex *>( *iv2 )->label_valid 
                      && labelV == label )
#else
		  if( (*iv2)->getProperty( SIA_LABEL, labelV ) 
		      && labelV == label )
#endif
		    //	relations intra-label
		    {
		      pe.insert( edg );
		      //cout << "rel to " << *iv2 << endl;

		      //	nombre de plis de passage
		      if( synt == SIA_PLI_DE_PASSAGE_SYNTAX )
			{
			  //cout << "pli de passage\n";
			  ++nplis;
			}
		    }
		}
	    }
	}
    }

  //	Extrêmités

  if( lextr.empty() )
    {
      extr1.push_back( 0 );
      extr1.push_back( 0 );
      extr1.push_back( 0 );
      extr2.push_back( 0 );
      extr2.push_back( 0 );
      extr2.push_back( 0 );
    }
  else
    {
      vector<FoldDescr2PExtr>::const_iterator	il, jl, fl=lextr.end();
      unsigned	indmax1 = 0, indmax2 = 0, i = 0, j;
      double	dmax = -1., ds, tx, ty, tz;
      bool	accept;
      int	id1, id2, id3, id4;

      //cout << "avant passe sur extrêmités\n";
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
		  // critère de choix
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
		  // nombres complètement artificiels
		  id1 = ( id1*2 + ( lextr[indmax1].f ? 1 : 0 ) ) * 100000
		    + id2*2 + ( lextr[indmax2].f ? 1 : 0 );
		  id2 = ( id3*2 + ( (*il).f ? 1 : 0 ) ) * 100000
		    + id4*2 + ( (*jl).f ? 1 : 0 );
		  assert( id1 != id2 );	// ça ne doit JAMAIS être égal
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
      //cout << "index max : " << indmax1 << ", " << indmax2 << endl;
      extr1 = lextr[indmax1].pt;
      extr2 = lextr[indmax2].pt;
    }
  //cout << "après passe sur extrêmités\n";

  //	Connexité

  bool	need_ncc = true, need_nccnc = true;

  if( fc )	// cache ?
    {
      //	conn. totale
      if( fc->subVecValid.size() > NCC && fc->subVecValid[ NCC ] )
	{
	  ncc = (unsigned) fc->inputVector[ NCC ];
	  need_ncc = false;
	}

      //	conn. sans corticales & relations entre elles
      if( fc->subVecValid.size() > DISTMAX_CC 
	  && fc->subVecValid[ NCC_NOT_CORTICAL ] && fc->subVecValid[ NCORT ] 
	  && fc->subVecValid[ DISTMAX_CC ] )
	{
	  nccnc = (unsigned) fc->inputVector[ NCC_NOT_CORTICAL ];
	  ncort = (unsigned) fc->inputVector[ NCORT ];
	  dmaxCC = fc->inputVector[ DISTMAX_CC ];
	  need_nccnc = false;
	}
    }

  if( need_ncc )	// calcul nécessaire pour la connexité
    {
      /*set<string>	syntx;
      syntx.insert( SIA_CORTICAL_SYNTAX );
      syntx.insert( SIA_JUNCTION_SYNTAX );
      syntx.innert( SIA_PLI_DE_PASSAGE_SYNTAX );*/
      ncc = vcl->connectivity( vertices /*, 0, syntx*/ );
    }
  else
    {
      /*cout << "Ncc cache : " << ncc;
      if( ncc == 0 )
	cout << ", vert : " << vertices.size();
	cout << endl;*/
    }
  if( need_nccnc )
    {
      set<string>	csynt;
      set<CComponent *>	compc;

      csynt.insert( SIA_JUNCTION_SYNTAX );
      csynt.insert( SIA_PLI_DE_PASSAGE_SYNTAX );
      //	comp. connexes sans cortical
      //cout << "vertices in clique: " << vertices.size() << endl;
      nccnc = vcl->connectivity( vertices, &compc, csynt );

      map<pair<CComponent *, CComponent *>, float >	rels;
      set<Edge *>::const_iterator		ipe, fpe = pe.end();
      set<CComponent *>::const_iterator		icc, fcc = compc.end();
      CComponent::const_iterator		iscc;
      CComponent				*cc1, *cc2, *cc1b, *cc2b;
      float					distc;

      //	re-scanner les relations intra-label
      for( ipe=pe.begin(); ipe!=fpe; ++ipe )
	if( (*ipe)->getSyntax() == SIA_CORTICAL_SYNTAX )
	  {
	    //	CC de départ et d'arrivée
	    iv2 = (*ipe)->begin();
#ifdef SI_USE_BUILTIN_FOLD
            v = static_cast<FoldVertex *>( *iv2 );
#else
	    v = *iv2;
#endif
	    for( icc=compc.begin(), cc1=0; !cc1 && icc!=fcc; ++icc )
	      {
		iscc = (*icc)->find( v );
		if( iscc != (*icc)->end() )
		  cc1 = *icc;
	      }
	    if( !cc1 )
	      {
		cerr << "node outside all connected components: " 
		     << v << ", syntax: " << v->getSyntax();
		string	lbl;
		if( v->getProperty( SIA_LABEL, lbl ) )
		  cerr << ", label: " << lbl;
		cerr << endl;
	      }
	    ++iv2;
#ifdef SI_USE_BUILTIN_FOLD
            v = static_cast<FoldVertex *>( *iv2 );
#else
	    v = *iv2;
#endif
	    for( icc=compc.begin(), cc2=0; !cc2 && icc!=fcc; ++icc )
	      {
		iscc = (*icc)->find( v );
		if( iscc != (*icc)->end() )
		  cc2 = *icc;
	      }
	    if( !cc2 )
	      {
		string	dlabel;
		float	dsz;
#ifdef SI_USE_BUILTIN_FOLD
                dlabel = v->label;
                dsz = structfield<float>( v, surfaceoff );
#else
		v->getProperty( SIA_LABEL, dlabel );
		v->getProperty( surfaceatt, dsz );
#endif
		cout << "cc2 nul, v : " << v << ", label : " << dlabel 
		     << ", sz : " << dsz << endl;
	      }
	    assert( cc1 && cc2 );
	    if( cc1 != cc2 )	// différentes
	      {
		if( cc1 < cc2 )	// insérer avec le + petit ptr en 1er
		  {
		    cc1b = cc1;
		    cc2b = cc2;
		  }
		else	// inverser (pour éviter les doublons)
		  {
		    cc1b = cc2;
		    cc2b = cc1;
		  }

#ifdef SI_USE_BUILTIN_FOLD
                distc = structfield<float>( static_cast<CorticalEdge *>(*ipe), 
                                            cdistoff );
#else
		(*ipe)->getProperty( cdistatt, distc );
#endif
		assert( distc > 0 );
		float &mi 
		  = rels[ pair<CComponent *, CComponent *>( cc1b, cc2b ) ];
		if( mi == 0 || distc < mi )
		  mi = distc;
	      }
	  }
      // nb de rel. corticales entre comp. connexes
      ncort = rels.size();
      map<pair<CComponent *, CComponent *>, float >::const_iterator 
	imi, fmi = rels.end();

      for( imi=rels.begin(); imi!=fmi; ++imi )
	if( (*imi).second > dmaxCC )
	  dmaxCC = (*imi).second;
      //	ménage
      for( icc=compc.begin(); icc!=fcc; ++icc )
	delete *icc;
    }

  //	corrections, normalisations

  if( size > 0 )
    {
      x /= size;
      y /= size;
      z /= size;
    }
  if( snrm > 0 )
    {
      float lnrm = sqrt( nrmx * nrmx + nrmy * nrmy + nrmz * nrmz );
      if( lnrm == 0 )
	{
	  nrmx = 0;
	  nrmy = 0;
	  nrmz = 0;
	  norm_valid = 0;
	}
      else
	{
	  nrmx /= lnrm;
	  nrmy /= lnrm;
	  nrmz /= lnrm;
	}
    }

  double nrm = sqrt( dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2] );
  if( nrm )
    {
      dir[0] /= nrm;
      dir[1] /= nrm;
      dir[2] /= nrm;
    }

  /*if( ncc > 1 )		// plusieurs composantes connexes
    dmaxCC = 30;	// on considère qu'elles sont trop éloignées
  else if( nccnc <= 1 )	// 1 seule CCnc (ou pas de sillon)
    dmaxCC = 0;		// hyper compact */
  if( ncort == 0 )	// pas de rels entre CC
  {
    if( nccnc <= 1 )	// 1 seule CC (ou pas de sillon)
      dmaxCC = 0;	// hyper compact
    else		// plusieurs composantes non reliées
      dmaxCC = 30;	// (trop loin pour être relié ?)
  }

  //	remplissage du vecteur

  vec.push_back( vec_valid );

  vec.push_back( e1e2_valid );
  /*if( e1e2_valid )
    {
      cout << "E1 : " << extr1[0] << ", " << extr1[1] << ", " << extr1[2] 
	   << endl;
      cout << "E2 : " << extr2[0] << ", " << extr2[1] << ", " << extr2[2] 
	   << endl;
	   }*/
  vec.push_back( extr1[0] );
  vec.push_back( extr1[1] );
  vec.push_back( extr1[2] );
  vec.push_back( extr2[0] );
  vec.push_back( extr2[1] );
  vec.push_back( extr2[2] );

  vec.push_back( x );
  vec.push_back( y );
  vec.push_back( z );

  vec.push_back( norm_valid );
  vec.push_back( nrmx );
  vec.push_back( nrmy );
  vec.push_back( nrmz );

  vec.push_back( dir[0] );
  vec.push_back( dir[1] );
  vec.push_back( dir[2] );

  vec.push_back( size );
  vec.push_back( depth );
  vec.push_back( mindepth );
#ifdef SI_USE_BUILTIN_FOLD
  if( meandepthoff > 0 )
#else
  if( !meandepthatt.empty() )
#endif
    vec.push_back( meandepth / ( np == 0 ? 1 : np ) );

  vec.push_back( ncc );
  vec.push_back( nccnc );
  vec.push_back( ncort );
  vec.push_back( dmaxCC );
  vec.push_back( nplis );
  vec.push_back( hjs );

  return vec_valid;
}


bool FoldDescr2::makeLearnVector( const Clique* cl, vector<double> & vec, 
				  GenericObject* model, double )
{
  return( makeVector( cl, vec, model ) );	// provisoire
}


void FoldDescr2::handleStats( const Clique*, vector<double> & vec, 
			      GenericObject*, double outp )
{
  if( vec[NX] == 0 && vec[NY] == 0 && vec[NZ] == 0 )
    return;	// pas de normale dans cet exemple

  if( outp != -1 )
    return;

  // seulement si c'est un bon exemple

  //	stats sur l'orientation
  if( _nnorm  == 0 )
    {
      _nx = vec[NX];
      _ny = vec[NY];
      _nz = vec[NZ];
      ++_nnorm;
    }
  else
    {
      float	cosA = _nx * vec[NX] + _ny * vec[NY] + _nz * vec[NZ];
      // cos de l'angle entre les 2 vecteurs

      if( cosA < 0 )	// mauvais côté: inverser le vecteur
	{
	  vec[NX] = -vec[NX];
	  vec[NY] = -vec[NY];
	  vec[NZ] = -vec[NZ];
	}
      _nx = ( _nx * _nnorm + vec[NX] ) / ( _nnorm + 1 );
      _ny = ( _ny * _nnorm + vec[NY] ) / ( _nnorm + 1 );
      _nz = ( _nz * _nnorm + vec[NZ] ) / ( _nnorm + 1 );

      double len = sqrt( _nx*_nx + _ny*_ny + _nz*_nz );
      if( len == 0 )	// cas pathologique
	_nnorm = 0;	// plus de stats valables, on recommence
      else
	{
	  _nx /= len;
	  _ny /= len;
	  _nz /= len;
	  ++_nnorm;
	}
    }

  //	stats sur le vecteur E1E2
  if( _nE1E2  == 0 )
    {
      _e12x = vec[E2X] - vec[E1X];
      _e12y = vec[E2Y] - vec[E1Y];
      _e12z = vec[E2Z] = vec[E1Z];
      ++_nE1E2;
    }
  else
    {
      vector<double>	e12;
      e12.push_back( vec[E2X] - vec[E1X] );
      e12.push_back( vec[E2Y] - vec[E1Y] );
      e12.push_back( vec[E2Z] - vec[E1Z] );

      float	cosA = _e12x * e12[0] + _e12y * e12[1] + _e12z * e12[2];
      // cos de l'angle entre les 2 vecteurs

      if( cosA < 0 )	// mauvais côté: inverser le vecteur
	{
	  double	tmp = vec[E1X];
	  vec[E1X] = vec[E2X];
	  vec[E2X] = tmp;
	  tmp = vec[E1Y];
	  vec[E1Y] = vec[E2Y];
	  vec[E2Y] = tmp;
	  tmp = vec[E1Z];
	  vec[E1Z] = vec[E2Z];
	  vec[E2Z] = tmp;
	  e12[0] *= -1;
	  e12[1] *= -1;
	  e12[2] *= -1;
	}
      _e12x = ( _e12x * _nE1E2 + e12[0] ) / ( _nE1E2 + 1 );
      _e12y = ( _e12y * _nE1E2 + e12[1] ) / ( _nE1E2 + 1 );
      _e12z = ( _e12z * _nE1E2 + e12[2] ) / ( _nE1E2 + 1 );
      ++_nE1E2;
    }

  //	stats sur la direction de la liaison hull_junction
  if( _nDirHJ  == 0 )
    {
      _dHJx = vec[DIRX];
      _dHJy = vec[DIRY];
      _dHJz = vec[DIRZ];
      ++_nDirHJ;
    }
  else
    {
      float	cosA = _dHJx * vec[DIRX] + _dHJy * vec[DIRY] 
	+ _dHJz * vec[DIRZ];
      // cos de l'angle entre les 2 vecteurs
      float	len;

      if( cosA < 0 )	// mauvais côté: inverser le vecteur
	{
	  vec[DIRX] = -vec[DIRX];
	  vec[DIRY] = -vec[DIRY];
	  vec[DIRZ] = -vec[DIRZ];
	}
      _dHJx = ( _dHJx * _nDirHJ + vec[DIRX] ) / ( _nDirHJ + 1 );
      _dHJy = ( _dHJy * _nDirHJ + vec[DIRY] ) / ( _nDirHJ + 1 );
      _dHJz = ( _dHJz * _nDirHJ + vec[DIRZ] ) / ( _nDirHJ + 1 );
      len = sqrt( _dHJx*_dHJx + _dHJy*_dHJy + _dHJz*_dHJz );
      if( len == 0 )	// cas pathologique
	_nDirHJ = 0;	// plus de stats valables, on recommence
      else
	{
	  _dHJx /= len;
	  _dHJy /= len;
	  _dHJz /= len;
	  ++_nDirHJ;
	}
    }
}


void FoldDescr2::preProcess( vector<double> & vec, GenericObject* )
{
  //	Utiliser les stats d'orientation

  vector<float>		nrm;
  float	cosA;

  if( _nnorm )
    {
      // cos de l'angle entre les 2 vecteurs
      cosA =  _nx * vec[NX] + _ny * vec[NY] + _nz * vec[NZ];

      if( cosA < 0 )
	{
	  //	Adapter le vecteur
	  vec[NX] *= -1;
	  vec[NY] *= -1;
	  vec[NZ] *= -1;
	}
    }

  if( _nE1E2 )
    {
      vector<double>	e12;
      e12.push_back( vec[E2X] - vec[E1X] );
      e12.push_back( vec[E2Y] - vec[E1Y] );
      e12.push_back( vec[E2Z] - vec[E1Z] );

      cosA = _e12x * e12[0] + _e12y * e12[1] + _e12z * e12[2];
      if( cosA < 0 )
	{
	  double	tmp = vec[E1X];
	  vec[E1X] = vec[E2X];
	  vec[E2X] = tmp;
	  tmp = vec[E1Y];
	  vec[E1Y] = vec[E2Y];
	  vec[E2Y] = tmp;
	  tmp = vec[E1Z];
	  vec[E1Z] = vec[E2Z];
	  vec[E2Z] = tmp;
	}
    }

  if( _nDirHJ )
    {
      cosA = _dHJx * vec[DIRX] + _dHJy * vec[DIRY] + _dHJz * vec[DIRZ];
      if( cosA < 0 )
	{
	  vec[DIRX] = -vec[DIRX];
	  vec[DIRY] = -vec[DIRY];
	  vec[DIRZ] = -vec[DIRZ];
	}
    }
}


void FoldDescr2::reset()
{
  _nnorm = 0;
  _nx = _ny = _nz = 0;
  _nE1E2 = 0;
  _e12x = _e12y = _e12z = 0;
  _nDirHJ = 0;
  _dHJx = _dHJy = _dHJz = 0;
}


void FoldDescr2::buildTree( Tree & t )
{
  t.setSyntax( SIA_FOLD_DESCRIPTOR2 );

  vector<float>	v;

  if( _nnorm )
    {
      t.setProperty( SIA_NSTATS_NORMAL, (int) _nnorm );
      v.push_back( _nx );
      v.push_back( _ny );
      v.push_back( _nz );
      t.setProperty( SIA_NORMAL, v );
    }
  if( _nE1E2 )
    {
      t.setProperty( SIA_NSTATS_E1E2, (int) _nE1E2 );
      v.erase( v.begin(), v.end() );
      v.push_back( _e12x );
      v.push_back( _e12y );
      v.push_back( _e12z );
      t.setProperty( SIA_E1E2, v );
    }
  if( _nDirHJ )
    {
      t.setProperty( SIA_NSTATS_DIR, (int) _nDirHJ );
      v.erase( v.begin(), v.end() );
      v.push_back( _dHJx );
      v.push_back( _dHJy );
      v.push_back( _dHJz );
      t.setProperty( SIA_DIRECTION, v );
    }
}


bool FoldDescr2::hasChanged( const Clique* cl, 
			     const map<Vertex*, string> & changes, 
			     const GenericObject* model ) const
{
  const VertexClique	*vcl = (const VertexClique *) cl;
  Model			*mod;
  TopModel		*tm = 0;

  if( !model || !model->getProperty( SIA_MODEL, mod ) 
      || !(tm=mod->topModel()) )
    {
      cout << "FoldDescr2::hasChanged : topmodel pas trouvé\n";
      return( true );	// manque des trucs: recalcule tout
    }

  VertexClique::const_iterator		iv, fv=vcl->end();
  map<Vertex *, string>::const_iterator	im, fm=changes.end();
  string				label2;

  set<string>	& sl = tm->significantLabels();
  string	vl = tm->voidLabel();

  if( sl.size() == 0 || vl.size() == 0 )
    {
      cout << "FoldDescr2::hasChanged : pas de labels significatifs\n";
      return( true );	// labels significatifs pourris
    }

  set<string>::const_iterator		fs = sl.end();

  for( iv=vcl->begin(); iv!=fv; ++iv )
    if( (im=changes.find( *iv )) != fm )
      {
	const string	&label1 = (*im).second;
	if( label1 != vl && sl.find( label1 ) != fs )	// ancien label
	  {
	    /*cout << "FoldDescr2::hasChanged : ancien label " << label1 
	      << " significatif\n";*/
	    return( true );				// non-void
	  }
#ifdef SI_USE_BUILTIN_FOLD
        label2 = static_cast<const FoldVertex *>( *iv )->label;
#else
	(*iv)->getProperty( SIA_LABEL, label2 );
#endif
	if( label2 != vl && sl.find( label2 ) != fs )	// nouveau label
	  {
	    /*cout << "FoldDescr2::hasChanged : nouveau label " << label2 
	      << " significatif\n";*/
	    return true;				// non-void
	  }
      }

  //cout << "FoldDescr2::hasChanged : pas de chgt pour cl " << cl << "\n";
  return false;	// si rien n'a changé, on ne recalcule pas
}


vector<string> FoldDescr2::descriptorsNames() const
{
  static vector<string>	names;
  if( names.empty() )
    {
      names.reserve( 27 );

      names.push_back( "valid" );
      names.push_back( "extremitiesValid" );
      names.push_back( "extremity1x" );
      names.push_back( "extremity1y" );
      names.push_back( "extremity1z" );
      names.push_back( "extremity2x" );
      names.push_back( "extremity2y" );
      names.push_back( "extremity2z" );
      names.push_back( "gravityCenter_x" );
      names.push_back( "gravityCenter_y" );

      names.push_back( "gravityCenter_z" );
      names.push_back( "normalValid" );
      names.push_back( "normal_x" );
      names.push_back( "normal_y" );
      names.push_back( "normal_z" );
      names.push_back( "direction_x" );
      names.push_back( "direction_y" );
      names.push_back( "direction_z" );
      names.push_back( "volume" );
      names.push_back( "geodesicDepthMax" );

      names.push_back( "geodesicDepthMin" );
      names.push_back( "connectedComponentsAllRels" );
      names.push_back( "connectedComponents" );
      names.push_back( "pureCortical" );
      names.push_back( "distanceBetweenComponentsMax" );
      names.push_back( "plisDePassage" );
      names.push_back( "hullJunctionsSize" );
    }
  return names;
}


string FoldDescr2::name() const
{
  return SIA_FOLD_DESCRIPTOR2;
}


void FoldDescr2::checkDataGraphVersion( const Clique* cl, 
                                        int & major, int & minor )
{
  Graph		*g;
  string	sver;

  if( cl->getProperty( SIA_GRAPH, g ) 
      && g->getProperty( SIA_DATAGRAPH_VERSION, sver ) )
    {
      istringstream	iss( sver );
      iss >> major;
      iss.get();
      iss >> minor;
    }
  major = 0;
  minor = 0;
}


bool FoldDescr2::dataVersionGE( int major, int minor, 
                                int datamajor, int dataminor )
{
  return datamajor > major || ( datamajor == major && dataminor >= minor );
}


string FoldDescr2::surfaceAttribute( bool, const Clique*, 
                                     int, int ) const
{
  return SIA_SIZE;
}


string FoldDescr2::gravityCenterAttribute( bool, const Clique*, 
                                           int, int ) const
{
  return SIA_REFGRAVITY_CENTER;
}


string FoldDescr2::normalAttribute( bool, const Clique*, int, int ) const
{
  return SIA_REFNORMAL;
}


string FoldDescr2::minDepthAttribute( bool, const Clique*, int, int ) const
{
  return SIA_MINDEPTH;
}


string FoldDescr2::maxDepthAttribute( bool, const Clique*, int, int ) const
{
  return SIA_MAXDEPTH;
}


string FoldDescr2::meanDepthAttribute( bool, const Clique*, int, int ) const
{
  return "";
}


string FoldDescr2::hullJunctionLengthAttribute( bool, const Clique*, 
                                                int, int ) const
{
  return SIA_SIZE;
}


string FoldDescr2::hullJunctionExtremity1Attribute( bool, const Clique*, 
                                                    int, int ) const
{
  return SIA_REFEXTREMITY1;
}


string FoldDescr2::hullJunctionExtremity2Attribute( bool, const Clique*,
                                                    int, int ) const
{
  return SIA_REFEXTREMITY2;
}


string FoldDescr2::hullJunctionDirectionAttribute( bool, const Clique*, 
                                                   int, int ) const
{
  return SIA_REFDIRECTION;
}


string FoldDescr2::corticalDistanceAttribute( bool, const Clique*, 
                                              int, int ) const
{
  return SIA_DIST;
}


int FoldDescr2::surfaceOffset( bool, const Clique*, int, int ) const
{
  return structoffset( FoldVertex, size );
}


int FoldDescr2::surfaceValidOffset( bool, const Clique*, int, int ) const
{
  return -1;
}


int FoldDescr2::gravityCenterOffset( bool, const Clique*, int, int ) const
{
  return structoffset( FoldVertex, refgravity_center );
}


int FoldDescr2::gravityCenterValidOffset( bool, const Clique*, int, int ) const
{
  return structoffset( FoldVertex, refgravity_center_valid );
}


int FoldDescr2::normalOffset( bool, const Clique*, int, int ) const
{
  return structoffset( FoldVertex, refnormal );
}


int FoldDescr2::normalValidOffset( bool, const Clique*, int, int ) const
{
  return structoffset( FoldVertex, refnormal_valid );
}


int FoldDescr2::minDepthOffset( bool, const Clique*, int, int ) const
{
  return structoffset( FoldVertex, mindepth );
}


int FoldDescr2::minDepthValidOffset( bool, const Clique*, int, int ) const
{
  return structoffset( FoldVertex, mindepth_valid );
}


int FoldDescr2::maxDepthOffset( bool, const Clique*, int, int ) const
{
  return structoffset( FoldVertex, maxdepth );
}


int FoldDescr2::maxDepthValidOffset( bool, const Clique*, int, int ) const
{
  return structoffset( FoldVertex, maxdepth_valid );
}


int FoldDescr2::meanDepthOffset( bool, const Clique*, int, int ) const
{
  return -1;
}


int FoldDescr2::meanDepthValidOffset( bool, const Clique*, int, int ) const
{
  return -1;
}


int FoldDescr2::hullJunctionLengthOffset( bool, const Clique*, int, int ) const
{
  return structoffset( HullJunctionEdge, size );
}


int FoldDescr2::hullJunctionLengthValidOffset( bool, const Clique*, int, 
                                               int ) const
{
  return -1;
}


int FoldDescr2::hullJunctionExtremity1Offset( bool, const Clique*, int, 
                                              int ) const
{
  return structoffset( HullJunctionEdge, refextremity1 );
}


int FoldDescr2::hullJunctionExtremity1ValidOffset( bool, const Clique*, int, 
                                                   int ) const
{
  return structoffset( HullJunctionEdge, refextremity1_valid );
}


int FoldDescr2::hullJunctionExtremity2Offset( bool, const Clique*, int, 
                                              int ) const
{
  return structoffset( HullJunctionEdge, refextremity2 );
}


int FoldDescr2::hullJunctionExtremity2ValidOffset( bool, const Clique*, int, 
                                                   int ) const
{
  return structoffset( HullJunctionEdge, refextremity2_valid );
}


int FoldDescr2::hullJunctionDirectionOffset( bool, const Clique*, int, 
                                             int ) const
{
  return structoffset( HullJunctionEdge, refdirection );
}


int FoldDescr2::hullJunctionDirectionValidOffset( bool, const Clique*, int, 
                                                  int ) const
{
  return structoffset( HullJunctionEdge, refdirection_valid );
}


int FoldDescr2::corticalDistanceOffset( bool, const Clique*, int, int ) const
{
  return structoffset( CorticalEdge, dist );
}


int FoldDescr2::corticalDistanceValidOffset( bool, const Clique*, int, 
                                             int ) const
{
  return structoffset( CorticalEdge, dist_valid );
}


