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

#include <si/functionalsketch/functionalsketchfinderparser.h>
#include <si/functionalsketch/functionalsketchfinder.h>
#include <si/functionalsketch/functionalsketchattrib.h>

using namespace sigraph;
using namespace carto;
using namespace std;


FunctionalSketchFinderParser::~FunctionalSketchFinderParser()
{
}


TreePostParser::FactorySet FunctionalSketchFinderParser::factories()
{
  FactorySet	fs;

  fs[ SIA_FUNCTIONALSKETCH_FINDER_SYNTAX ] = buildFunctionalSketchFinder;

  return( fs );
}


#include <iostream>
void 
FunctionalSketchFinderParser::buildFunctionalSketchFinder( AttributedObject* parent, 
                                                   Tree*, const string & )
{
  cout << "buildFunctionalSketchFinder\n";
  parent->setProperty( "model_finder_ptr", (ModelFinder *) 
                        new FunctionalSketchFinder( (MGraph &) *parent) );
}


