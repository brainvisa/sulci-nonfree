
#ifndef SI_FOLD_FRGWRITER_H
#define SI_FOLD_FRGWRITER_H


#include <si/graph/mgWriter.h>
#include <aims/io/argW.h>


namespace sigraph
{

  /**	FrgWriter : writing random filds graphs
   */
  class FrgWriter : public MGWriter
  {
  public:
    FrgWriter( const std::string & filename );
    virtual ~FrgWriter();

  protected:

  private:

  };


  class LowLevelFRGArgWriter : public aims::LowLevelArgWriter
  {
    public:
      LowLevelFRGArgWriter();
      virtual ~LowLevelFRGArgWriter();
      virtual void write( const std::string & filename, Graph & graph,
                          SavingMode mode = Keep,
                          bool saveOnlyModified = true );
  };

}

#endif


