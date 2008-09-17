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

#ifndef SI_FUNCTIONALSKETCH_SURFACEBASEDFUNCTIONALSKETCHFINDERPARSER_H
#define SI_FUNCTIONALSKETCH_SURFACEBASEDFUNCTIONALSKETCHFINDERPARSER_H


#include <si/finder/finderParser.h>


namespace sigraph
{

  class SurfaceBasedFunctionalSketchFinderParser : public FinderParser
  {
  public:
    SurfaceBasedFunctionalSketchFinderParser();
    virtual ~SurfaceBasedFunctionalSketchFinderParser();

    virtual FactorySet factories();

    static void buildSurfaceBasedFunctionalSketchFinder( carto::AttributedObject* parent, 
                                         Tree* t, 
                                         const std::string & filename );
  };

  //	inline

  inline SurfaceBasedFunctionalSketchFinderParser::SurfaceBasedFunctionalSketchFinderParser() : FinderParser()
  {
  }

}

#endif

