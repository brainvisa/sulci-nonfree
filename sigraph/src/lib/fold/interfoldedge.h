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

#ifndef SI_FOLD_INTERFOLDEDGE_H
#define SI_FOLD_INTERFOLDEDGE_H


#include <graph/graph/uedge.h>

namespace sigraph
{

  class CorticalEdge : public UEdge
  {
  public:
    virtual ~CorticalEdge();

    static GenericObject* makeCortical( const std::string & );

    // public properties for fast access
    float	size;
    float	refsize;
    bool	refsize_valid;
    float	dist;
    bool	dist_valid;
    float	refdist;
    bool	refdist_valid;
    std::vector<int>	SS1nearest;
    bool	SS1nearest_valid;
    std::vector<float>	refSS1nearest;
    bool	refSS1nearest_valid;
    std::vector<int>	SS2nearest;
    bool	SS2nearest_valid;
    std::vector<float>	refSS2nearest;
    bool	refSS2nearest_valid;
    float	length;
    bool	length_valid;
    float	reflength;
    bool	reflength_valid;

  protected:
    CorticalEdge( const std::string & s );
    CorticalEdge( const CorticalEdge & s );
    virtual Edge * cloneEdge() const;
    void addBuiltins();
  };


  class JunctionEdge : public UEdge
  {
  public:
    virtual ~JunctionEdge();

    static GenericObject* makeJunction( const std::string & );

    // public properties for fast access
    float	size;
    float	refsize;
    bool	refsize_valid;
    float	mindepth;
    bool	mindepth_valid;
    float	refmindepth;
    bool	refmindepth_valid;
    float	maxdepth;
    bool	maxdepth_valid;
    float	refmaxdepth;
    bool	refmaxdepth_valid;
    float	length;
    bool	length_valid;
    float	reflength;
    bool	reflength_valid;

  protected:
    JunctionEdge( const std::string & s );
    JunctionEdge( const JunctionEdge & s );
    virtual Edge * cloneEdge() const;
    void addBuiltins();
  };


  class HullJunctionEdge : public UEdge
  {
  public:
    virtual ~HullJunctionEdge();

    static GenericObject* makeHullJunction( const std::string & );

    // public properties for fast access
    float		size;
    float		refsize;
    bool		refsize_valid;
    std::vector<float>	direction;
    bool		direction_valid;
    std::vector<float>	refdirection;
    bool		refdirection_valid;
    std::vector<int>	extremity1;
    bool		extremity1_valid;
    std::vector<int>	extremity2;
    bool		extremity2_valid;
    std::vector<float>	refextremity1;
    bool		refextremity1_valid;
    std::vector<float>	refextremity2;
    bool		refextremity2_valid;
    float		length;
    bool		length_valid;
    float		reflength;
    bool		reflength_valid;

  protected:
    HullJunctionEdge( const std::string & s );
    HullJunctionEdge( const HullJunctionEdge & s );
    virtual Edge * cloneEdge() const;
    void addBuiltins();
  };


  class PliDePassageEdge : public UEdge
  {
  public:
    virtual ~PliDePassageEdge();

    static GenericObject* makePliDePassage( const std::string & );

    // public properties for fast access
    float	size;
    float	refsize;
    bool	refsize_valid;
    float	depth;
    bool	depth_valid;
    float	refdepth;
    bool	refdepth_valid;

  protected:
    PliDePassageEdge( const std::string & s );
    PliDePassageEdge( const PliDePassageEdge & s );
    virtual Edge * cloneEdge() const;
    void addBuiltins();
  };

}

#endif

