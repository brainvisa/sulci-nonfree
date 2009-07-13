/*
 *  Copyright (C) 1997-2005 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 *  General variables and Group/Type returning string
 *
 */

#include <cstdlib>
#include <si/global/global.h>
#include <aims/def/path.h>
#include <si/fold/foldReader.h>
#include <si/fold/frgReader.h>
#include <si/global/siInfo.h>
#include <si/fold/domainBox2.h>
#include <si/fold/domainRbf.h>
#include <si/fold/inertialDomainBox.h>
#include <si/domain/domReader.h>
#include <si/fold/fattrib.h>
#include <cartobase/stream/fileutil.h>
#include <cartobase/stream/directory.h>
#include <cartobase/config/version.h>
#include <cartobase/plugin/plugin.h>

using namespace sigraph;
using namespace aims;
using namespace carto;
using namespace std;

namespace
{
  static bool __initSigraph()
  {
    //cout << "initSigraph\n";
    ArgReader::registerReader( SIA_GRAPH_SYNTAX, new LowLevelFoldArgReader );
    ArgReader::registerReader( SIA_MODEL_GRAPH_SYNTAX, 
			       new LowLevelFRGArgReader );

    DomReader::registerBuilder( SIA_DOMAIN_BOX, &DomainBox::buildDomBox );
    DomReader::registerBuilder( SIA_DOMAIN_BOX2, &DomainBox2::buildDomBox2 );
    DomReader::registerBuilder( SIA_INERTIAL_DOMAIN_BOX, 
				&InertialDomainBox::buildInertialDomBox );
    DomReader::registerBuilder( SIA_DOMAIN_RBF, &DomainRBF::buildDomRBF );

    new SiInfo;

    PluginLoader::pluginFiles().push_back
      ( PluginLoader::PluginFile( si().basePath() + FileUtil::separator() 
                                  + "plugins" + FileUtil::separator() 
                                  + "sigraph.plugins", 
                                  si().shortVersion() ) );
    /*cout << "plugins file: " 
         << si().basePath() + FileUtil::separator() + "plugins" 
      + FileUtil::separator() + "sigraph.plugins" 
         << endl;*/
    //PluginLoader::load();

    return true;
  }

  static bool _initSigraph = __initSigraph();
}

Si & sigraph::si()
{
  static Si *theSi = 0;

  if( !theSi ) theSi = new Si;
  return *theSi;
}


Si::Si() : _basePath( getBasePath() ), 
  _labelsTranslPath( Path::singleton().hierarchy() 
		     + FileUtil::separator() + "sulcal_root_colors.hie" )
{
}


Si::~Si()
{
}


string Si::getBasePath()
{
  const char	*base = getenv( "SIGRAPH_PATH" );
  string	shared;
  char		s = FileUtil::separator();
  Directory	d( "/" );

  if( base )
    {
      shared = string( base ) + s + "shared";
      d.chdir( shared );
      if( !d.isValid() )
        {
          shared = base;
        }
    }
  else
    {
      string	shfj = Path::singleton().globalShared();
      shared = shfj + s + "sigraph-" + shortVersion();
      d.chdir( shared );
      if( !d.isValid() )
        {
          shared = shfj + s + "sigraph-main";
          d.chdir( shared );
          if( !d.isValid() )
            shared = shfj + s + "sigraph";
        }
    }

  return shared;
}


const string & Si::version()
{
  static string	ver( carto::cartobaseVersionString() );
  return ver;
}


const string & Si::shortVersion()
{
  static string	ver( carto::cartobaseShortVersion() );
  return ver;
}






