
#ifndef SI_FOLD_FDPARSER_H
#define SI_FOLD_FDPARSER_H


#include <si/descr/descrParser.h>


namespace sigraph
{

  class FDParser : public DescrParser
  {
  public:
    FDParser();
    virtual ~FDParser();

    virtual FactorySet factories();
    virtual void registerFactory( const std::string & name, 
				  const Factory & fac );

    static void buildFDescr( carto::AttributedObject* parent, Tree* t, 
			     const std::string & filename );
    static void buildFDescr2( carto::AttributedObject* parent, Tree* t, 
			      const std::string & filename );
    static void buildFDescr3( carto::AttributedObject* parent, Tree* t, 
			      const std::string & filename );
    static void buildFDescr4( carto::AttributedObject* parent, Tree* t, 
			      const std::string & filename );
    static void buildFDescr5( carto::AttributedObject* parent, Tree* t, 
			      const std::string & filename );
    static void buildIFDescr( carto::AttributedObject* parent, Tree* t, 
			      const std::string & filename );
    static void buildIFDescr2( carto::AttributedObject* parent, Tree* t, 
			       const std::string & filename );
    static void buildIFDescr4( carto::AttributedObject* parent, Tree* t, 
			       const std::string & filename );
    static void buildIFDescr5( carto::AttributedObject* parent, Tree* t, 
			       const std::string & filename );
    static void buildBJDescr( carto::AttributedObject* parent, Tree* t, 
			      const std::string & filename );
    static void buildGyrusDescr( carto::AttributedObject* parent, Tree* t, 
                                 const std::string & filename );
    static void buildRoiDescr( carto::AttributedObject* parent, Tree* t, 
                               const std::string & filename );

  protected:
    FactorySet	_factories;

  private:
  };

  //	inline

  inline FDParser::~FDParser()
  {
  }

}

#endif


