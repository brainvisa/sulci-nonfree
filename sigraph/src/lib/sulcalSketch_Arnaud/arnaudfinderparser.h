/*
 *  Copyright (C) 2004 CEA
 *
 *  This software and supporting documentation were developed by
 *
 *      Olivier Coulon
 *      Laboratoire LSIS,
 *      Marseille, France
 *
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 */

#ifndef SI_ARNAUD_ARNAUDFINDERPARSER_H
#define SI_ARNAUD_ARNAUDFINDERPARSER_H


#include <si/finder/finderParser.h>


namespace sigraph
{

  class ArnaudFinderParser : public FinderParser
  {
  public:
    ArnaudFinderParser();
    virtual ~ArnaudFinderParser();

    virtual FactorySet factories();

    static void buildArnaudFinder( carto::AttributedObject* parent, 
                                         Tree* t, 
                                         const std::string & filename );
  };

  //	inline

  inline ArnaudFinderParser::ArnaudFinderParser() : FinderParser()
  {
  }

}

#endif

