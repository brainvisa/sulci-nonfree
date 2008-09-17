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

#include <si/gyrus/gfparser.h>
#include <si/gyrus/gyrusfinder.h>

using namespace sigraph;
using namespace carto;
using namespace std;


GFParser::~GFParser()
{
}


TreePostParser::FactorySet GFParser::factories()
{
  FactorySet	fs;

  fs[ "gyrus_finder" ] = buildGyrusFinder;

  return( fs );
}


void GFParser::buildGyrusFinder( AttributedObject* parent, Tree*, 
                                 const string & )
{
  parent->setProperty( "model_finder_ptr", 
			(ModelFinder *) new GyrusFinder( (MGraph &) *parent) );
}


