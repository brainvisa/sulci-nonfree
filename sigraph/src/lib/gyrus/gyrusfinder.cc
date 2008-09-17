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

#include <si/gyrus/gyrusfinder.h>
#include <iostream>

using namespace sigraph;
using namespace carto;
using namespace std;


GyrusFinder::GyrusFinder( MGraph & rg ) : ModelFinder( rg )
{
}


GyrusFinder::~GyrusFinder()
{
}


AttributedObject* GyrusFinder::selectModel( const Clique* )
{
  AttributedObject	*mod = 0;

  if( _mgraph.order() != 1 )
    cerr << "GyrusFinder::selectModel : the model has " << _mgraph.order() 
         << " vertices, it should have 1" << endl;
  else
    mod = *_mgraph.begin();

  return( mod );
}


