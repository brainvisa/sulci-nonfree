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

#include <si/functionalsketch/surfacebasedfunctionalsketchfinderparser.h>
#include <si/functionalsketch/surfacebasedfunctionalsketchfinder.h>
#include <si/functionalsketch/surfacebasedfunctionalsketchattrib.h>

using namespace sigraph;
using namespace carto;
using namespace std;


SurfaceBasedFunctionalSketchFinderParser::~SurfaceBasedFunctionalSketchFinderParser()
{
}


TreePostParser::FactorySet SurfaceBasedFunctionalSketchFinderParser::factories()
{
  FactorySet	fs;

  fs[ SIA_SURFACEBASEDFUNCTIONALSKETCH_FINDER_SYNTAX ] = buildSurfaceBasedFunctionalSketchFinder;

  return( fs );
}


#include <iostream>
void 
SurfaceBasedFunctionalSketchFinderParser::buildSurfaceBasedFunctionalSketchFinder( AttributedObject* parent, 
                                                   Tree*, const string & )
{
  cout << "buildSurfaceBasedFunctionalSketchFinder\n";
  parent->setProperty( "model_finder_ptr", (ModelFinder *) 
                        new SurfaceBasedFunctionalSketchFinder( (MGraph &) *parent) );
}


