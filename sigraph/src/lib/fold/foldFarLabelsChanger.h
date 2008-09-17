
#ifndef SI_FOLD_FOLDFARLABELSCHANGER_H
#define SI_FOLD_FOLDFARLABELSCHANGER_H


#include <si/fold/foldLabelsChanger.h>


namespace sigraph
{

  /**	Change des labels dans les cliques de sillons, prend des exemples loin
	des bons
  */
  class FoldFarLabelsChanger : public FoldLabelsChanger
  {
  public:
    FoldFarLabelsChanger() : FoldLabelsChanger( "fold_far_labels_changer" )
    {}
    virtual ~FoldFarLabelsChanger() {}

    /**	Tirer un nombre entre 0 et n inclus */
    virtual unsigned randomGen( unsigned n );
    virtual double constrainedNoise( Clique* cl, double & outp, 
				     const std::set<std::string> & 
				     significantLabels, 
				     const std::string & voidLabel );

  protected:
    FoldFarLabelsChanger( const std::string & syntax )
      : FoldLabelsChanger( syntax )
    {}

  private:
  };

}

#endif

