
#include <si/fold/foldDescr.h>
#include <si/fold/foldCache.h>
#include <si/model/adaptive.h>
#include <si/graph/vertexclique.h>
#include <si/fold/fattrib.h>
#include <graph/tree/tree.h>
#include <math.h>

using namespace sigraph;
using namespace carto;
using namespace std;


FoldDescr::FoldDescr() : AdapDescr(), _nnorm( 0 ), _nx( 0 ), _ny( 0 ), 
  _nz( 0 ), _limitSize( 0 )
{
}


FoldDescr::FoldDescr( const FoldDescr & f )
  : AdapDescr( f ), _nnorm( 0 ), _nx( f._nx ), _ny( f._ny ), _nz( f._nz ), 
    _limitSize( f._limitSize )
{
}


FoldDescr::~FoldDescr()
{
}


bool FoldDescr::makeVector( const Clique* cl, vector<double> & vec, 
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


bool FoldDescr::makeVectorElements( const Clique* cl, vector<double> & vec, 
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
	  return true;
	}
    }
  // else cout << "no cache\n";

  const VertexClique	*vcl = (const VertexClique *) cl;
  float			size = 0, s, depth = 0, d, x = 0, y = 0, z = 0;
  float			nrmx = 0, nrmy = 0, nrmz = 0, snrm = 0;
  unsigned		n = 0, ns = 0, nb = 0, norm_valid = 0, vec_valid = 0;
  vector<float>		g;
  VertexClique::iterator	iv, fv=vcl->end();
  Vertex		*v;
  string		label, labelV;
  Vertex::const_iterator	ie, fe;
  set<Edge *>			pe;
  Edge::const_iterator		iv2;
  float				csurf = 0, surf, hjs = 0;
  set<Vertex *>		vertices;
  unsigned		ccj, ccc;	// comp. connexes

  cl->getProperty( SIA_LABEL, label );

  for( iv=vcl->begin(); iv!=fv; ++iv )
    {
      v = *iv;
      v->getProperty( SIA_LABEL, labelV );
      if( label == labelV && v->getProperty( SIA_SIZE, s ) )
	{
	  vertices.insert( v );
	  vec_valid = 1;
	  size += s;
	  if( v->getProperty( SIA_REFGRAVITY_CENTER, g ) )
	    {
	      x += g[0] * s;
	      y += g[1] * s;
	      z += g[2] * s;
	      ++n;
	    }
	  if( v->getProperty( SIA_DEPTH, d ) && d > depth )
	    depth = d;
	  if( v->getProperty( SIA_REFNORMAL, g ) && snrm < s )
	    {
	      snrm = s;
	      nrmx = g[0];
	      nrmy = g[1];
	      nrmz = g[2];
	      norm_valid = 1;
	    }
	  if( s >= _limitSize )
	    ++nb;	// nb big
	  else
	    ++ns;	// nb small

	  //cout << "avant vertex, sz= " << v->size() << "...";
	  //	relations intra-label
	  for( ie=v->begin(), fe=v->end(); ie!=fe; ++ie )
	    {
	      //cout << "s: " << (*ie)->getSyntax();
	      if( (*ie)->getSyntax() == SIA_HULLJUNCTION_SYNTAX )
		{
		  //cout << "hj... ";
		  if( (*ie)->getProperty( SIA_SIZE, surf ) )
		    hjs += surf;
		}
	      else if( pe.find( *ie ) == pe.end() )
		{
		  iv2 = (*ie)->begin();
		  if( *iv2 == v )
		    ++iv2;
		  if( (*iv2)->getProperty( SIA_LABEL, labelV ) 
		      && labelV == label )
		    {
		      if( (*ie)->getProperty( SIA_SIZE, surf ) )
			csurf += surf;
		      pe.insert( *ie );
		    }
		}
	    }
	}
    }

  if( n > 0 && size > 0 )
    {
      x /= size;
      y /= size;
      z /= size;
    }

  if( fc )
    {
      //cout << "connex peut-être pris dans le cache, fc = " << fc << "\n";
      //cout << "sz : " << fc->subVecValid.size() << endl;
      /*if( fc->subVecValid.size() > CORT_CC )
	cout << "sz OK, valid = " << fc->subVecValid[ CORT_CC ] << endl;
	else cout << "sz pas bonne: pas valide\n";*/
      if( fc->subVecValid.size() > CORT_CC && fc->subVecValid[ CORT_CC ] )
	{
	  //cout << "cort caché\n";
	  ccc = (unsigned) fc->inputVector[ CORT_CC ];
	}
      else
	{
	  //cout << "on recalcule cort\n";
	  ccc = vcl->connectivity( vertices, 0, SIA_CORTICAL_SYNTAX );
	}
      if( fc->subVecValid.size() > JUNC_CC && fc->subVecValid[ JUNC_CC ] )
	{
	  //cout << "junc caché\n";
	  ccj = (unsigned) fc->inputVector[ JUNC_CC ];
	}
      else
	{
	  //cout << "on recalcule junc\n";
	  ccj = vcl->connectivity( vertices, 0, SIA_JUNCTION_SYNTAX );
	}
      //cout << "connex OK\n";
    }
  else
    {
      //cout << "connex recalculées\n";
      ccc = vcl->connectivity( vertices, 0, SIA_CORTICAL_SYNTAX );
      ccj = vcl->connectivity( vertices, 0, SIA_JUNCTION_SYNTAX );
    }

  //	Remplissage du vecteur

  vec.push_back( vec_valid );

  vec.push_back( x );
  vec.push_back( y );
  vec.push_back( z );

  vec.push_back( size );
  vec.push_back( depth );

  vec.push_back( norm_valid );
  vec.push_back( nrmx );
  vec.push_back( nrmy );
  vec.push_back( nrmz );

  vec.push_back( (double) nb );
  vec.push_back( (double) ns );
  vec.push_back( csurf );
  vec.push_back( hjs );
  vec.push_back( ccc );
  vec.push_back( ccj );

  // *** DEBUG ***
  /*if( hjs == 0 )
    {
      cout << "hjs=0 : " << cl->size() << " noeuds, avec label: " << nb+ns 
	   << ".\n";
	   }*/

  return true;
}


