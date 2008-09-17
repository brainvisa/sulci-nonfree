
#ifndef SI_FOLD_FOLDWRITER_H
#define SI_FOLD_FOLDWRITER_H


#include <graph/graph/gwriter.h>


namespace sigraph
{

  /**	Classe FoldWriter : écriture de graphes de sillons
   */
  class FoldWriter : public GraphWriter
  {
  public:
    FoldWriter( const std::string & filename );
    virtual ~FoldWriter();
    virtual void write( Graph & ao );
    virtual void write( const Graph & ao );

  protected:
    /*virtual void write( const GraphObject & ao );
    virtual void write( const Vertex & ao );
    virtual void write( const Edge & ao );*/

    std::string	_filenameBase;

  private:

  };


  //	inline


#if 0
  inline void FoldWriter::write( const Vertex & ao )
  {
    GraphWriter::write( ao );
    write( (GraphObject &) ao );
  }


  inline void FoldWriter::write( const Edge & ao )
  {
    GraphWriter::write( ao );
    write( (GraphObject &) ao );
  }
#endif

}

#endif



