/*
 *  Copyright (C) 2004 CEA
 *
 *  This software and supporting documentation were developed by
 *   Olivier Coulon
 *   Laboratoire LSIS,Groupe LXAO
 *   ESIL, campus de Luminy, Case 925,
 *   13288 Marseille Cedex 29, France
 *
 *   CEA/DSV/SHFJ
 *   4 place du General Leclerc
 *   91401 Orsay cedex
 *   France
 *
 */


#ifndef SI_FUNCTIONALSKETCH_SURFACEBASEDFUNCTIONALSKETCHMODEL_H
#define SI_FUNCTIONALSKETCH_SURFACEBASEDFUNCTIONALSKETCHMODEL_H

#include <si/model/model.h>
#include <cartobase/smart/rcptr.h>
#include <map>
#include <string>

namespace sigraph
{

   struct SurfaceBasedOverlapMap
  {
      std::map<std::pair<Vertex *, Vertex * >,double> value;
  };

  class SurfaceBasedFunctionalSketchSimilarityModel : public Model
  {
  public:
    SurfaceBasedFunctionalSketchSimilarityModel( float simweight = 1, float simdistance = 1);
    virtual ~SurfaceBasedFunctionalSketchSimilarityModel();
    virtual Model* clone() const;
    SurfaceBasedFunctionalSketchSimilarityModel & operator =
        ( const SurfaceBasedFunctionalSketchSimilarityModel & );

    virtual double prop( const Clique* cl );
/*    virtual double prop( const Clique* cl, const std::map<Vertex*, std::string> & changes );*/
    virtual void buildTree( Tree & tr ) const;

    static void buildSimilarity( carto::AttributedObject* parent, Tree* ao,                                 
                                 const std::string & filename );
    float getdis(void){return _simdistance;}

  private:
    float _simweight;
    float _simdistance;
  };


  class SurfaceBasedFunctionalSketchDataDrivenModel : public Model
  {
  public:
    SurfaceBasedFunctionalSketchDataDrivenModel( float ddweight = 1, float ddh=0.1, float ddx1=1, float ddx2=2);
    virtual ~SurfaceBasedFunctionalSketchDataDrivenModel();
    virtual Model* clone() const;
    SurfaceBasedFunctionalSketchDataDrivenModel & operator =
        ( const SurfaceBasedFunctionalSketchDataDrivenModel & );

    virtual double prop( const Clique* cl );
    virtual double prop( const Clique* cl, const std::map<Vertex*, std::string> & changes );

    virtual void buildTree( Tree & tr ) const;

    static void buildDataDriven( carto::AttributedObject* parent, Tree* ao,
                                 const std::string & filename );

  private:
    float _ddweight;
    float   _ddh;
    float   _ddx1;
    float   _ddx2;
  };

  class SurfaceBasedFunctionalSketchLowerScaleModel : public Model
  {
  public:
    SurfaceBasedFunctionalSketchLowerScaleModel( float lsweight = 1);
    virtual ~SurfaceBasedFunctionalSketchLowerScaleModel();
    virtual Model* clone() const;
    SurfaceBasedFunctionalSketchLowerScaleModel & operator =
        ( const SurfaceBasedFunctionalSketchLowerScaleModel & );

    virtual double prop( const Clique* cl );
    virtual double prop( const Clique* cl, const std::map<Vertex*, std::string> & changes );

    virtual void buildTree( Tree & tr ) const;

    static void buildLowerScale( carto::AttributedObject* parent, Tree* ao,
                                 const std::string & filename );

  private:
    float _lsweight;
  };

  class SurfaceBasedFunctionalSketchIntraPSModel : public Model
  {
  public:
    SurfaceBasedFunctionalSketchIntraPSModel( float ipsweight = 1);
    virtual ~SurfaceBasedFunctionalSketchIntraPSModel();
    virtual Model* clone() const;
    SurfaceBasedFunctionalSketchIntraPSModel & operator =
        ( const SurfaceBasedFunctionalSketchIntraPSModel & );

    virtual double prop( const Clique* cl );
    virtual double prop( const Clique* cl, const std::map<Vertex*, std::string> & changes );

    virtual double update( const Clique* cl);
    //virtual double update( const Clique* cl, const std::map<Vertex*, std::string> & changes );

    virtual void buildTree( Tree & tr ) const;

    static void buildIntraPS( carto::AttributedObject* parent, Tree* ao,
                              const std::string & filename );

  private:
    float _ipsweight;
    float _test;
  };

}

#endif


