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


#ifndef SI_FUNCTIONALSKETCH_FUNCTIONALSKETCHMODEL_H
#define SI_FUNCTIONALSKETCH_FUNCTIONALSKETCHMODEL_H

#include <si/model/model.h>
#include <cartobase/smart/rcptr.h>
#include <map>
#include <string>

namespace sigraph
{

   struct OverlapMap
  {
      std::map<std::pair<Vertex *, Vertex * >,double> value;
  };

  class FunctionalSketchSimilarityModel : public Model
  {
  public:
    FunctionalSketchSimilarityModel( float simweight = 1);
    virtual ~FunctionalSketchSimilarityModel();
    virtual Model* clone() const;
    FunctionalSketchSimilarityModel & operator =
    ( const FunctionalSketchSimilarityModel & );

    virtual double prop( const Clique* cl );
/*    virtual double prop( const Clique* cl, const std::map<Vertex*, std::string> & changes );*/
    virtual void buildTree( Tree & tr ) const;

    static void buildSimilarity( carto::AttributedObject* parent, Tree* ao,
                                 const std::string & filename );

  private:
    float _simweight;
  };


  class FunctionalSketchDataDrivenModel : public Model
  {
  public:
    FunctionalSketchDataDrivenModel( float ddweight = 1, float ddh=0.1, float ddx1=1, float ddx2=2);
    virtual ~FunctionalSketchDataDrivenModel();
    virtual Model* clone() const;
    FunctionalSketchDataDrivenModel & operator =
    ( const FunctionalSketchDataDrivenModel & );

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

  class FunctionalSketchLowerScaleModel : public Model
  {
  public:
    FunctionalSketchLowerScaleModel( float lsweight = 1);
    virtual ~FunctionalSketchLowerScaleModel();
    virtual Model* clone() const;
    FunctionalSketchLowerScaleModel & operator =
    ( const FunctionalSketchLowerScaleModel & );

    virtual double prop( const Clique* cl );
    virtual double prop( const Clique* cl, const std::map<Vertex*, std::string> & changes );

    virtual void buildTree( Tree & tr ) const;

    static void buildLowerScale( carto::AttributedObject* parent, Tree* ao,
                                 const std::string & filename );

  private:
    float _lsweight;
  };

  class FunctionalSketchIntraPSModel : public Model
  {
  public:
    FunctionalSketchIntraPSModel( float ipsweight = 1);
    virtual ~FunctionalSketchIntraPSModel();
    virtual Model* clone() const;
    FunctionalSketchIntraPSModel & operator =
    ( const FunctionalSketchIntraPSModel & );

    virtual double prop( const Clique* cl );
    virtual double prop( const Clique* cl, const std::map<Vertex*, std::string> & changes );

    virtual double update( const Clique* cl);
    //virtual double update( const Clique* cl, const std::map<Vertex*, std::string> & changes );

    virtual void buildTree( Tree & tr ) const;

    static void buildIntraPS( carto::AttributedObject* parent, Tree* ao,
                              const std::string & filename );

  private:
    float _ipsweight;
  };

}

#endif


