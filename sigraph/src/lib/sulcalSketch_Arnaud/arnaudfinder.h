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
 
 
#ifndef SI_ARNAUD_ARNAUDFINDER_H
#define SI_ARNAUD_ARNAUDFINDER_H

#include <si/finder/modelFinder.h>

namespace sigraph
{

  class ArnaudFinder : public ModelFinder
  {
  public:
    ArnaudFinder( MGraph & mg );
    virtual ~ArnaudFinder();

    virtual carto::AttributedObject* selectModel( const Clique* cl );
    virtual void initCliques( CGraph &data, bool verbose=true, 
                              bool withCache=false, bool translateLabels=true, 
                              bool checkLabels=true, 
                              const aims::SelectionSet *sel=0 );
  };

}

#endif
