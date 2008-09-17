/*
 *  Copyright (C) 1998-2003 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 */

#include <si/descr/cliqueDescr.h>

using namespace sigraph;
using namespace carto;
using namespace std;


CliqueDescr::CliqueDescr()
{
}


CliqueDescr::~CliqueDescr()
{
}


vector<string> CliqueDescr::descriptorsNames() const
{
  vector<string>	names;
  return names;
}


string CliqueDescr::name() const
{
  return "Abstract_CliqueDescr";
}


bool CliqueDescr::makeVector( const Clique* cl, vector<double> & vec, 
                              GenericObject* model )
{
  return makeVectorElements( cl, vec, model );
}

#include <cartobase/object/object_d.h>
INSTANTIATE_GENERIC_OBJECT_TYPE( CliqueDescr * )

