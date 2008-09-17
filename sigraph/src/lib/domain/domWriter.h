
#ifndef SI_DOMAIN_DOMWRITER_H
#define SI_DOMAIN_DOMWRITER_H


#include <si/domain/domain.h>
#include <si/graph/exoticTreeWriter.h>


namespace sigraph
{

  /**	Classe DomWriter : écriture de domaines de validité de noeud
   */
  class DomWriter : public ExoticTreeWriter
  {
  public:
    DomWriter( const std::string & filename, bool makedir = false );
    virtual ~DomWriter();

    virtual void write( const Domain & dom );

  protected:
    virtual void buildTree( Tree & tr, const Domain & dom );

  private:
  };


  inline DomWriter & operator << ( DomWriter & dw, const Domain & dom )
  { dw.write( dom ); return( dw ); }

}

#endif

