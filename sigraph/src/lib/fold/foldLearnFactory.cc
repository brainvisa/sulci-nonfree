
#include <si/fold/foldLearnFactory.h>
#include <si/fold/foldNoiser.h>
#include <si/fold/foldLabelsChanger.h>
#include <si/fold/foldReducedLabelsChanger.h>
#include <si/fold/foldFarLabelsChanger.h>
#include <si/fold/oneLabelTrier.h>
#include <si/fold/foldCopier.h>
#include <iostream>

using namespace sigraph;
using namespace std;


FoldLearnFactory::FoldLearnFactory() : LearnFactory()
{
}


FoldLearnFactory::~FoldLearnFactory()
{
}


TreeFactory* FoldLearnFactory::clone() const
{
  return( new FoldLearnFactory );
}


Tree* FoldLearnFactory::makeTree( const string & syntax, 
				  bool allowChildren ) const
{
  if( syntax == "fold_noiser" )
    {
      cout << "Creating FoldNoiser\n";
      return( new FoldNoiser );
    }
  else if( syntax == "fold_labels_changer" )
    {
      cout << "Creating FoldLabelsChanger\n";
      return( new FoldLabelsChanger );
    }
  else if( syntax == "fold_reduced_labels_changer" )
    {
      cout << "Creating FoldReducedLabelsChanger\n";
      return( new FoldReducedLabelsChanger );
    }
  else if( syntax == "fold_far_labels_changer" )
    {
      cout << "Creating FoldFarLabelsChanger\n";
      return( new FoldFarLabelsChanger );
    }
  else if( syntax == "fold_copier" )
    {
      cout << "Creating FoldCopier\n";
      return( new FoldCopier );
    }
  else if( syntax == "one_label_trier" )
    {
      cout << "Creating OneLabelTrier\n";
      return( new OneLabelTrier );
    }

  //cout << "syntax " << syntax << " not recognized in FoldLearnFactory\n";
  return( LearnFactory::makeTree( syntax, allowChildren ) );
}





