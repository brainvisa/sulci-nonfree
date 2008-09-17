/*
 *  Copyright (C) 2003 CEA - LSIS
 *
 *  This software and supporting documentation were developed by
 *
 *  	Olivier Coulon
 *  	Laboratoire LSIS,Groupe LXAO
 *  	ESIL, campus de Luminy, Case 925,
 *  	13288 Marseille Cedex 29, France
 *
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 */
 
 
#ifndef SI_FUNCTIONALSKETCH_SURFACEBASEDFUNCTIONALSKETCHFINDER_H
#define SI_FUNCTIONALSKETCH_SURFACEBASEDFUNCTIONALSKETCHFINDER_H

#include <si/finder/modelFinder.h>
#include <graph/tree/tree.h>


namespace sigraph
{
//   float meshDistance_adapt( AimsSurfaceTriangle &mesh, uint dep, uint ind ) ;

  class SurfaceBasedFunctionalSketchFinder : public ModelFinder
  {
  public:
    SurfaceBasedFunctionalSketchFinder( MGraph & mg );
    virtual ~SurfaceBasedFunctionalSketchFinder();

    virtual carto::AttributedObject* selectModel( const Clique* cl );
    virtual void initCliques( CGraph &data, bool verbose=true, 
                              bool withCache=false, bool translateLabels=true, 
                              bool checkLabels=true, 
                              const aims::SelectionSet *sel=0 );
  };

}

#endif
