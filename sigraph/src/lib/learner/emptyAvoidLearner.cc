
#include <si/learner/emptyAvoidLearner.h>
#include <si/model/model.h>
#include <si/graph/mgraph.h>
#include <si/model/topModel.h>
#include <si/finder/modelFinder.h>
#include <si/graph/attrib.h>
#include <si/graph/vertexclique.h>
#include <math.h>

using namespace sigraph;
using namespace carto;
using namespace std;


EmptyAvoidLearner::EmptyAvoidLearner( bool allowsChildren, 
				      const string & synt )
  : ConstLearner( allowsChildren, synt )
{
}


EmptyAvoidLearner::~EmptyAvoidLearner()
{
}


void EmptyAvoidLearner::process(LearnParam *lp)
{
  checkClique(lp->clique, lp->outp);
  //    outp = 0;

  ConstLearner::process(lp);
}


void EmptyAvoidLearner::process(LearnConstParam *lp)
{
  checkClique(lp->clique, lp->outp);
  //    outp = 0;

  ConstLearner::process(lp);
}


bool EmptyAvoidLearner::checkClique( const Clique* cl, double & outp )
{
  const VertexClique	*vcl = (const VertexClique *) cl;
  AttributedObject	*modV;
  Model			*mod;
  BaseTree		*top = getTopParent();
  Learner		*bl = dynamic_cast<Learner *>( top );
  MGraph		*mg;
  TopModel		*tm;

  assert( bl );
  bl->getProperty( SIA_MODEL, mg );

  ModelFinder	& mf = mg->modelFinder();

  modV = mf.selectModel( cl );

  modV->getProperty( SIA_MODEL, mod );
  tm = mod->topModel();

  if( !tm )
    return( false );

  set<string>	& sl = tm->significantLabels();
  const string	& vl = tm->voidLabel();

  if( sl.size() == 0 || vl == "" )
    return( false );

  VertexClique::const_iterator	iv, fv=vcl->end();
  string			label;
  set<string>::const_iterator	fl = sl.end();
  unsigned			n = sl.size();	// nb labels significatifs 
						// non-void
  if( sl.find( vl ) != fl )
    --n;

  // sillon / relation n'existe pas si un au moins des labels significatifs 
  // non-void ne sont pas représentés

  set<string>			isthere;
  bool				isrel = false;
  Vertex::const_iterator	ie, fe;
  Edge::const_iterator		iv2;
  string			label2;

  for( iv=vcl->begin(); iv!=fv; ++iv )
    if( (*iv)->getProperty( SIA_LABEL, label ) 
	&& label != vl && sl.find( label ) != fl )
      {
	isthere.insert( label );	// label significatif non-void
	if( !isrel && n == 2 )		// cas clique de relation
	  {
	    for( ie=(*iv)->begin(), fe=(*iv)->end(); !isrel && ie!=fe; ++ie )
	      {
		iv2 = (*ie)->begin();	// autre bout de la relation
		if( *iv2 == *iv )
		  ++iv2;
		if( (*iv2)->getProperty( SIA_LABEL, label2 ) 
		    && label2 != label && label2 != vl 
		    && sl.find( label2 ) != fl )
		  isrel = true;		// relation entre les 2 bons labels
	      }
	    if( isrel )
	      return( false );		// c'est bon, tout est là maintenant
	  }
	else if( isthere.size() == n )	// tout est là
	  return( false );		// OK, pas de complication
      }

  //	maintenant on est dans le cas où il manque un truc

  int	p, ng;

  if( !mg->getProperty( SIA_NBASEGRAPHS, ng ) || ng == 0 )
    {	// on ne sait pas combien de graphes il y avait dans les stats
      outp = 0;	// on ne peut rien faire de mieux
      return( true );
    }

  if( !modV->getProperty( SIA_NOINSTANCE_COUNT, p ) )
    p = 0;

  double	f = 1. - double(p)/ng;	// fréquence d'apparition de cet expert
  double	thld = 0.85;	// centrage de la sigmoïde

  outp = 0.5;
  getProperty( SIA_MANDATORY_EMPTY_OUTPUT, outp );
  getProperty( SIA_EMPTY_PENALTY_THRESHOLD, thld );
  // sigmoïde recentrée
  outp = outp / ( 1 + exp( -(f - thld) * 40 ) );

  return( true );
}





