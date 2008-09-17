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

#ifndef SI_FUNCTIONALSKETCH_FUNCTIONALSKETCHFINDERPARSER_H
#define SI_FUNCTIONALSKETCH_FUNCTIONALSKETCHFINDERPARSER_H


#include <si/finder/finderParser.h>


namespace sigraph
{

  class FunctionalSketchFinderParser : public FinderParser
  {
  public:
    FunctionalSketchFinderParser();
    virtual ~FunctionalSketchFinderParser();

    virtual FactorySet factories();

    static void buildFunctionalSketchFinder( carto::AttributedObject* parent, 
                                         Tree* t, 
                                         const std::string & filename );
  };

  //	inline

  inline FunctionalSketchFinderParser::FunctionalSketchFinderParser() : FinderParser()
  {
  }

}

#endif

