
#include <si/optimizer/optimizerParser.h>
#include <si/optimizer/gridOptimizer.h>
#include <si/model/adaptiveLeaf.h>
#include <graph/tree/tree.h>
#include <iostream>

using namespace sigraph;
using namespace carto;
using namespace std;


TreePostParser::FactorySet OptimizerParser::factories()
{
  TreePostParser::FactorySet	optimizer;

  optimizer[ "grid_optimizer" ] = buildGrid;

  return optimizer;
}


void OptimizerParser::buildGrid(AttributedObject* parent, Tree* t, const string &)
{
  std::string			strategy;

  std::vector<int>::const_iterator	i, f;

  Object	dic = t->getProperty("parameters");
  t->getProperty("strategy", strategy);
  Optimizer	*optimizer = new GridOptimizer(dic, strategy);

  t->setProperty("pointer", (Optimizer *) optimizer);
  parseOptimizer(parent, t, optimizer);
}

void OptimizerParser::parseOptimizer(AttributedObject* parent, Tree *ao, Optimizer *optimizer)
{
  Model		*mod = NULL;
  AdaptiveLeaf	*al = NULL;

  if(ao->childrenSize())
  {
    std::cerr << "warning : OPTIMIZER with children ("
              << ao->childrenSize() << ")" << std::endl;;
    return;
  }
  if (!parent)
  {
    std::cerr << "optimizer without a parent!" << std::endl;
    return;
  }
  if (parent->getSyntax() != "ad_leaf")
  {
    std::cerr << "optimizer parent is NOT an ad_leaf!" << std::endl;
    return;
  }
  if (!parent->getProperty("pointer", mod))
  {
    std::cerr << "optimizer parent has no pointer!"<< std::endl;
    return;
  }
  if ((al = dynamic_cast<AdaptiveLeaf *>(mod)))
    al->setOptimizer(optimizer);
  else
    std::cerr << "optimizer parent is not an AdLeaf" << std::endl;
}



