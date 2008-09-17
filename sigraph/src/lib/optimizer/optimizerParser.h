
#ifndef SI_OPTIMIZER_OPTIMIZERPARSER_H
#define SI_OPTIMIZER_OPTIMIZERPARSER_H


#include <si/optimizer/optimizer.h>
#include <si/graph/treeParser.h>


namespace sigraph
{
  class OptimizerParser : public TreePostParser
  {
  public:
    OptimizerParser();
    virtual ~OptimizerParser();

    virtual FactorySet factories();
    static void buildGrid(carto::AttributedObject* parent,
			Tree* t, const std::string &);

  protected:
    static void parseOptimizer(carto::AttributedObject* parent,
			Tree*, Optimizer *optimizer);

  private:
  };


  //	inline

  inline OptimizerParser::OptimizerParser() {}

  inline OptimizerParser::~OptimizerParser() {}

}

#endif



