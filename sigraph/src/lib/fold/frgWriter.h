
#ifndef SI_FOLD_FRGWRITER_H
#define SI_FOLD_FRGWRITER_H


#include <si/graph/mgWriter.h>


namespace sigraph
{

  /**	Classe FrgWriter : �criture de graphes al�atoires de sillons
   */
  class FrgWriter : public MGWriter
  {
  public:
    FrgWriter( const std::string & filename );
    virtual ~FrgWriter();

  protected:

  private:

  };

}

#endif


