
#ifndef SI_LEARNER_NOISER_H
#define SI_LEARNER_NOISER_H


#include <si/learner/learner.h>


namespace sigraph
{

  /**     Bruiteur de clique
   */
  class Noiser : public Learner
  {
  public:
    virtual ~Noiser();

    virtual void process(LearnParam *lp);

  protected:
    Noiser( const std::string & syntax="noiser" );
    ///	Retourne la distance de changement si la clique a changé, 0 sinon
    virtual double noise( Clique* cl, double & outp ) = 0;

  private:
  };

}

#endif


