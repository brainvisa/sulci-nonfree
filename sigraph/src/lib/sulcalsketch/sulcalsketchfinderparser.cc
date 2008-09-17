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

#include <si/sulcalsketch/sulcalsketchfinderparser.h>
#include <si/sulcalsketch/sulcalsketchfinder.h>
#include <si/sulcalsketch/sulcalsketchattrib.h>

using namespace sigraph;
using namespace carto;
using namespace std;


SulcalSketchFinderParser::~SulcalSketchFinderParser()
{
}


TreePostParser::FactorySet SulcalSketchFinderParser::factories()
{
  FactorySet	fs;

  fs[ SIA_SULCALSKETCH_FINDER_SYNTAX ] = buildSulcalSketchFinder;

  return( fs );
}


#include <iostream>
void 
SulcalSketchFinderParser::buildSulcalSketchFinder( AttributedObject* parent, 
                                                   Tree*, const string & )
{
  cout << "buildSulcalSketchFinder\n";
  parent->setProperty( "model_finder_ptr", (ModelFinder *) 
                        new SulcalSketchFinder( (MGraph &) *parent) );
}


