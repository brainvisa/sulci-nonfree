
#ifndef SI_FOLD_FOLDREDUCEDLABELSCHANGER_H
#define SI_FOLD_FOLDREDUCEDLABELSCHANGER_H


#include <si/fold/foldLabelsChanger.h>


namespace sigraph
{

  /**	Change des labels dans les cliques de sillons, parmi les noeuds 
	voisins des noeuds significatifs
  */
  class FoldReducedLabelsChanger : public FoldLabelsChanger
  {
  public:
    FoldReducedLabelsChanger() 
      : FoldLabelsChanger( "fold_reduced_labels_changer" ) {}
    virtual ~FoldReducedLabelsChanger() {}

    ///	Enlève les noeuds en trop avant de bruiter
    virtual double constrainedNoise( Clique* cl, double & outp, 
				     const std::set<std::string> 
				     & significantLabels, 
				     const std::string & voidLabel );

  protected:
    FoldReducedLabelsChanger( const std::string & syntax ) 
      : FoldLabelsChanger( syntax ) {}
  };

}

#endif



