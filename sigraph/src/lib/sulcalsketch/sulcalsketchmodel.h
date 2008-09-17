/*
 *  Copyright (C) 2004 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 */


#ifndef SI_SULCALSKETCH_SULCALSKETCHMODEL_H
#define SI_SULCALSKETCH_SULCALSKETCHMODEL_H

#include <si/model/model.h>


namespace sigraph
{

  class SulcalSketchSimilarityModel : public Model
  {
  public:
    SulcalSketchSimilarityModel( float distthresh, float distslope = 1, 
                                 float distweight = 1, float sclslope = 1, 
                                 float sclweight = 1 );
    virtual ~SulcalSketchSimilarityModel();
    virtual Model* clone() const;
    SulcalSketchSimilarityModel & operator = 
    ( const SulcalSketchSimilarityModel & );

    virtual double prop( const Clique* cl );
    virtual void buildTree( Tree & tr ) const;

    static void buildSimilarity( carto::AttributedObject* parent, Tree* ao, 
                                 const std::string & filename );

  private:
    float	_distthresh;
    float	_distslope;
    float	_distweight;
    float	_sclslope;
    float	_sclweight;
  };


  class SulcalSketchDataDrivenModel : public Model
  {
  public:
    SulcalSketchDataDrivenModel( float weight = 1 );
    virtual ~SulcalSketchDataDrivenModel();
    virtual Model* clone() const;
    SulcalSketchDataDrivenModel & operator = 
    ( const SulcalSketchDataDrivenModel & );

    virtual double prop( const Clique* cl );
    virtual void buildTree( Tree & tr ) const;

    static void buildDataDriven( carto::AttributedObject* parent, Tree* ao, 
                                 const std::string & filename );

  private:
    float	_weight;
  };

}

#endif


