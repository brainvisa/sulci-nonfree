
#ifndef SI_SUBADAPTIVE_SUBADGAUSS_H
#define SI_SUBADAPTIVE_SUBADGAUSS_H

#include <si/subadaptive/incrementalSubAdaptive.h>
#include <si/subadaptive/subAdaptive.h>
#include <neur/gauss/gaussnet.h>


namespace sigraph
{

  ///	Enrobage d'un réseau de Gaussiennes
  class SubAdGauss : public IncrementalSubAdaptive
  {
  public:
    SubAdGauss( const std::string & name = "" );
    SubAdGauss( const SubAdGauss & g );
    virtual ~SubAdGauss();

    SubAdGauss & operator = ( const SubAdGauss & g );
    virtual SubAdaptive* clone() const;

    virtual void init();
    virtual void randinit();
    virtual double prop( const std::vector<double> & vec );
    virtual double learn(AdaptiveLeaf &al,
		const SiDBLearnable &train, const SiDBLearnable &test);
    virtual SubAdResponse *train(AdaptiveLeaf &al, const SiDBLearnable &train,
				const SiDBLearnable &test);
    virtual double learn(const GaussVectorLearnable &vl);
    virtual void buildTree( Tree & tr ) const;

    /**@name	Accès aux données */
    //@{
    ///	Accès au réseau (R/W)
    GaussNet & net() { return( _gnet ); }
    ///	Coefficient du gradient d'apprentissage : poids
    double etaW() const { return( _etaW ); }
    ///	Coefficient du gradient d'apprentissage : centres
    double etaC() const { return( _etaC ); }
    ///	Coefficient du gradient d'apprentissage : sigmas
    double etaS() const { return( _etaS ); }
    void setEtaW( double etaW ) { _etaW = etaW; }
    void setEtaC( double etaC ) { _etaC = etaC; }
    void setEtaS( double etaS ) { _etaS = etaS; }
    double defaultValue() const { return( _defVal ); }
    void setDefaultValue( double dv ) { _defVal = dv; }
    //@}

  protected:
    GaussNet	_gnet;
    ///	Facteur d'apprentissage des poids
    double	_etaW;
    ///	Facteur d'apprentissage des centres
    double	_etaC;
    ///	Facteur d'apprentissage des sigmas
    double	_etaS;
    ///	Valeur par défaut (à l'infini)
    double	_defVal;
  private:
  };


  //	inline

  inline SubAdGauss & SubAdGauss::operator = ( const SubAdGauss & g )
  {
    if( this != &g )
      {
	_gnet = g._gnet;
	_etaW = g._etaW;
	_etaC = g._etaC;
	_etaS = g._etaS;
	_defVal = g._defVal;
      }
    return( *this );
  }


  inline SubAdaptive* SubAdGauss::clone() const
  {
    return( new SubAdGauss( *this ) );
  }

}

#endif


