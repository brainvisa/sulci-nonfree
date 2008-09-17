

#ifndef SI_FOLD_FRGREADER_H
#define SI_FOLD_FRGREADER_H


#include <si/fold/frgraph.h>
#include <si/model/mReader.h>
#include <si/graph/exoticGraphReader.h>
#include <si/finder/finderParser.h>
#include <aims/io/argR.h>


namespace sigraph
{

  /**	Classe FrgReader : lecteur de graphes aléatoires de sillons
   */
  class FrgReader : public ExoticGraphReader
  {
  public:
    FrgReader( const std::string & filename, 
	       MReader & mr = defaultMReader(), 
	       const TreePostParser::FactorySet & fs = defaultFFactories() );
    FrgReader( MReader & mr = defaultMReader(), 
	       const TreePostParser::FactorySet & fs = defaultFFactories() );
    virtual ~FrgReader();

    static carto::SyntaxSet & syntax();
    static MReader& defaultMReader();
    static TreePostParser::FactorySet defaultFFactories();

  protected:
    virtual void read( FRGraph & gr );
    virtual void parse( Graph & sg, carto::AttributedObject* go );
    virtual void parse( Graph & sg, Graph* rg );
    virtual void readModel( Graph & sg, carto::AttributedObject* go, 
			    const std::string & attrib, 
			    const std::string & filename );
    virtual void readDomain( Graph & sg, carto::AttributedObject* go, 
			     const std::string & attrib, 
			     const std::string & filename );
    static void readFakeRel( carto::AttributedObject* parent, Tree* t, 
			     const std::string & filename );

  private:
    MReader			*_mreader;
    TreePostParser::FactorySet	_ffact;
  };


  class LowLevelFRGArgReader : public aims::LowLevelArgReader
  {
  public:
    LowLevelFRGArgReader();
    virtual ~LowLevelFRGArgReader();
    virtual Graph* read( const std::string & filename, int subobj = -1 );
  };

}


#endif

