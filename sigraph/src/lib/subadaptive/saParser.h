/*
 *  Copyright (C) 1998-2005 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 */


#ifndef SI_SUBADAPTIVE_SAPARSER_H
#define SI_SUBADAPTIVE_SAPARSER_H


#include <si/graph/treeParser.h>
#include <string>


namespace sigraph
{
  class SubAdaptive;
  class SubAdMlp;

  class SAParser : public TreePostParser
  {
  public:
    SAParser();
    virtual ~SAParser();

    static FactorySet & sharedFactories();
    virtual FactorySet factories();

    static void buildSubMlp( carto::AttributedObject* parent, Tree* t, 
			     const std::string & filename );
    static void buildSubGauss( carto::AttributedObject* parent, Tree* t, 
			       const std::string & filename );
    static void buildSubMixGauss( carto::AttributedObject* parent, Tree* t, 
			       const std::string & filename );
    static void buildMatriceList( carto::AttributedObject* parent, Tree* t, 
			       const std::string & filename );
    static void parseSubAd( carto::AttributedObject* parent, Tree* t, 
			    SubAdaptive & sad );

  protected:
    static void parseSubMlp( carto::AttributedObject* parent, Tree* t, 
			     SubAdMlp & sad );
  };

}

#endif


