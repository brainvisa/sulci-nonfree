

#ifndef SI_FOLD_FOLDREADER_H
#define SI_FOLD_FOLDREADER_H


#include <si/fold/fgraph.h>
#include <si/graph/exoticGraphReader.h>
#include <aims/io/argR.h>


namespace sigraph
{

  ///	Folds graph reader
  class FoldReader : public ExoticGraphReader
  {
  public:
    FoldReader( const std::string & filename );
    virtual ~FoldReader();

    virtual void read( FGraph & gr, long subobj );

    static carto::SyntaxSet	syntax;

  protected:
    virtual void read( FGraph & gr ) { read( gr, -1 ); }
    virtual void parse( Graph & sg, carto::AttributedObject* go );
    virtual void computeGravCenter( Graph & sg, carto::AttributedObject* go, 
				    const std::string & attrib );

  private:
  };


  class LowLevelFoldArgReader : public aims::LowLevelArgReader
  {
  public:
    LowLevelFoldArgReader();
    virtual ~LowLevelFoldArgReader();
    virtual Graph* read( const std::string & filename, int subobj = -1 );
  };

}

#endif

