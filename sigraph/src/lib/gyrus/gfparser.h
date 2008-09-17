/*
 *  Copyright (C) 2003 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 */

#ifndef SI_GYRUS_GFPARSER_H
#define SI_GYRUS_GFPARSER_H


#include <si/finder/finderParser.h>


namespace sigraph
{

  class GFParser : public FinderParser
  {
  public:
    GFParser();
    virtual ~GFParser();

    virtual FactorySet factories();

    static void buildGyrusFinder( carto::AttributedObject* parent, Tree* t, 
                                  const std::string & filename );
  };

  //	inline

  inline GFParser::GFParser() : FinderParser()
  {
  }

}

#endif


