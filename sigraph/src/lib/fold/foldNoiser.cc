
#include <si/fold/foldNoiser.h>

using namespace sigraph;
using namespace std;


FoldNoiser::FoldNoiser() : Noiser( "fold_noiser" )
{
}


FoldNoiser::~FoldNoiser()
{
}


double FoldNoiser::noise( Clique*, double & )
{
  return( 1. );
}





