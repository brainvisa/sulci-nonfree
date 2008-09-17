
#ifndef SI_DOMAIN_NULLDOMAIN_H
#define SI_DOMAIN_NULLDOMAIN_H


#include <si/domain/domain.h>


namespace sigraph
{

  ///	Classe "nulle part" ...
  class NullDomain : public Domain
  {
  public:
    NullDomain();
    NullDomain( const NullDomain & d );
    virtual ~NullDomain();

    virtual Domain* clone() const;

    virtual void buildTree( Tree & tr ) const;

    bool canBeFound( double x, double y, double z );
    bool canBeFound( const Vertex* v, const Graph* g = 0 );
  };

  //	inline

  inline NullDomain::NullDomain() : Domain()
  {
  }


  inline NullDomain::NullDomain( const NullDomain & d ) : Domain( d )
  {
  }


  inline Domain* NullDomain::clone() const
  {
    return( new NullDomain( *this ) );
  }


  inline bool NullDomain::canBeFound( double, double, double )
  {
    return( false );
  }


  inline bool NullDomain::canBeFound( const Vertex*, const Graph* )
  {
    return( false );
  }

}

#endif


