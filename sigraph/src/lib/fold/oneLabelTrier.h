
#ifndef SI_LEARNER_ONELABELTRIER_H
#define SI_LEARNER_ONELABELTRIER_H


#include <si/fold/foldLabelsChanger.h>


namespace sigraph
{

  /**	Change des labels dans les cliques de sillons
   */
  class OneLabelTrier : public FoldLabelsChanger
  {
  public:
    OneLabelTrier() : FoldLabelsChanger( "one_label_trier" ) {}
    virtual ~OneLabelTrier() {}

    virtual double constrainedNoise( Clique* cl, double & outp, 
				     const std::set<std::string> 
				     & significantLabels, 
				     const std::string & voidLabel );
    virtual double constrainedNoiseOLT( Clique* cl, double & outp, 
					const std::set<std::string> 
					& significantLabels, 
					const std::string & voidLabel );

  protected:
    OneLabelTrier( const std::string & syntax )
      : FoldLabelsChanger( syntax ) {}

  private:
  };

}

#endif

