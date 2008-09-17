
#include <si/domain/adapDomain.h>
#include <graph/tree/tree.h>
#include <iostream>

using namespace sigraph;
using namespace std;


void AdapDomain::learn( const Vertex*, const Graph* )
{
  cout << "AdapDomain::learn\n";
  ++_ndata;
}


void AdapDomain::buildTree( Tree & tr ) const
{
  tr.setProperty( "ndata", (int) _ndata );
}





