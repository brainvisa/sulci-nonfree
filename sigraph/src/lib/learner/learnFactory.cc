
#include <si/learner/learnFactory.h>
#include <si/learner/copyLearner.h>
#include <si/learner/badLearner.h>
#include <si/learner/terminalLearner.h>
#include <si/learner/statLearner.h>
#include <si/learner/selectiveLearner.h>
#include <si/learner/labelsChanger.h>
#include <si/learner/emptyAvoidLearner.h>
#include <si/learner/cliquememorizer.h>
#include <iostream>

using namespace sigraph;
using namespace std;


LearnFactory::LearnFactory() : TreeFactory()
{
}


LearnFactory::~LearnFactory()
{
}


TreeFactory* LearnFactory::clone() const
{
  return( new LearnFactory );
}


Tree* LearnFactory::makeTree( const string & syntax, 
			      bool allowChildren ) const
{
  if( syntax == "copier" )
    {
      cout << "Creating CopyLearner\n";
      return( new CopyLearner );
    }
  else if( syntax == "terminal_learner" )
    {
      cout << "Creating TerminalLearner\n";
      return( new TerminalLearner );
    }
  else if( syntax == "bad_learner" )
    {
      cout << "Creating BadLearner\n";
      return( new BadLearner );
    }
  else if( syntax == "stats_learner" )
    {
      cout << "Creating StatLearner\n";
      return( new StatLearner );
    }
  else if( syntax == "const_learner" )
    {
      cout << "Creating ConstLearner\n";
      return( new ConstLearner );
    }
  else if( syntax == "labels_changer" )
    {
      cout << "Creating LabelsChanger\n";
      return( new LabelsChanger );
    }
  else if( syntax == "selective_learner" )
    {
      cout << "Creating SelectiveLearner\n";
      return( new SelectiveLearner );
    }
  else if( syntax == "empty_avoid_learner" )
    {
      cout << "Creating EmptyAvoidLearner\n";
      return( new EmptyAvoidLearner );
    }
  else if( syntax == "clique_memorizer" )
    {
      cout << "Creating CliqueMemorizer\n";
      return( new CliqueMemorizer );
    }

  //cout << "syntax " << syntax << " not recognized in LearnFactory\n";
  return( TreeFactory::makeTree( syntax, allowChildren ) );
}






