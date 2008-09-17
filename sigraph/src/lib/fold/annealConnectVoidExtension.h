
#ifndef SI_FOLD_ANNEALCONNECTVOIDEXTENSION_H
#define SI_FOLD_ANNEALCONNECTVOIDEXTENSION_H

#include <si/graph/annealExtension.h>


namespace sigraph
{

  class AnnealConnectVoidExtension : public AnnealExtension
  {
  public:
    AnnealConnectVoidExtension( Anneal* ann ) : AnnealExtension( ann ) {}
    virtual ~AnnealConnectVoidExtension();
    virtual void specialStep( unsigned passnum = 0 );
    virtual std::string name() const { return( " CONN_VOID" ); }

  protected:

  private:
  };

}

#endif

