
#include <si/optimizer/gridOptimizer.h>
#include <graph/tree/tree.h>

using namespace sigraph;
using namespace carto;
using namespace std;


GridOptimizerParameter::GridOptimizerParameter() : carto::PropertySet()
{
	initBuiltins();
}

GridOptimizerParameter::GridOptimizerParameter(const std::string &name) :
carto::PropertySet(), _name(name)
{
	initBuiltins();
}

GridOptimizerParameter::GridOptimizerParameter(const GridOptimizerParameter &g):
RCObject(), Interface(), SizeInterface(), IterableInterface(),
carto::PropertySet(g), _name(g._name), _ranges(g._ranges), _scale(g._scale)
{
	initBuiltins();
}


void	GridOptimizerParameter::initBuiltins()
{
	addBuiltinProperty("ranges", _ranges);
	addBuiltinProperty("scale", _scale);
}




GridOptimizer::GridOptimizer(const carto::Object &parameters,
	const std::string &strategy) :
	_parameters(parameters), _strategy(strategy) {}

GridOptimizer::~GridOptimizer()
{
}

void GridOptimizer::buildTree(Tree & tr) const
{
	tr.setSyntax("grid_optimizer");
	tr.setProperty("strategy", (std::string) _strategy);
	tr.setProperty("parameters", (carto::Object) _parameters);
}


#include <cartobase/object/object_d.h>
#include <cartobase/object/syntobject_d.h>

namespace carto
{
	INSTANTIATE_GENERIC_OBJECT_TYPE(GridOptimizerParameter)
}

