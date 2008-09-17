

#include <si/learner/learner.h>
#include <si/finder/adapFinder.h>
#include <si/model/adaptive.h>
#include <si/graph/attrib.h>
#include <iostream>

using namespace sigraph;
using namespace carto;
using namespace std;




Learner::Learner( bool allowsChildren, const string & synt ) 
  : Tree( allowsChildren, synt )
{
}


Learner::~Learner()
{
}


void	Learner::process(LearnConstParam *)
{
	cerr << "Non-const Learner process of a const Clique. Abort\n";
	assert( 0 );
}

