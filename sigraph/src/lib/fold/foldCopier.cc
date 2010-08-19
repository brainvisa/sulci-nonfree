
#include <si/fold/foldCopier.h>
#include <si/graph/cliqueCache.h>
#include <si/fold/fattrib.h>
#include <si/graph/cgraph.h>
#include <si/graph/vertexclique.h>
#include <cartobase/exception/assert.h>

using namespace sigraph;
using namespace std;


FoldCopier::FoldCopier() : CopyLearner( "fold_copier" )
{
}


FoldCopier::~FoldCopier()
{
}


void	FoldCopier::process(LearnConstParam *lp)
{
  // cout << "  FoldCopier\n" << endl;
  const VertexClique	*vcl = (const VertexClique *) lp->clique;
  Graph			*bg;
  Graph			cg( "temporary_copy_graph" );
  VertexClique		*copy;
  Graph::const_iterator	ig, fg;
  const_iterator	it, ft;
  Learner		*lrn;
  set<Vertex *>		sv;
  CliqueCache		*cch = 0, *oldc;

  if( isLeaf() )
    return; // no childre, no use.

  ASSERT( lp->clique->getProperty( SIA_GRAPH, bg ) );
  lp->clique->getProperty( SIA_ORIGINAL_CACHE, cch );

  // cout << "FoldCopier : " << size() << " children\n";

  //	copie temporaire de la clique
  sv.insert( vcl->begin(), vcl->end() );

  Graph::const_iterator	iv, fv = bg->end();

  for( iv=bg->begin(); iv!=fv; ++iv )
    if( (*iv)->getSyntax() == SIA_HULL_SYNTAX )	// noeud hull (particulier)
    {
      sv.insert( *iv );
      break;
    }

  for( it=begin(), ft=end(); it!=ft; ++it )
  {
    bg->extract( cg, sv.begin(), sv.end() );
    copy = new VertexClique( *vcl );
    copy->setProperty( SIA_GRAPH, &cg );
    //	mark clique as copy
    //	( invaludate ModelFinder cache )
    copy->setProperty( "is_copy", true );
    if( cch )
    {
      if( copy->getProperty( SIA_CACHE, oldc ) )
        delete oldc;
      oldc = cch->clone();
      copy->setProperty( SIA_CACHE, oldc );
    }

    for( ig=cg.begin(), fg=cg.end(); ig!=fg; ++ig )
    {
      if( (*ig)->hasProperty( SIA_CLIQUES ) )
        (*ig)->removeProperty( SIA_CLIQUES );
      copy->addVertex( *ig );
    }

    lrn = dynamic_cast<Learner *>( *it );
    ASSERT( lrn );
    LearnParam lp2(*lp);
    lp2.clique = copy;
    lrn->process(&lp2);
    delete copy;
    cg.clear();
    cout << flush;
  }
}


