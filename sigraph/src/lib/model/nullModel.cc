
#include <si/model/nullModel.h>
#include <graph/tree/tree.h>

using namespace sigraph;
using namespace std;


NullModel::~NullModel()
{
}


void NullModel::buildTree( Tree & tr ) const
{
  tr.setSyntax( "null_model" );
}




