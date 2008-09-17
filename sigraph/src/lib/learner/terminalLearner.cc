
#include <si/learner/terminalLearner.h>
#include <si/graph/mgraph.h>
#include <si/finder/adapFinder.h>
#include <si/descr/adapDescr.h>

using namespace sigraph;
using namespace std;


TerminalLearner::TerminalLearner( const string & synt ) 
  : ConstLearner( false, synt )
{
}


TerminalLearner::~TerminalLearner()
{
}


void	TerminalLearner::process(LearnConstParam *lp)
{
	lp->descr->addGeneratedVector(lp);
}





