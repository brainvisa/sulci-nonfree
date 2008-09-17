/*
 *  Copyright (C) 2003 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 */

#ifndef SI_ROI_ROIDESCR_H
#define SI_ROI_ROIDESCR_H


#include <si/descr/adapDescr.h>


namespace sigraph
{

  class RoiDescr : public AdapDescr
  {
  public:
    enum Inputs
      {
	VEC_VALID, 
	END
      };

    RoiDescr();
    RoiDescr( const RoiDescr & f );
    virtual ~RoiDescr();
    virtual CliqueDescr* clone() const;

    virtual bool hasChanged( const Clique* cl, 
			     const std::map<Vertex*, std::string> & changes, 
			     const carto::GenericObject* model = 0 ) const;
    virtual void buildTree( Tree & t );
    virtual std::vector<std::string> descriptorsNames() const;
    virtual std::string name() const;

  protected:
    virtual bool makeVectorElements( const Clique* cl, 
                                     std::vector<double> & vec, 
                                     carto::GenericObject* model = 0 );
  };


  //	inline


  inline CliqueDescr* RoiDescr::clone() const
  {
    return( new RoiDescr( *this ) );
  }

}

#endif

