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

#ifndef SI_FOLD_FGRAPH_H
#define SI_FOLD_FGRAPH_H


#include <si/graph/cgraph.h>

namespace sigraph
{
  class MGraph;

  /**	Folds graph
   */
  class FGraph : public CGraph
  {
  public:
    FGraph( const std::string s = "" );
    virtual ~FGraph();

    virtual void flipHemispheres();
  };

}


#endif


