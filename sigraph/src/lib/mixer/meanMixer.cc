
#include <si/mixer/meanMixer.h>
#include <graph/tree/tree.h>

using namespace sigraph;
using namespace std;


MeanMixer::~MeanMixer()
{
}


void MeanMixer::buildTree( Tree & tr )
{
  tr.setSyntax( "mean_mixer" );
}






