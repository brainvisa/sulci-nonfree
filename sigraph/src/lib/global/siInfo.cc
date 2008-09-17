
#include <si/global/siInfo.h>
#include <si/global/global.h>

using namespace carto;
using namespace sigraph;
using namespace std;


SiInfo::SiInfo() : Info()
{
}


SiInfo::~SiInfo()
{
}


void SiInfo::printVersion( ostream &out )
{
  out << "SiGraph version : " << si().version() << endl;
}


void SiInfo::printPaths( ostream &out )
{
  out << "sigraph base path       : " << si().basePath() << endl;
  out << "labels translation path : " << si().labelsTranslPath() << endl;
}


