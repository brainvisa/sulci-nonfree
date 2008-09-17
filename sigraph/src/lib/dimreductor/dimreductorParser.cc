
#include <si/dimreductor/dimreductorParser.h>
#include <si/dimreductor/fakeDimreductor.h>
#include <si/dimreductor/ranksDimreductor.h>
#include <si/dimreductor/matrixDimreductor.h>
#include <si/model/adaptiveLeaf.h>
#include <graph/tree/tree.h>
#include <iostream>

using namespace sigraph;
using namespace carto;
using namespace std;


TreePostParser::FactorySet DimReductorParser::factories()
{
	TreePostParser::FactorySet	dimreductor;

	dimreductor[ "ranks_dimreduction" ] = buildRanks;
	dimreductor[ "matrix_dimreduction" ] = buildMatrix;
	dimreductor[ "fake_dimreduction" ] = buildFake;

	return dimreductor;
}


void DimReductorParser::buildFake(AttributedObject* parent, Tree* t, const string &)
{
	std::vector<float>		vec;
	parent->getProperty("mean", vec);
	DimReductor	*dimreductor = new FakeDimReductor(vec.size());

	t->setProperty("pointer", (DimReductor *) dimreductor);
	parseDimReductor(parent, t, dimreductor);
}


void DimReductorParser::buildRanks(AttributedObject* parent, Tree* t, const string &)
{
	std::vector<int>		vd;
	int				d;
	std::string			s;

	std::vector<int>::const_iterator	i, f;
	t->getProperty("ranks", vd);
	t->getProperty("select", d);
		
	DimReductor	*dimreductor = new RanksDimReductor(vd, d);

	t->setProperty("pointer", (DimReductor *) dimreductor);
	parseDimReductor(parent, t, dimreductor);
}


void DimReductorParser::buildMatrix(AttributedObject* parent, Tree* t, const string &)
{
	std::vector<float>		matrix;
	std::vector<int>		shape;
	int				selected;
	std::string			mode;

	t->getProperty("matrix", matrix);
	t->getProperty("shape", shape);
	t->getProperty("select", selected);

	if (shape.size() != 2)
	{
		std::cerr << "warning : shape must have 2 components, found "
			  << shape.size() << "." << std::endl;
		return;
	}
	if (shape[0] * shape[1] != ((int) matrix.size()))
	{
		std::cerr << "warning : inconsistent shape and data : number "
			     "of components in matrix according to shape : "
			  << shape[0] * shape[1] << ", against "
			  << matrix.size() << " according to data."
			  << std::endl;
		return;
	}
	if (selected < 1 or selected > shape[1])
	{
		std::cerr << "warning : bad number of selected transformed "
			  << "features. It should verify : "
			  << "0 < selected (" << selected << ") < "
			  << shape[1] << " (shape[1])" << std::endl;
		return;
	}
		
	DimReductor	*dimreductor = new MatrixDimReductor(matrix,
						shape, selected);

	t->setProperty("pointer", (DimReductor *) dimreductor);
	parseDimReductor(parent, t, dimreductor);
}


void DimReductorParser::parseDimReductor(AttributedObject* parent, Tree *ao, DimReductor *dimreductor)
{
	Model		*mod = NULL;
	AdaptiveLeaf	*al = NULL;

	if(ao->size())
	{
		std::cerr << "warning : DimReductor with children ("
			  << ao->size() << ")" << std::endl;;
		return;
	}
	if (!parent)
	{
		std::cerr << "DimReductor without a parent!" << std::endl;
		return;
	}
	if (parent->getSyntax() != "ad_leaf")
	{
		std::cerr << "DimReductor parent is NOT an ad_leaf!" << std::endl;
		return;
	}
	if (!parent->getProperty("pointer", mod))
	{
		std::cerr << "DimReductor parent has no pointer!"<< std::endl;
		return;
	}
	if ((al = dynamic_cast<AdaptiveLeaf *>(mod)))
		al->setDimReductor(dimreductor);
	else	std::cerr << "DimReductor parent is not an AdLeaf" << std::endl;
}



