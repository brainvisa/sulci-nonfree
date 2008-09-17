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

#include <si/fold/interfoldedge.h>
#include <si/fold/fattrib.h>

using namespace sigraph;
using namespace carto;
using namespace std;


CorticalEdge::~CorticalEdge()
{
}


CorticalEdge::CorticalEdge( const std::string & s )
  : UEdge( s ), size( 0 ), refsize( 0 ), refsize_valid( false ), 
    dist( 0 ), dist_valid( false ), 
    refdist( 0 ), refdist_valid( false ), 
    SS1nearest_valid( false ), refSS1nearest_valid( false ), 
    SS2nearest_valid( false ), refSS2nearest_valid( false ), 
    length( 0 ), length_valid( false ), 
    reflength( 0 ), reflength_valid( false )
{
  addBuiltins();
}


CorticalEdge::CorticalEdge( const CorticalEdge & x )
  : RCObject(), UEdge( x ), size( x.size ),
    refsize( x.refsize ), refsize_valid( x.refsize_valid ), 
    dist( x.dist ), dist_valid( x.dist_valid ), 
    refdist( x.refdist ), refdist_valid( x.refdist_valid ), 
    SS1nearest( x.SS1nearest ), SS1nearest_valid( x.SS1nearest_valid ), 
    refSS1nearest( x.refSS1nearest ), 
    refSS1nearest_valid( x.refSS1nearest_valid ), 
    SS2nearest( x.SS2nearest ), SS2nearest_valid( x.SS2nearest_valid ), 
    refSS2nearest( x.refSS2nearest ), 
    refSS2nearest_valid( x.refSS2nearest_valid ), 
    length( x.length ), length_valid( x.length_valid ), 
    reflength( x.reflength ), reflength_valid( x.reflength_valid )
{
  addBuiltins();
}


void CorticalEdge::addBuiltins()
{
  PropertySet	& ps = getValue();
  ps.addBuiltinProperty( SIA_SIZE, size );
  ps.addBuiltinProperty( SIA_REFSIZE, refsize, refsize_valid );
  ps.addBuiltinProperty( SIA_DIST, dist, dist_valid );
  ps.addBuiltinProperty( SIA_REFDIST, refdist, refdist_valid );
  ps.addBuiltinProperty( SIA_SS1NEAREST, SS1nearest, SS1nearest_valid );
  ps.addBuiltinProperty( SIA_REFSS1NEAREST, refSS1nearest, 
                         refSS1nearest_valid );
  ps.addBuiltinProperty( SIA_SS2NEAREST, SS2nearest, SS2nearest_valid );
  ps.addBuiltinProperty( SIA_REFSS2NEAREST, refSS2nearest, 
                         refSS2nearest_valid );
  ps.addBuiltinProperty( SIA_LENGTH, length, length_valid );
  ps.addBuiltinProperty( SIA_REFLENGTH, reflength, reflength_valid );
}


Edge * CorticalEdge::cloneEdge() const
{
  return new CorticalEdge( *this );
}


GenericObject* CorticalEdge::makeCortical( const string & s )
{
  return new CorticalEdge( s );
}

//

JunctionEdge::~JunctionEdge()
{
}


JunctionEdge::JunctionEdge( const std::string & s )
  : UEdge( s ), 
    size( 0 ), refsize( 0 ), refsize_valid( false ), 
    mindepth( 0 ), mindepth_valid( false ), 
    refmindepth( 0 ), refmindepth_valid( false ), 
    maxdepth( 0 ), maxdepth_valid( false ), 
    refmaxdepth( 0 ), refmaxdepth_valid( false ), 
    length( 0 ), length_valid( false ), 
    reflength( 0 ), reflength_valid( false )
{
  addBuiltins();
}


JunctionEdge::JunctionEdge( const JunctionEdge & x )
  : RCObject(), UEdge( x ), 
    size( x.size ), refsize( x.refsize ), refsize_valid( x.refsize_valid ), 
    mindepth( x.mindepth ), mindepth_valid( x.mindepth_valid ), 
    refmindepth( x.refmindepth ), refmindepth_valid( x.refmindepth_valid ), 
    maxdepth( x.maxdepth ), maxdepth_valid( x.maxdepth_valid ), 
    refmaxdepth( x.refmaxdepth ), refmaxdepth_valid( x.refmaxdepth_valid ), 
    length( x.length ), length_valid( x.length_valid ), 
    reflength( x.reflength ), reflength_valid( x.reflength_valid )
{
  addBuiltins();
}


