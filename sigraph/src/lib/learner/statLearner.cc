
#include <si/learner/statLearner.h>
#include <si/graph/mgraph.h>
//#include <si/finder/adapFinder.h>
#include <si/model/adaptiveLeaf.h>
#include <si/descr/adapDescr.h>
#include <si/graph/attrib.h>

using namespace sigraph;
using namespace std;


StatLearner::StatLearner( const string & synt ) : TerminalLearner( synt )
{
}


StatLearner::~StatLearner()
{
}


void StatLearner::process(LearnConstParam *lp)
{
	std::vector<double>     vec;

	lp->descr->makeStatsVector(lp->clique, vec,
		lp->adap->graphObject(), lp->outp);
	lp->adap->workEl().learnStats(vec, lp->outp);
}






