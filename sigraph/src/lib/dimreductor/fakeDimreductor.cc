
#include <si/dimreductor/fakeDimreductor.h>
#include <graph/tree/tree.h>

using namespace sigraph;
using namespace carto;
using namespace std;


FakeDimReductor::~FakeDimReductor()
{
}

void FakeDimReductor::buildTree(Tree & tr) const
{
	tr.setSyntax("fake_dimreduction");
}