void JunctionEdge::addBuiltins()
{
  PropertySet	& ps = getValue();
  ps.addBuiltinProperty( SIA_SIZE, size );
  ps.addBuiltinProperty( SIA_REFSIZE, refsize, refsize_valid );
  ps.addBuiltinProperty( SIA_MINDEPTH, mindepth, mindepth_valid );
  ps.addBuiltinProperty( SIA_REFMINDEPTH, refmindepth, refmindepth_valid );
  ps.addBuiltinProperty( SIA_MAXDEPTH, maxdepth, maxdepth_valid );
  ps.addBuiltinProperty( SIA_REFMAXDEPTH, refmaxdepth, refmaxdepth_valid );
  ps.addBuiltinProperty( SIA_LENGTH, length, length_valid );
  ps.addBuiltinProperty( SIA_REFLENGTH, reflength, reflength_valid );
}


Edge * JunctionEdge::cloneEdge() const
{
  return new JunctionEdge( *this );
}


GenericObject* JunctionEdge::makeJunction( const string & s )
{
  return new JunctionEdge( s );
}

//

HullJunctionEdge::~HullJunctionEdge()
{
}


HullJunctionEdge::HullJunctionEdge( const std::string & s )
  : UEdge( s ), 
    size( 0 ), refsize( 0 ), refsize_valid( false ), 
    direction_valid( false ), refdirection_valid( false ), 
    extremity1_valid( false ), extremity2_valid( false ), 
    refextremity1_valid( false ), refextremity2_valid( false ), 
    length( 0 ), length_valid( false ), 
    reflength( 0 ), reflength_valid( false )
{
  addBuiltins();
}


HullJunctionEdge::HullJunctionEdge( const HullJunctionEdge & x )
  : RCObject(), UEdge( x ),
    size( x.size ), refsize( x.refsize ), refsize_valid( x.refsize_valid ), 
    direction( x.direction ), direction_valid( x.direction_valid ), 
    refdirection( x.refdirection ), refdirection_valid( x.refdirection_valid ),
    extremity1( x.extremity1 ), extremity1_valid( x.extremity1_valid ), 
    extremity2( x.extremity2 ), extremity2_valid( x.extremity2_valid ), 
    refextremity1( x.refextremity1 ), 
    refextremity1_valid( x.refextremity1_valid ), 
    refextremity2( x.refextremity2 ), 
    refextremity2_valid( x.refextremity2_valid ), 
    length( x.length ), length_valid( x.length_valid ), 
    reflength( x.reflength ), reflength_valid( x.reflength_valid )
{
  addBuiltins();
}


void HullJunctionEdge::addBuiltins()
{
  PropertySet	& ps = getValue();
  ps.addBuiltinProperty( SIA_SIZE, size );
  ps.addBuiltinProperty( SIA_REFSIZE, refsize, refsize_valid );
  ps.addBuiltinProperty( SIA_DIRECTION, direction, direction_valid );
  ps.addBuiltinProperty( SIA_REFDIRECTION, refdirection, refdirection_valid );
  ps.addBuiltinProperty( SIA_EXTREMITY1, extremity1, extremity1_valid );
  ps.addBuiltinProperty( SIA_EXTREMITY2, extremity2, extremity2_valid );
  ps.addBuiltinProperty( SIA_REFEXTREMITY1, refextremity1, 
                         refextremity1_valid );
  ps.addBuiltinProperty( SIA_REFEXTREMITY2, refextremity2, 
                         refextremity2_valid );
  ps.addBuiltinProperty( SIA_LENGTH, length, length_valid );
  ps.addBuiltinProperty( SIA_REFLENGTH, reflength, reflength_valid );
}


Edge * HullJunctionEdge::cloneEdge() const
{
  return new HullJunctionEdge( *this );
}


GenericObject* HullJunctionEdge::makeHullJunction( const string & s )
{
  return new HullJunctionEdge( s );
}

//

PliDePassageEdge::~PliDePassageEdge()
{
}


PliDePassageEdge::PliDePassageEdge( const std::string & s )
  : UEdge( s ),
    size( 0 ), refsize( 0 ), refsize_valid( false ), 
    depth( 0 ), depth_valid( false ), 
    refdepth( 0 ), refdepth_valid( false )
{
  addBuiltins();
}


PliDePassageEdge::PliDePassageEdge( const PliDePassageEdge & x )
  : RCObject(), UEdge( x ),
    size( x.size ), refsize( x.refsize ), refsize_valid( x.refsize_valid ), 
    depth( x.depth ), depth_valid( x.depth_valid ), 
    refdepth( x.refdepth ), refdepth_valid( x.refdepth_valid )
{
  addBuiltins();
}


void PliDePassageEdge::addBuiltins()
{
  PropertySet	& ps = getValue();
  ps.addBuiltinProperty( SIA_SIZE, size );
  ps.addBuiltinProperty( SIA_REFSIZE, refsize, refsize_valid );
  ps.addBuiltinProperty( SIA_DEPTH, depth, depth_valid );
  ps.addBuiltinProperty( SIA_REFDEPTH, refdepth, refdepth_valid );
}


Edge * PliDePassageEdge::cloneEdge() const
{
  return new PliDePassageEdge( *this );
}


GenericObject* PliDePassageEdge::makePliDePassage( const string & s )
{
  return new PliDePassageEdge( s );
}


