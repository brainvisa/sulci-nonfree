
#ifndef SI_MODEL_MWRITER_H
#define SI_MODEL_MWRITER_H


#include <si/model/model.h>
#include <si/graph/exoticTreeWriter.h>


namespace sigraph
{

  /**	Classe MWriter : écriture d'arbres d'éléments modèles
   */
  class MWriter : public ExoticTreeWriter
  {
  public:
    typedef bool (*Parser)( MWriter &, const carto::AttributedObject* );
    typedef std::set<Parser> ParserSet;

    MWriter( const std::string & filename, bool makedir = false );
    virtual ~MWriter();

    virtual void write( const Model & ad );

    static void registerParser( Parser p );
    static void unregisterParser( Parser p );
    static ParserSet & parsers();

  protected:
    virtual void parse( const carto::AttributedObject* ao );
    virtual void buildTree( Tree & tr, const Model & m );

  private:
  };


  inline MWriter & operator << ( MWriter & mw, const Model & m )
  { mw.write( m ); return( mw ); }

}

#endif

