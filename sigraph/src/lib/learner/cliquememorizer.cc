
#include <si/learner/cliquememorizer.h>
#include <si/graph/vertexclique.h>
#include <si/graph/cliqueCache.h>
#include <graph/graph/graph.h>
#include <si/graph/attrib.h>

using namespace sigraph;
using namespace std;


CliqueMemorizer::CliqueMemorizer() : ConstLearner(true, "clique_memorizer")
{
}


CliqueMemorizer::~CliqueMemorizer()
{
}


void CliqueMemorizer::process( LearnConstParam *lp )
{
  if( isLeaf() )	// if no children, this is not useful
    return;

  const VertexClique		*vc;
  VertexClique::const_iterator	iv, ev;
  const_iterator	it, ft;
  Learner		*lrn;
  string		label;
  map<Vertex *, string>::iterator	il, el;
  LearnParam		lp2( *lp );

  vc = dynamic_cast<const VertexClique *>( lp->clique );
  if( !vc )
    {
      cerr << "CliqueMemorizer: cannot memorize non vertex-based clique" 
           << endl;
      return;
    }
  VertexClique	*vcm = const_cast<VertexClique *>( vc );
  lp2.clique = vcm;
  // remove caches
  CliqueCache	*cc = 0, *cco = 0, *ncc;
  vc->getProperty( SIA_ORIGINAL_CACHE, cco );
  if( vc->getProperty( SIA_CACHE, cc ) )
    cco = cc;

  for(it = begin(), ft = end(); it != ft; ++it)
    {
      map<Vertex *, string> memo;
      lrn = dynamic_cast<Learner *>(*it);
      assert(lrn);

      // store labels
      for( iv=vc->begin(), ev=vc->end(); iv!=ev; ++iv )
        if( (*iv)->getProperty( SIA_LABEL, label ) )
          memo[ *iv ] = label;
      // build a new cache
      if( cco )
        {
          ncc = cco->clone();
          vcm->setProperty( SIA_CACHE, ncc );
        }
      // learn / generate vectors
      lrn->process( &lp2 );
      // restore former labels
      for( il=memo.begin(), el=memo.end(); il!=el; ++il )
        il->first->setProperty( SIA_LABEL, il->second );
      if( cco )
        delete ncc;
    }

  // restore caches
  if( cc )
    vcm->setProperty( SIA_CACHE, cc );
  else
    vcm->removeProperty( SIA_CACHE );
}


