
#ifndef SI_GRAPH_MGWRITER_H
#define SI_GRAPH_MGWRITER_H


#include <graph/graph/gwriter.h>
#include <graph/graph/graph.h>
#include <cartobase/stream/fileutil.h>


namespace sigraph
{

  class MGWriter : public GraphWriter
  {
  public:
    MGWriter( const std::string & filename, const carto::SyntaxSet & synt );
    virtual ~MGWriter();
    virtual void parseModel( const GraphObject & ao, 
			     const std::string & mfile = "model_file", 
			     const std::string & mstr = "model" );
    virtual void write( const Graph & ao );
    virtual std::string name() const;

  protected:
    virtual void write( const GraphObject & ao );
    virtual void write( const Vertex & ao );
    virtual void write( const Edge & ao );

    std::string	_filenameBase;

  private:
    std::string	_mgFilename;
  };


  //	inline


  inline void MGWriter::write( const Vertex & ao )
  {
    GraphWriter::write( ao );
    write( (GraphObject &) ao );
  }


  inline void MGWriter::write( const Edge & ao )
  {
    GraphWriter::write( ao );
    write( (GraphObject &) ao );
  }


  inline void MGWriter::write( const Graph & ao )
  {
    if( !ao.getProperty( "filename_base", _filenameBase ) )
      _filenameBase = "";
    if( _filenameBase == "*" )
    {
      _filenameBase = carto::FileUtil::basename( name() );
      if( _filenameBase.substr( _filenameBase.length() - 4, 4 ) == ".arg" )
        _filenameBase = _filenameBase.substr( 0, _filenameBase.length() - 4 );
      _filenameBase += ".data";
    }
    if( !is_open() )
      open( name() );

    GraphWriter::write( ao );
    write( (GraphObject &) ao );
  }


  inline std::string MGWriter::name() const
  {
    return( _mgFilename );
  }

}

#endif


