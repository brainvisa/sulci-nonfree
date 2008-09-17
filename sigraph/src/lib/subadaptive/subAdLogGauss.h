
#ifndef SI_SUBADAPTIVE_SUBADLOGGAUSS_H
#define SI_SUBADAPTIVE_SUBADLOGGAUSS_H

#include <si/subadaptive/subAdaptive.h>
#include <si/subadaptive/subAdGauss.h>
#include <neur/gauss/gaussnet.h>


namespace sigraph
{

  ///	Enrobage d'un réseau de Gaussiennes
  class SubAdLogGauss : public SubAdGauss
  {
  public:
    SubAdLogGauss( const std::string & name = "" );
    SubAdLogGauss( const SubAdLogGauss & g );
    virtual ~SubAdLogGauss();

    SubAdLogGauss & operator = ( const SubAdLogGauss & g );
    virtual SubAdaptive* clone() const;

    virtual double prop( const std::vector<double> & vec );
    virtual double learn(const GaussVectorLearnable &vl);

    virtual void buildTree( Tree & tr ) const;
  };


  //	inline

  inline SubAdLogGauss & SubAdLogGauss::operator = ( const SubAdLogGauss & g )
  {
    if( this != &g )
	SubAdGauss::operator = ( g );
    return( *this );
  }


  inline SubAdaptive* SubAdLogGauss::clone() const
  {
    return( new SubAdLogGauss( *this ) );
  }

}

#endif


