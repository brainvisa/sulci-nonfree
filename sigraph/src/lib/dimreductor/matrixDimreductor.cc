
#include <si/dimreductor/matrixDimreductor.h>
#include <graph/tree/tree.h>

using namespace sigraph;
using namespace carto;
using namespace std;


MatrixDimReductor::~MatrixDimReductor()
{
}

void MatrixDimReductor::buildTree(Tree &tr) const
{
	tr.setSyntax("matrix_dimreduction");
	tr.setProperty("matrix", (std::vector<float>) _matrix);
	tr.setProperty("shape", (std::vector<int>) _shape);
	tr.setProperty("select", (int) _selected);
}
