
#ifndef SI_FOLD_FOLDLABELSCHANGER_H
#define SI_FOLD_FOLDLABELSCHANGER_H


#include <si/learner/labelsChanger.h>


namespace sigraph
{

  /**	Change des labels dans les cliques de sillons
   */
  class FoldLabelsChanger : public LabelsChanger
  {
  public:
    FoldLabelsChanger() : LabelsChanger( "fold_labels_changer" ) {}
    virtual ~FoldLabelsChanger() {}

    virtual double constrainedNoise( Clique* cl, double & outp, 
				     const std::set<std::string> & 
				     significantLabels, 
				     const std::string & voidLabel );

    ///	Distance de l'exemple changé à celui d'origine
    virtual double distance( Vertex* v, const std::string & oldlabel, 
			     const std::string & newlabel, 
			     CliqueCache* cc = 0 );
    ///	Distance de changement pour les relations
    virtual double edgeDist( Vertex* v, const std::string & oldlabel, 
			     const std::string & newlabel, CliqueCache* cc );
    ///	Conversion distance -> sortie d'apprentissage
    virtual double output( double outp, double dist );

  protected:
    FoldLabelsChanger( const std::string & syntax )
      : LabelsChanger( syntax ) {}
    ///	Bruitage pour les anciens FoldDescr
    double noiseFoldDescr( Clique* cl, double & outp, 
			   const std::set<std::string> & significantLabels, 
			   const std::string & voidLabel );
    ///	Bruitage pour les nouveaux FoldDescr2
    double noiseFoldDescr2( Clique* cl, double & outp, 
			    const std::set<std::string> & significantLabels, 
			    const std::string & voidLabel );
    ///	Bruitage pour les anciens InterFoldDescr
    double noiseIFDescr( Clique* cl, double & outp, 
			 const std::set<std::string> & significantLabels, 
			 const std::string & voidLabel );
    ///	Bruitage pour les nouveaux InterFoldDescr2
    double noiseIFDescr2( Clique* cl, double & outp, 
			  const std::set<std::string> & significantLabels, 
			  const std::string & voidLabel );

    ///	Dit si c'est à une relation qu'on s'intéresse
    bool		_edge;
    ///	Pointeur sur labels significatifs
    std::string	_label1;
    ///	Pointeur sur void label
    std::string	_label2;
    ///	ID de version de FoldDescr/InterfoldDescr utilisé
    std::string	_version;

  private:
  };

}

#endif

