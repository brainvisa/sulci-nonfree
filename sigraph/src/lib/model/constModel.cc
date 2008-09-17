
#include <si/model/constModel.h>
#include <graph/tree/tree.h>

using namespace sigraph;
using namespace std;


ConstModel::~ConstModel()
{
}


void ConstModel::buildTree( Tree & tr ) const
{
  tr.setSyntax( "const_model" );
  tr.setProperty( "value", (float) _value );
}




