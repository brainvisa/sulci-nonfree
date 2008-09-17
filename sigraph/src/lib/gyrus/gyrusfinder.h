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

#ifndef SI_GYRUS_GYRUSFINDER_H
#define SI_GYRUS_GYRUSFINDER_H

#include <si/finder/modelFinder.h>

namespace sigraph
{

  /**	Gyrus model selector
   */
  class GyrusFinder : public ModelFinder
  {
  public:
    GyrusFinder( MGraph & rg );
    virtual ~GyrusFinder();

    virtual carto::AttributedObject* selectModel( const Clique* cl );
  };

}

#endif

