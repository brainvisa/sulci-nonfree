/* Copyright (c) 1995-2006 CEA
 *
 *  This software and supporting documentation were developed by
 *      CEA/DSV/SHFJ
 *      4 place du General Leclerc
 *      91401 Orsay cedex
 *      France
 *
 */

#ifndef SI_PLUGIN_SISVMPLUGIN_H
#define SI_PLUGIN_SISVMPLUGIN_H

#include <cartobase/plugin/plugin.h>

namespace sigraph
{

  class SiSvmPlugin : public carto::Plugin
  {
  public:
    SiSvmPlugin();
    virtual ~SiSvmPlugin();
    virtual std::string name() const;
    static bool noop();
  };

}

#endif

