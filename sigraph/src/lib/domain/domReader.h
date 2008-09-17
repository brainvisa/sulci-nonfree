
#ifndef SI_DOMAIN_DOMREADER_H
#define SI_DOMAIN_DOMREADER_H


#include <si/domain/domain.h>
#include <si/graph/exoticTreeReader.h>


namespace sigraph
{

  /**	Lecteur de domaine de validité (Domain Reader)
   */
  class DomReader : public ExoticTreeReader
  {
  public:
    typedef void (*BuildFunc)( Tree* parent, Tree* tr );

    DomReader( const std::string & filename );
    virtual ~DomReader();
    virtual Domain* readDom();
    static void registerBuilder( const std::string syntax, BuildFunc builder )
    { builders()[ syntax ] = builder; }

    static carto::SyntaxSet & syntax();

  protected:
    virtual void parse( Tree* ao );

    static void buildNull( Tree* parent, Tree* ao );
    static std::map<std::string, BuildFunc> & builders();

  private:
  };

}

#endif

