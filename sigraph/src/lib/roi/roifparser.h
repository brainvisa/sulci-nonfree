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

#ifndef SI_ROI_RFPARSER_H
#define SI_ROI_RFPARSER_H


#include <si/finder/finderParser.h>


namespace sigraph
{

  class RoiFParser : public FinderParser
  {
  public:
    RoiFParser();
    virtual ~RoiFParser();

    virtual FactorySet factories();

    static void buildRoiFinder( carto::AttributedObject* parent, Tree* t, 
                                const std::string & filename );
  };

  //	inline

  inline RoiFParser::RoiFParser() : FinderParser()
  {
  }

}

#endif


