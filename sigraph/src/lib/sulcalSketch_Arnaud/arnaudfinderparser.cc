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

#include <si/sulcalSketch_Arnaud/arnaudfinderparser.h>
#include <si/sulcalSketch_Arnaud/arnaudfinder.h>
#include <si/sulcalSketch_Arnaud/arnaudattrib.h>

using namespace sigraph;
using namespace carto;
using namespace std;


ArnaudFinderParser::~ArnaudFinderParser()
{
}


TreePostParser::FactorySet ArnaudFinderParser::factories()
{
  FactorySet	fs;

  fs[ SIA_ARNAUD_FINDER_SYNTAX ] = buildArnaudFinder;

  return( fs );
}


#include <iostream>
void 
ArnaudFinderParser::buildArnaudFinder( AttributedObject* parent, 
                                                   Tree*, const string & )
{
  cout << "buildArnaudFinder\n";
  parent->setProperty( "model_finder_ptr", (ModelFinder *) 
                        new ArnaudFinder( (MGraph &) *parent) );
}


