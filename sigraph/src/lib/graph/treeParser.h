/*
 *  Copyright (C) 1998-2004 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 */

#ifndef SI_GRAPH_TREEPARSER_H
#define SI_GRAPH_TREEPARSER_H


#include <cartobase/object/attributed.h>

class Tree;


namespace sigraph
{

  ///	Scans a tree and do something to each element
  class TreePostParser
  {
  public:
    ///	Elements readers (depending on syntax)
    typedef void (* Factory )( carto::AttributedObject* parent, Tree* t, 
			       const std::string & filename );
    ///	syntax -> reader
    typedef std::map<std::string, Factory>	FactorySet;

    virtual ~TreePostParser();

    virtual FactorySet factories() = 0;

  protected:
    TreePostParser() {}

  private:
  };

}

#endif


