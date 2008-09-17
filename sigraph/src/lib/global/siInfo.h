
#ifndef SI_GLOBAL_INFO_H
#define SI_GLOBAL_INFO_H

#include <cartobase/config/info.h>

namespace sigraph
{

  class SiInfo : public carto::Info
  {
  public:
    SiInfo();
    virtual ~SiInfo();
    virtual void printPaths( std::ostream &output=std::cout );
    virtual void printVersion( std::ostream &output=std::cout );
  };

}

#endif

