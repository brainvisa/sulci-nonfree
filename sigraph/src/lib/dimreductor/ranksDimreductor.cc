
#include <si/dimreductor/ranksDimreductor.h>
#include <graph/tree/tree.h>

using namespace sigraph;
using namespace carto;
using namespace std;


RanksDimReductor::~RanksDimReductor()
{
}

void RanksDimReductor::buildTree(Tree & tr) const
{
	tr.setSyntax("ranks_dimreduction");
	tr.setProperty("ranks", (std::vector<int>) _ranks);
	tr.setProperty("select", (int) _selected);
}
