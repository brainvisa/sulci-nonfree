/* Copyright (c) 1995-2006 CEA
 *
 *  This software and supporting documentation were developed by
 *      CEA/DSV/SHFJ
 *      4 place du General Leclerc
 *      91401 Orsay cedex
 *      France
 *
 */

#ifndef SI_SUBADAPTIVE_SVMPARSER_H
#define SI_SUBADAPTIVE_SVMPARSER_H

#include <cartobase/object/attributed.h>
class Tree;

namespace sigraph
{
  class SubAdSvm;

  class SvmSAParser
  {
  public:
    static void buildSubSvm( carto::AttributedObject* parent, Tree* t, 
                             const std::string & filename );

  protected:
    static void parseSubSvm( carto::AttributedObject* parent, Tree* t, 
			     SubAdSvm & sad );
  };

}

#endif

