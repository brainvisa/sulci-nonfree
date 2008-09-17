
#ifndef SI_FOLD_FFPARSER_H
#define SI_FOLD_FFPARSER_H


#include <si/finder/finderParser.h>


namespace sigraph
{

  class FFParser : public FinderParser
  {
  public:
    FFParser();
    virtual ~FFParser();

    virtual FactorySet factories();

    static void buildFoldFinder( carto::AttributedObject* parent, Tree* t, 
				 const std::string & filename );

  protected:

  private:
  };

  //	inline

  inline FFParser::FFParser() : FinderParser()
  {
  }

}

#endif


