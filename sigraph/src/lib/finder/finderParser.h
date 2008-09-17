
#ifndef SI_FINDER_FINDERPARSER_H
#define SI_FINDER_FINDERPARSER_H

#include <si/graph/treeParser.h>


namespace sigraph
{

  class FinderParser : public TreePostParser
  {
  public:
    virtual ~FinderParser();

  protected:
    FinderParser();

  private:
  };

  //	inline

  inline FinderParser::FinderParser() : TreePostParser()
  {
  }

}

#endif