bool FoldDescr::makeLearnVector( const Clique* cl, vector<double> & vec, 
				 GenericObject* model, double )
{
  return( makeVectorElements( cl, vec, model ) );	// provisoire
}


void FoldDescr::handleStats( const Clique*, vector<double> & vec, 
			     GenericObject*, double outp )
{
  //	stats sur l'orientation
  if( vec[NX] != 0 || vec[NY] != 0 || vec[NZ] != 0 )
    {
      if( _nnorm  == 0 )
	{
	  if( outp == -1 )	// seulement si c'est un bon exemple
	    {
	      _nx = vec[NX];
	      _ny = vec[NY];
	      _nz = vec[NZ];
	      ++_nnorm;
	    }
	  //	Adapter le vecteur lui-même
	  vec[NX] = 0;
	  vec[NY] = 0;
	  vec[NZ] = 0;
	}
      else
	{
	  if( outp == -1 )	// seulement si c'est un bon exemple
	    {
	      float	cosA = _nx * vec[NX] + _ny * vec[NY] + _nz * vec[NZ];
	      // cos de l'angle entre les 2 vecteurs
	      float	len;

	      if( cosA < 0 )	// mauvais côté: inverser le vecteur
		{
		  vec[NX] = -vec[NX];
		  vec[NY] = -vec[NY];
		  vec[NZ] = -vec[NZ];
		}
	      _nx = ( _nx * _nnorm + vec[NX] ) / ( _nnorm + 1 );
	      _ny = ( _ny * _nnorm + vec[NY] ) / ( _nnorm + 1 );
	      _nz = ( _nz * _nnorm + vec[NZ] ) / ( _nnorm + 1 );
	      len = sqrt( _nx*_nx + _ny*_ny + _nz*_nz );
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
	  //	Adapter le vecteur lui-même
	  vec[NX] -= _nx;
	  vec[NY] -= _ny;
	  vec[NZ] -= _nz;
	}

    }
}


void FoldDescr::preProcess( vector<double> & vec, GenericObject* )
{
  //	Utiliser les stats d'orientation

  vector<float>		nrm;

  if( _nnorm )
    {
      //	Adapter le vecteur lui-même
      vec[NX] -= _nx;
      vec[NY] -= _ny;
      vec[NZ] -= _nz;
    }
}


void FoldDescr::reset()
{
  _nnorm = 0;
  _nx = _ny = _nz = 0;
}


void FoldDescr::buildTree( Tree & t )
{
  t.setSyntax( SIA_FOLD_DESCRIPTOR );

  if( _nnorm )
    {
      t.setProperty( SIA_NSTATS_NORMAL, (int) _nnorm );
      vector<float>	v;
      v.push_back( _nx );
      v.push_back( _ny );
      v.push_back( _nz );
      t.setProperty( SIA_NORMAL, v );
    }
  t.setProperty( SIA_LIMIT_SIZE, (float) _limitSize );
}


bool FoldDescr::hasChanged( const Clique* cl, 
			    const map<Vertex*, string> & changes, 
			    const GenericObject* model ) const
{
  const VertexClique	*vcl = (const VertexClique *) cl;
  Model			*mod;
  TopModel		*tm = 0;

  if( !model || !model->getProperty( SIA_MODEL, mod ) 
      || !(tm=mod->topModel()) )
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
	(*iv)->getProperty( SIA_LABEL, label2 );
	if( label2 != vl && sl.find( label2 ) != fs )	// nouveau label
	  return( true );				// non-void
      }

  return( false );	// si rien n'a changé, on ne recalcule pas
}
