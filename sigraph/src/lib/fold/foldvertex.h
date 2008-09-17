/*
 *  Copyright (C) 2005 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 */

#ifndef SI_FOLD_FOLDVERTEX_H
#define SI_FOLD_FOLDVERTEX_H


#include <graph/graph/vertex.h>

namespace sigraph
{

  class FoldVertex : public Vertex
  {
  public:
    virtual ~FoldVertex();

    static GenericObject* makeFold( const std::string & );

    // public properties for fast access
    float		size;
    float		refsize;
    bool		refsize_valid;
    int			index;
    bool		index_valid;
    std::string		label;
    bool		label_valid;
    std::vector<float>	gravity_center;
    bool		gravity_center_valid;
    std::vector<float>	refgravity_center;
    bool		refgravity_center_valid;
    std::vector<float>	normal;
    bool		normal_valid;
    std::vector<float>	refnormal;
    bool		refnormal_valid;
    float		mindepth;
    bool		mindepth_valid;
    float		refmindepth;
    bool		refmindepth_valid;
    float		maxdepth;
    bool		maxdepth_valid;
    float		refmaxdepth;
    bool		refmaxdepth_valid;
    float		meandepth;
    bool		meandepth_valid;
    float		refmeandepth;
    bool		refmeandepth_valid;
    float		surface_area;
    bool		surface_area_valid;
    float		refsurface_area;
    bool		refsurface_area_valid;
    int			bottom_point_number;
    bool		bottom_point_number_valid;

  protected:
    FoldVertex( const std::string & s );
    FoldVertex( const FoldVertex & s );
    virtual Vertex * cloneVertex() const;
    void addBuiltins();
  };

}

#endif

