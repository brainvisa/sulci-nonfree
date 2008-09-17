
#ifndef SI_FOLD_DOMAINBOX2_H
#define SI_FOLD_DOMAINBOX2_H


#include <si/fold/domainBox.h>


namespace sigraph
{

class DomainBox2 : public DomainBox
{
public:
  DomainBox2();
  DomainBox2( const DomainBox2 & dom );
  virtual ~DomainBox2();
  virtual Domain* clone() const;
  virtual void learn( const Vertex* v, const Graph* g = 0 );
  virtual void buildTree( Tree & tr ) const;
  static void buildDomBox2( Tree* parent, Tree* tr );
};


//	inline

inline DomainBox2::DomainBox2() : DomainBox()
{
}


inline DomainBox2::DomainBox2( const DomainBox2 & dom ) : DomainBox( dom )
{
}


inline Domain* DomainBox2::clone() const
{
  return( new DomainBox2( *this ) );
}


inline DomainBox2::~DomainBox2()
{
}

}

#endif


