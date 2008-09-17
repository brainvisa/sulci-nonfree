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

#ifndef SI_SULCALSKETCH_SULCALSKETCHFINDER_H
#define SI_SULCALSKETCH_SULCALSKETCHFINDER_H

#include <si/finder/modelFinder.h>

namespace sigraph
{

  class SulcalSketchFinder : public ModelFinder
  {
  public:
    SulcalSketchFinder( MGraph & mg );
    virtual ~SulcalSketchFinder();

    virtual carto::AttributedObject* selectModel( const Clique* cl );
    virtual void initCliques( CGraph &data, bool verbose=true, 
                              bool withCache=false, bool translateLabels=true, 
                              bool checkLabels=true, 
                              const aims::SelectionSet *sel=0 );
  };

}

#endif


