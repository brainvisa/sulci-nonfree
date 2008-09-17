
#include <si/learner/labelsChanger.h>
#include <si/model/model.h>
#include <si/model/topModel.h>
#include <si/graph/mgraph.h>
#include <si/finder/modelFinder.h>
#include <si/graph/vertexclique.h>
#include <si/graph/cliqueCache.h>
#include <aims/math/random.h>

using namespace sigraph;
using namespace carto;
using namespace std;


double LabelsChanger::noise( Clique* cl, double & outp )
{
  //	changements de labels

  AttributedObject		*modV;
  Model				*mod;
  BaseTree			*top = getTopParent();
  Learner			*bl = dynamic_cast<Learner *>( top );
  MGraph			*mg;
  TopModel			*tm;

  assert( bl );
  bl->getProperty( "model", mg );

  ModelFinder	& mf = mg->modelFinder();

  modV = mf.selectModel( cl );

  modV->getProperty( "model", mod );
  tm = mod->topModel();

  if( tm )
    {
      set<string>	& sl = tm->significantLabels();
      const string	& vl = tm->voidLabel();

      if( sl.size() != 0 && vl != "" )	// définis
	return( constrainedNoise( cl, outp, sl, vl ) );
    }

  return( openNoise( cl, outp ) );	// tous changements autorisés
}


double LabelsChanger::constrainedNoise( Clique* cl, double &, 
					const set<string> & sl, 
					const string & vl )
{
  VertexClique::const_iterator	iv, fv=((const VertexClique *) cl)->end();
  // noeuds potentiellement ajoutés, enlevés, changés
  set<Vertex *>			an, rn;
  map<Vertex*, set<string>* >	cn;	// liste de labels prenables associée
  string			label, oldlabel;
  set<string>::const_iterator	fsl = sl.end();
  Vertex			*v;
  vector<string>		*plv;
  set<string>			*inter;
  CliqueCache			*cch = 0;

  // Distance a l'exemple d'origine
  double			dist = 0;

  if( !cl->getProperty( "cache", cch ) )
    cl->getProperty( "original_cache", cch );

  //	tri des noeuds de la clique en 3 catégories
  for( iv=((const VertexClique *) cl)->begin(); iv!=fv; ++iv )
    {
      v = *iv;
      v->getProperty( "label", label );
      if( label == vl || sl.find( label ) == fsl )
	an.insert( v );		// ajoutable
      else
	{
	  rn.insert( v );	// enlevable
	  assert( v->getProperty( "possible_labels", plv ) );
	  //	copie ordonnée (oui: pas optimal)
	  set<string>	pl( plv->begin(), plv->end() );
	  //	intersection des ensembles pl et sl
	  inter = new set<string>;
	  intersection( pl, sl, *inter );
	  inter->erase( vl );		// vl est dedans en principe
	  inter->erase( label );	// enlever le label actuel
	  if( inter->size() > 0 )
	    cn[ v ] = inter;
	  else
	    delete inter;
	}
    }

  if( an.size() + rn.size() == 0 )	// on peut rien changer
    return( false );

  unsigned	na, nr, nc;		// nombres de changements

  na = randomGen( an.size() );	// nb d'ajouts effectifs
  nr = randomGen( rn.size() );	// nb d'enlevés + changés
  if( na + nr == 0 )		// faut que ça fasse au moins 1
    {
      if( an.size() == 0 )
	++nr;
      else if( rn.size() == 0 || UniformRandom() < 0.5 )
	++na;
      else
	++nr;
    }

  unsigned	maxc = cn.size();
  if( maxc > nr )
    maxc = nr;
  nc = (unsigned ) ( UniformRandom( 0U, maxc ) );

  nr -= nc;	// ceux qui sont changés ne sont pas enlevés

  // bon, maintenant qu'on sait combien on change de chaque, il faut le faire

  map<Vertex*, set<string>* >::iterator	ic, fc=cn.end();
  set<unsigned>				todo;
  set<unsigned>::iterator		it, ft;
  unsigned				i, j, num;
  set<string>::iterator			il;
  bool					changed = false;

  // changements parmi les labels significatifs non-void

  while( todo.size() < nc )
    todo.insert( unsigned ( UniformRandom( (const unsigned &) 0U, 
                                        (const unsigned &) cn.size() - 1 ) ) );
  //cout << "changements : " << nc << " / " << cn.size() << endl;
  //int	toto;

  for( ic=cn.begin(), it=todo.begin(), ft=todo.end(), i=0; it!=ft; ++ic, ++i )
    {
      if( i == *it )	// on change celui-là
	{
	  inter = (*ic).second;
	  //	tirage du nouveau label
	  num = (unsigned) ( UniformRandom( (const unsigned &) 0U, 
                                    (const unsigned &) inter->size() - 1 ) );
	  for( j=0, il=inter->begin(); j<num; ++j )
	    ++il;
	  (*ic).first->getProperty( "label", oldlabel );
	  (*ic).first->setProperty( "label", *il );
	  // enlever ce noeud de la liste de ceux qu'on peut encore enlever
	  rn.erase( (*ic).first );
	  ++it;	// passer au suivant
	  changed = true;
	  //(*ic).first->getProperty( "skeleton_label", toto );
	  //cout << toto << "  ";

	  // Distance a l'exemple d'origine
	  dist += distance( (*ic).first, oldlabel, *il, cch );
	}
      delete (*ic).second;	// nettoyage
    }
  for( ; ic!=fc; ++ic )
    delete (*ic).second;	// finir de nettoyer

  //cout << endl;

  //	enlèvements de labels significatifs (-> void)

  set<Vertex *>::iterator	ir;

  todo.erase( todo.begin(), todo.end() );
  while( todo.size() < nr )
    todo.insert( (unsigned) ( UniformRandom( (const unsigned &) 0U, 
                                        (const unsigned &) rn.size() - 1 ) ) );
  //cout << "enleves : " << nr << " / " << rn.size() << endl;

  for( ir=rn.begin(), i=0, it=todo.begin(), ft=todo.end(); it!=ft; ++it )
    {
      //	positionner l'itérateur de noeuds
      while( i < *it )
	{
	  ++i;
	  ++ir;
	}
      //	mettre au label void
      (*ir)->getProperty( "label", oldlabel );
      (*ir)->setProperty( "label", vl );
      changed = true;
      //(*ir)->getProperty( "skeleton_label", toto );
      //cout << toto << "  ";

      // Distance a l'exemple d'origine
      dist += distance( *ir, oldlabel, vl, cch );
    }
  //cout << endl;

  //	ajouts de noeuds (label void -> significatif)

  todo.erase( todo.begin(), todo.end() );
  //cout << "ajouts : " << na << " / " << an.size() << endl;
  while( todo.size() < na )
    todo.insert( (unsigned) ( UniformRandom( (const unsigned &) 0U, 
                                        (const unsigned &) an.size() - 1 ) ) );

  for( ir=an.begin(), i=0, it=todo.begin(), ft=todo.end(); it!=ft; ++it )
    {
      //	positionner l'itérateur de noeuds
      while( i < *it )
	{
	  ++i;
	  ++ir;
	}
      //	choix du nouveau label: dans inter( sl, pl ) \ vl
      (*ir)->getProperty( "possible_labels", plv );
      set<string>	pl( plv->begin(), plv->end() );
      set<string>	poss;
      intersection( pl, sl, poss );
      poss.erase( vl );

      if( poss.size() != 0 )
	{
	  // tirage parmi ceux-là
	  num = (unsigned) ( UniformRandom( (const unsigned &) 0U, 
                                        (const unsigned &) poss.size() - 1 ) );
	  for( j=0, il=poss.begin(); j<num; ++j )
	    ++il;
	  (*ir)->getProperty( "label", oldlabel );
	  (*ir)->setProperty( "label", *il );
	  //(*ir)->getProperty( "skeleton_label", toto );
	  //cout << toto << "  ";
	  changed = true;
	  // Distance a l'exemple d'origine
	  dist += distance( *ir, oldlabel, *il, cch );
	}
      //else
      //cerr << "pas de label significatif pour un noeud.\n";
    }
  //cout << endl;

  //	Bon, moi je trouve ça compliqué, tâtu et tout, comme fonction.
  //	j'ai mal au crâne...

  return( changed ? fabs( dist ) : 0. );
}


