/*
 *  Copyright (C) 1998-2004 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 *  Slices of a mask to buckets
 *
 */

#include <si/descr/descrParser.h>
#include <si/model/adaptiveLeaf.h>
#include <graph/tree/tree.h>
#include <iostream>
#include <assert.h>

using namespace sigraph;
using namespace carto;
using namespace std;


void DescrParser::parseDescr( AttributedObject* parent, Tree*, 
			      CliqueDescr* cd )
{
  if( parent->getSyntax() != "ad_leaf" )
    {
      cerr << "CliqueDescr not child of AdLeaf !\n";
    }
  else
    {
      Model* mod;
      if( parent->getProperty( "pointer", mod ) )
	{
	  AdaptiveLeaf	*adl = dynamic_cast<AdaptiveLeaf *>( mod );
	  assert( adl );
	  adl->setCliqueDescr( cd );
	}
      else
	cerr << "CliqueDecr parent has no pointer !\n";
    }
}


