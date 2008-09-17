/*
 *  Copyright (C) 2004 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 */

#ifndef SI_SULCALSKETCH_SULCALSKETCHFINDERPARSER_H
#define SI_SULCALSKETCH_SULCALSKETCHFINDERPARSER_H


#include <si/finder/finderParser.h>


namespace sigraph
{

  class SulcalSketchFinderParser : public FinderParser
  {
  public:
    SulcalSketchFinderParser();
    virtual ~SulcalSketchFinderParser();

    virtual FactorySet factories();

    static void buildSulcalSketchFinder( carto::AttributedObject* parent, 
                                         Tree* t, 
                                         const std::string & filename );
  };

  //	inline

  inline SulcalSketchFinderParser::SulcalSketchFinderParser() : FinderParser()
  {
  }

}

#endif

