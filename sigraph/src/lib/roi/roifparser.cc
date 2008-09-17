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

#include <si/roi/roifparser.h>
#include <si/roi/roifinder.h>

using namespace sigraph;
using namespace carto;
using namespace std;


RoiFParser::~RoiFParser()
{
}


TreePostParser::FactorySet RoiFParser::factories()
{
  FactorySet	fs;

  fs[ "roi_finder" ] = buildRoiFinder;

  return( fs );
}


void RoiFParser::buildRoiFinder( AttributedObject* parent, Tree*, 
                                 const string & )
{
  parent->setProperty( "model_finder_ptr", 
			(ModelFinder *) new RoiFinder( (MGraph &) *parent) );
}


