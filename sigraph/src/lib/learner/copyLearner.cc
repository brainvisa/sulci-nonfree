
#include <si/learner/copyLearner.h>
#include <si/graph/cliqueCache.h>
#include <graph/graph/graph.h>

using namespace sigraph;
using namespace std;


CopyLearner::CopyLearner() : ConstLearner(true, "copier")
{
}


CopyLearner::~CopyLearner()
{
}


void CopyLearner::process(LearnConstParam *lp)
{
	Clique		*copy;
	const_iterator	it, ft;
	Learner		*lrn;

	assert(!isLeaf());	// si pas d'enfants, ça sert à rien

	for(it = begin(), ft = end(); it != ft; ++it)
	{
		copy = lp->clique->deepCopy();
		lrn = dynamic_cast<Learner *>(*it);
		assert(lrn);
		LearnParam lp2(*lp);
		lp2.clique = copy;
		lrn->process(&lp2);
		delete copy;
	}
}





