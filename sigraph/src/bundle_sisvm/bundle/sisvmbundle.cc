/*
 *  Copyright (C) 2006 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 */

#include <si/plugin/sisvmplugin.h>

using namespace sigraph;

namespace
{
  // force using libsigraphsvm.dylib on Mac
  bool __sisvmplugin = SiSvmPlugin::noop();
}


