
#include <si/fold/foldCopier.h>
#include <si/graph/cliqueCache.h>
#include <si/fold/fattrib.h>
#include <si/graph/cgraph.h>
#include <si/graph/vertexclique.h>

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
  const VertexClique	*vcl = (const VertexClique *) lp->clique;
  Graph			*bg;
  Graph			cg( "temporary_copy_graph" );
  VertexClique		*copy;
  Graph::const_iterator	ig, fg;
  const_iterator	it, ft;
  Learner		*lrn;
  set<Vertex *>		sv;
  CliqueCache		*cch = 0, *oldc;

  assert( !isLeaf() );	// si pas d'enfants, ça sert à rien

  assert( lp->clique->getProperty( SIA_GRAPH, bg ) );
  lp->clique->getProperty( SIA_ORIGINAL_CACHE, cch );

  //cout << "FoldCopier : " << size() << " children\n";

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
      //cout << "nouvelle copie...\n";
      bg->extract( cg, sv.begin(), sv.end() );
      //cout << "extract() OK\n";
      copy = new VertexClique( *vcl );
      copy->setProperty( SIA_GRAPH, &cg );
      //	marquer la clique comme copie
      //	( pour désactiver le cache éventuel du ModelFinder )
      copy->setProperty( "is_copy", true );
      if( cch )
	{
	  if( copy->getProperty( SIA_CACHE, oldc ) )
	    {
	      //cout << "il y avait déjà un cache\n";
	      delete oldc;
	    }
	  //cout << "copie du cache: " << cch << ", copy = " << copy << "\n";
	  oldc = cch->clone();
	  //cout << "cache créé\n";
	  copy->setProperty( SIA_CACHE, oldc );
	  //cout << "copie du cache OK\n";
	}
      //else cout << "pas de cache\n";

      for( ig=cg.begin(), fg=cg.end(); ig!=fg; ++ig )
	{
	  if( (*ig)->hasProperty( SIA_CLIQUES ) )
	    (*ig)->removeProperty( SIA_CLIQUES );
	  copy->addVertex( *ig );
	}

      //cout << "Copy clique : " << copy->size() << " noeuds\n";
      lrn = dynamic_cast<Learner *>( *it );
      assert( lrn );
      LearnParam lp2(*lp);
      lp2.clique = copy;
      lrn->process(&lp2);
      //cout << "FoldCopier::process() effectué\n";
      delete copy;
      //cout << "copie effacée\n";
      cg.clear();
      //cout << "graphe temporaire nettoyé\n";
    }
}


