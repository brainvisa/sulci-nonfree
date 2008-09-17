/*
 *  Copyright (C) 2004 CEA
 *
 *  This software and supporting documentation were developed by
 *  	Olivier Coulon
 *  	Laboratoire LSIS,Groupe LXAO
 *  	ESIL, campus de Luminy, Case 925,
 *  	13288 Marseille Cedex 29, France
 *
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 */


#ifndef SI_ARNAUD_ARNAUDMODEL_H
#define SI_ARNAUD_ARNAUDMODEL_H

#include <si/model/model.h>
#include <cartobase/smart/rcptr.h>
#include <map>
#include <string>

namespace sigraph
{
  class ArnaudSimilarityModel : public Model
  {
  public:
    ArnaudSimilarityModel( float simweight = 1, float simdist=20 );
    virtual ~ArnaudSimilarityModel();
    virtual Model* clone() const;
    ArnaudSimilarityModel & operator = 
    ( const ArnaudSimilarityModel & );

    virtual double prop( const Clique* cl );
    virtual void buildTree( Tree & tr ) const;

    static void buildSimilarity( carto::AttributedObject* parent, Tree* ao, 
                                 const std::string & filename );

  private:
    float	_simweight;
    float       _simdist;
  };


  class ArnaudDataDrivenModel : public Model
  {
  public:
    ArnaudDataDrivenModel( float ddweight = 1, float ddh=0.1, float ddx1=1, float ddx2=2);
    virtual ~ArnaudDataDrivenModel();
    virtual Model* clone() const;
    ArnaudDataDrivenModel & operator = 
    ( const ArnaudDataDrivenModel & );

    virtual double prop( const Clique* cl );
    virtual void buildTree( Tree & tr ) const;

    static void buildDataDriven( carto::AttributedObject* parent, Tree* ao, 
                                 const std::string & filename );

  private:
    float	_ddweight;
    float   _ddh;
    float   _ddx1;
    float   _ddx2;
  };
  
  class ArnaudLowerScaleModel : public Model
  {
  public:
    ArnaudLowerScaleModel( float lsweight = 1);
    virtual ~ArnaudLowerScaleModel();
    virtual Model* clone() const;
    ArnaudLowerScaleModel & operator = 
    ( const ArnaudLowerScaleModel & );

    virtual double prop( const Clique* cl );
    virtual void buildTree( Tree & tr ) const;

    static void buildLowerScale( carto::AttributedObject* parent, Tree* ao, 
                                 const std::string & filename );

  private:
    float	_lsweight;
  };
  
  class ArnaudIntraPSModel : public Model
  {
  public:
    ArnaudIntraPSModel( float ipsweight = 1);
    virtual ~ArnaudIntraPSModel();
    virtual Model* clone() const;
    ArnaudIntraPSModel & operator = 
    ( const ArnaudIntraPSModel & );

    virtual double prop( const Clique* cl );
    virtual void buildTree( Tree & tr ) const;

    static void buildIntraPS( carto::AttributedObject* parent, Tree* ao, 
                              const std::string & filename );

  private:
    float	_ipsweight;
  };

}

#endif


