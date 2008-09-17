
#ifndef SI_DESCR_DESCRPARSER_H
#define SI_DESCR_DESCRPARSER_H


#include <si/graph/treeParser.h>


namespace sigraph
{
  class CliqueDescr;

  class DescrParser : public TreePostParser
  {
  public:
    DescrParser();
    virtual ~DescrParser();
    virtual void registerFactory( const std::string & name, 
				  const Factory & fac ) = 0;

  protected:
    static void parseDescr( carto::AttributedObject* parent, Tree* t, 
			    CliqueDescr* cd );

  private:
  };

  //	inline

  inline DescrParser::DescrParser()
  {
  }


  inline DescrParser::~DescrParser()
  {
  }

}

#endif

