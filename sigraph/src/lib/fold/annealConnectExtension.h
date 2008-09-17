
#ifndef SI_FOLD_ANNEALCONNECTEXTENSION_H
#define SI_FOLD_ANNEALCONNECTEXTENSION_H

#include <si/graph/annealExtension.h>


namespace sigraph
{

  class AnnealConnectExtension : public AnnealExtension
  {
  public:
    AnnealConnectExtension( Anneal* ann ) : AnnealExtension( ann ) {}
    virtual ~AnnealConnectExtension();
    virtual void specialStep( unsigned passnum = 0 );
    virtual std::string name() const { return( "   CONNECT" ); }

  protected:

  private:
  };

}

#endif

