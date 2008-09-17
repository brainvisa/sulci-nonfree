
#ifndef SI_FOLD_FOLDNOISER_H
#define SI_FOLD_FOLDNOISER_H


#include <si/learner/noiser.h>


namespace sigraph
{

  /**	Bruiteur de sillon
   */
  class FoldNoiser : public Noiser
  {
  public:
    FoldNoiser();
    virtual ~FoldNoiser();

    virtual double noise( Clique* cl, double & outp );

  protected:
    FoldNoiser( const std::string & syntax ) : Noiser( syntax ) {}

  private:
  };

}

#endif

