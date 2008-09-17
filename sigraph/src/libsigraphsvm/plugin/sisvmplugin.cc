/* Copyright (c) 1995-2006 CEA
 *
 *  This software and supporting documentation were developed by
 *      CEA/DSV/SHFJ
 *      4 place du General Leclerc
 *      91401 Orsay cedex
 *      France
 *
 */

#include <si/plugin/sisvmplugin.h>
#include <si/subadaptive/saParser.h>
#include <si/subadaptive/svmparser.h>

using namespace sigraph;
using namespace carto;
using namespace std;

namespace
{

  bool initSiSvm()
  {
    new SiSvmPlugin;
    return true;
  }

  bool _svminit = initSiSvm();

}


SiSvmPlugin::SiSvmPlugin() : Plugin()
{
  SAParser::sharedFactories()[ "sub_ad_svm" ] = &SvmSAParser::buildSubSvm;
}


SiSvmPlugin::~SiSvmPlugin()
{
}


string SiSvmPlugin::name() const
{
  return string( "Sigraph SVM support" );
}


bool SiSvmPlugin::noop()
{
  return true;
}

