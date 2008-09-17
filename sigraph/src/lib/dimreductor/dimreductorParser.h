
#ifndef SI_FS_FSPARSER_H
#define SI_FS_FSPARSER_H


#include <si/dimreductor/dimreductor.h>
#include <si/graph/treeParser.h>


namespace sigraph
{
  class DimReductorParser : public TreePostParser
  {
  public:
    DimReductorParser();
    virtual ~DimReductorParser();

    virtual FactorySet factories();
    static void buildRanks(carto::AttributedObject* parent,
			Tree* t, const std::string &);
    static void buildMatrix(carto::AttributedObject* parent,
			Tree* t, const std::string &);
    static void buildFake(carto::AttributedObject* parent,
			Tree* t, const std::string &);

  protected:
    static void parseDimReductor(carto::AttributedObject* parent,
			Tree*, DimReductor *dimreductor);

  private:
  };


  //	inline

  inline DimReductorParser::DimReductorParser()
  {
  }


  inline DimReductorParser::~DimReductorParser()
  {
  }

}

#endif



