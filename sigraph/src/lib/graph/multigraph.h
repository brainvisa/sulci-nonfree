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

#ifndef SI_GRAPH_MULTIGRAPH_H
#define SI_GRAPH_MULTIGRAPH_H

#include <string>
#include <vector>

class Graph;

namespace sigraph
{
  void concatenateGraphs( const std::vector<Graph*> in, Graph & out, 
                          const std::string & subjectatt );
  void recoverIndividualGraph( const std::vector<Graph*> subjects, 
                               Graph & multi, 
                               const std::string & vertexatt );
}

#endif


