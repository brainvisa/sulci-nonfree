
#include <si/domain/nullDomain.h>
#include <graph/tree/tree.h>

using namespace sigraph;


NullDomain::~NullDomain()
{
}


void NullDomain::buildTree( Tree & tr ) const
{
  tr.setSyntax( "null_domain" );
}