unsigned LabelsChanger::randomGen( unsigned n )
{
  double fate = UniformRandom();
  return( (unsigned) ( fate * fate * (n+0.99) ) );
  // bon, c'est de la bricole... d'ac...

  //return( (unsigned) ( fate * (n+0.99) ) );
}


double LabelsChanger::openNoise( Clique* cl, double & )
{
  bool				trans = false, poss = true;
  VertexClique::const_iterator	iv, fv=((const VertexClique *) cl)->end();
  const double			proba = 0.5;
  vector<string>		*pl;
  unsigned			num;
  string			oldlabel;

  while( !trans && poss )
    {
      poss = false;
      for( iv=((const VertexClique *) cl)->begin(); iv!=fv; ++iv )
	{
	  (*iv)->getProperty( "possible_labels", pl );
	  if( pl->size() >= 2 )
	    {
	      poss = true;
	      if( UniformRandom() <= proba )
		{	// change label
		  num = UniformRandom( 0U, (unsigned) pl->size()-2 );
		  (*iv)->getProperty( "label", oldlabel );
		  if( (*pl)[num] == oldlabel )
		    num = pl->size()-1;
		  (*iv)->setProperty( "label", (*pl)[num] );
		  trans = true;
		  poss = true;
		}
	    }
	}
    }

  return( trans ? 1. : 0. );
}


template< class T >
void LabelsChanger::intersection( const set<T> & s1, const set<T> & s2, 
				  set<T> & s3 ) const
{
  typename set<T>::const_iterator	i1=s1.begin(), i2=s2.begin(), 
    f1=s1.end(), f2=s2.end();

  while( i1!=f1 && i2!=f2 )
    {
      if( *i1 == *i2 )
	{
	  s3.insert( *i1 );
	  ++i1;
	  ++i2;
	}
      else if( *i1 < *i2 )
	++i1;
      else
	++i2;
    }
}


// for gcc-3.1 on Darwin at least
template void LabelsChanger::intersection( const set<string> &, 
                                           const set<string> &, 
                                           set<string> & ) const;

