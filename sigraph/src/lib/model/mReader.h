
#ifndef SI_MODEL_MREADER_H
#define SI_MODEL_MREADER_H


#include <cartobase/object/pythonreader.h>
#include <si/model/adaptive.h>
#include <si/subadaptive/subAdMlp.h>
#include <si/graph/exoticTreeReader.h>
#include <si/graph/treeParser.h>


namespace sigraph
{

  class MReader : public ExoticTreeReader
  {
  public:
    MReader( const std::string & filename, 
	     const TreePostParser::FactorySet & fs = baseFactorySet(), 
	     const carto::SyntaxSet & synt = syntax(),
	     const carto::AttributedReader::HelperSet& helpers_ = helpers());
    MReader( const TreePostParser::FactorySet & fs = baseFactorySet(), 
	     const carto::SyntaxSet & synt = syntax(),
	     const carto::AttributedReader::HelperSet& helpers_ = helpers());
    virtual ~MReader();

    virtual Model* readModel();
    void addFactory( const std::string & synt, TreePostParser::Factory fact );
    void addFactories( const TreePostParser::FactorySet & fs );
    void setFactories( const TreePostParser::FactorySet & fs );
    static TreePostParser::FactorySet baseFactorySet();
    /// replace factory dest by source (like dest=source)
    void aliasFactory( const std::string & dest, const std::string & source );
    const TreePostParser::FactorySet & factories();

    static carto::AttributedReader::HelperSet *initHelpers();
    static carto::PythonReader::HelperSet *initPythonHelpers();

    static carto::PythonReader::HelperSet & python_helpers();
    static carto::SyntaxSet & syntax();
    static carto::AttributedReader::HelperSet & helpers();
    static void delete_helpers();

  protected:
    virtual void parse( Tree* ao );
    static void buildTop( carto::AttributedObject* parent, Tree* ao, 
			  const std::string & filename );
    static void buildTree( carto::AttributedObject* parent, Tree* ao, 
			   const std::string & filename );
    static void buildLeaf( carto::AttributedObject* parent, Tree* ao, 
			   const std::string & filename );
    static void buildNull( carto::AttributedObject* parent, Tree* ao, 
			   const std::string & filename );
    static void buildConst( carto::AttributedObject* parent, Tree* ao, 
			    const std::string & filename );
    static void parseModel( Model* mod, carto::AttributedObject* parent, 
			    Tree* ao );
    static carto::PythonReader::HelperSet *& python_helpers_p();
    static carto::AttributedReader::HelperSet *& helpers_p();
    static carto::SyntaxSet *& syntax_p();

    TreePostParser::FactorySet	_knownElems;
    bool			_isOpen;

  private:
  };

}

#endif


