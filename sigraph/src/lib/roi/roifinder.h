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

#ifndef SI_ROI_ROIFINDER_H
#define SI_ROI_ROIFINDER_H

#include <si/finder/modelFinder.h>

namespace sigraph
{

  /**	ROI model selector
   */
  class RoiFinder : public ModelFinder
  {
  public:
    RoiFinder( MGraph & rg );
    virtual ~RoiFinder();

    virtual carto::AttributedObject* selectModel( const Clique* cl );
  };

}

#endif

