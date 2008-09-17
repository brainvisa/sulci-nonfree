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

#include <si/sulcalSketch_Arnaud/arnaudmodel.h>
#include <si/sulcalSketch_Arnaud/arnaudattrib.h>
#include <si/graph/vertexclique.h>
#include <graph/tree/tree.h>
#include <aims/math/mathelem.h>
#include <iostream>

using namespace sigraph;
using namespace std;

//---------------------------------------------------------------------------------------
//-------------- similarity model-------------------------------------------------------
//---------------------------------------------------------------------------------------

ArnaudSimilarityModel::ArnaudSimilarityModel( float simweight, float simdist )
  : Model(), _simweight( simweight ), _simdist( simdist  )
{
}


ArnaudSimilarityModel::~ArnaudSimilarityModel()
{
}


ArnaudSimilarityModel & 
ArnaudSimilarityModel::operator = 
( const ArnaudSimilarityModel & x )
{
  *(Model *) this = x;
  return *this;
}


Model* ArnaudSimilarityModel::clone() const
{
  return new ArnaudSimilarityModel( *this );
}


double ArnaudSimilarityModel::prop( const Clique* cl )
{
  double			pot = 0;
  const VertexClique		*vcl = (const VertexClique *) cl;

  float dist;
  vcl->getProperty("distance", dist);
  pot=_simweight*((dist/_simdist) -1.0);
  
  
  
  return pot;
}


void ArnaudSimilarityModel::buildTree( Tree & tr ) const
{
  tr.setSyntax( SIA_ARNAUD_SIMILARITY_MODEL_SYNTAX );
  tr.setProperty( "similarity_weight", _simweight );
  tr.setProperty( "similarity_distance", _simdist );
}


void 
ArnaudSimilarityModel::buildSimilarity( carto::AttributedObject* 
                                              /* parent */,
                                              Tree* ao, 
                                              const std::string & 
                                              /* filename */ )
{
  float	simweight = 1, simdist=20;
  ao->getProperty( "similarity_weight", simweight );
    ao->getProperty( "similarity_distance", simdist );
  ArnaudSimilarityModel	*sm 
    = new ArnaudSimilarityModel( simweight, simdist );

  ao->setProperty( "pointer", (Model *) sm );
  // parseModel( sm, parent, ao );
}

//---------------------------------------------------------------------------------------
//-------------- data driven model ------------------------------------------------------
//---------------------------------------------------------------------------------------

ArnaudDataDrivenModel::ArnaudDataDrivenModel( float ddweight, float ddh, float ddx1, float ddx2 )
  : Model(), _ddweight( ddweight ), _ddh( ddh ), _ddx1( ddx1 ), _ddx2( ddx2 )
{
}

ArnaudDataDrivenModel::~ArnaudDataDrivenModel()
{
}

ArnaudDataDrivenModel & 
ArnaudDataDrivenModel::operator = 
( const ArnaudDataDrivenModel & x )
{
  *(Model *) this = x;
  _ddweight = x._ddweight;
  _ddh = x._ddh;
  _ddx1 = x._ddx1;
  _ddx2 = x._ddx2;
  return *this;
}

Model* ArnaudDataDrivenModel::clone() const
{
  return new ArnaudDataDrivenModel( *this );
}

double ArnaudDataDrivenModel::prop( const Clique* cl )
{
  const VertexClique		*vcl = (const VertexClique *) cl;
  VertexClique::const_iterator	ic, ec = vcl->end();
  set<Vertex *>	vertices;
  string			vlabel;
  int				ns;
  double            result=0, pot;
  float maxInt;

  //DEBUG
  //cout << "ArnaudDataDrivenModel::prop" << endl;
    
  cl->getProperty( "num_subjects", ns );

  for( ic=vcl->begin(); ic!=ec; ++ic )
  {
      (*ic)->getProperty( SIA_LABEL, vlabel );
      if( vlabel != "0" )
      {
          (*ic)->getProperty("maxIntensity", maxInt); // CHOIX DELA  MESURE !!!    
          if (maxInt < _ddx1) pot=_ddweight;
          else if (maxInt > _ddx2) pot=_ddh;
          else pot=( (_ddweight-_ddh)*maxInt + (_ddx1*_ddh - _ddx2*_ddweight) )/(double(_ddx1 - _ddx2));
          result+= pot;
      }
  }
  
  //DEBUG
  //cout << "exit" << endl;
    
  return(ns*result);
  
}

void ArnaudDataDrivenModel::buildTree( Tree & tr ) const
{

  tr.setSyntax( SIA_ARNAUD_DATADRIVEN_MODEL_SYNTAX );
 
  tr.setProperty( "datadriven_weight", _ddweight );
  tr.setProperty( "datadriven_value_h", _ddh );
  tr.setProperty( "datadriven_value_x1", _ddx1 );
  tr.setProperty( "datadriven_value_x2", _ddx2 );
}

void 
ArnaudDataDrivenModel::buildDataDriven( carto::AttributedObject* 
                                              /* parent */,
                                              Tree* ao, 
                                              const std::string & 
                                              /* filename */ )
{
  float	ddweight = 1.0, ddh=0.1, ddx1=1.0, ddx2=2.0;
  ao->getProperty( "datadriven_weight", ddweight );
  ao->getProperty( "datadriven_value_h", ddh );
  ao->getProperty( "datadriven_value_x1", ddx1 );
  ao->getProperty( "datadriven_value_x2", ddx2 );

  ArnaudDataDrivenModel	*sm 
    = new ArnaudDataDrivenModel( ddweight, ddh, ddx1, ddx2 );

  ao->setProperty( "pointer", (Model *) sm );
  // parseModel( sm, parent, ao );
}

//---------------------------------------------------------------------------------------
//-------------- lower scale best model -------------------------------------------------
//---------------------------------------------------------------------------------------

ArnaudLowerScaleModel::ArnaudLowerScaleModel( float weight )
  : Model(), _lsweight( weight )
{
}


ArnaudLowerScaleModel::~ArnaudLowerScaleModel()
{
}


ArnaudLowerScaleModel & 
ArnaudLowerScaleModel::operator = 
( const ArnaudLowerScaleModel & x )
{
  *(Model *) this = x;
  _lsweight = x._lsweight;
  return *this;
}


Model* ArnaudLowerScaleModel::clone() const
{
  return new ArnaudLowerScaleModel( *this );
}


double ArnaudLowerScaleModel::prop( const Clique* cl )
{
  const VertexClique		*vcl = (const VertexClique *) cl;
  VertexClique::const_iterator	ic, ec = vcl->end();
  string			vlabel;
  int				ns;
  float             trep;
  double            result=0;
  
  //DEBUG
  //cout << "ArnaudLowerScaleModel::prop" << endl;
  
  cl->getProperty( "num_subjects", ns );
  for( ic=vcl->begin(); ic!=ec; ++ic )
  {
      (*ic)->getProperty( SIA_LABEL, vlabel );
      if( vlabel != "0" )
      {
          (*ic)->getProperty("trep", trep); 
          result+= trep;
      }
  }
  //DEBUG
  //cout << "exit" << endl;
  return(_lsweight*ns*result);
}


void ArnaudLowerScaleModel::buildTree( Tree & tr ) const
{

  tr.setSyntax( SIA_ARNAUD_LOWERSCALEBEST_MODEL_SYNTAX );
  tr.setProperty( "scale_weight", _lsweight );

}


void 
ArnaudLowerScaleModel::buildLowerScale( carto::AttributedObject* 
                                              /* parent */,
                                              Tree* ao, 
                                              const std::string & 
                                              /* filename */ )
{

  float lsweight=1.0;
  ao->getProperty( "scale_weight", lsweight );

  ArnaudLowerScaleModel	*sm 
    = new ArnaudLowerScaleModel( lsweight );

  ao->setProperty( "pointer", (Model *) sm );
  // parseModel( sm, parent, ao );
}

//---------------------------------------------------------------------------------------
//-------------- intraPS  model -------------------------------------------------------
//---------------------------------------------------------------------------------------

ArnaudIntraPSModel::ArnaudIntraPSModel( float weight )
  : Model(), _ipsweight( weight )
{
}


ArnaudIntraPSModel::~ArnaudIntraPSModel()
{
}


ArnaudIntraPSModel & 
ArnaudIntraPSModel::operator = 
( const ArnaudIntraPSModel & x )
{
  *(Model *) this = x;
  _ipsweight = x._ipsweight;
  return *this;
}


Model* ArnaudIntraPSModel::clone() const
{
  return new ArnaudIntraPSModel( *this );
}


double ArnaudIntraPSModel::prop( const Clique* cl )
{
  const VertexClique		*vcl = (const VertexClique *) cl;
  VertexClique::const_iterator	ic, ec = vcl->end();
  string			vlabel;
  int				ns;
  std::map<string, map<string, set<Vertex *> > > vertices;
  double pot, result=0;
  string subject;
  
  // DEBUG
  //cout << "ArnaudIntraPSModel::prop" << endl;
  cl->getProperty( "num_subjects", ns );
  
  // Sorting clique nodes by subjects
  for( ic=vcl->begin(); ic!=ec; ++ic )
  {
      (*ic)->getProperty( SIA_SUBJECT, subject );
      (*ic)->getProperty( SIA_LABEL, vlabel);
      std::map<string, set<Vertex *> > & lv = vertices[ subject ];
      lv[ vlabel ].insert( *ic );
  }

  // Computing potential
  
  std::map<string, map<string, set<Vertex *> > >::iterator iv, ev=vertices.end();
  for (iv=vertices.begin(); iv!=ev; ++iv)
  {
      map<string, set<Vertex *> > ivm=(*iv).second;
      map<string, set<Vertex *> >::iterator il, el=ivm.end();
      for (il=ivm.begin();  il!=el; ++il)
      {
          int nl=(*il).second.size();
          vlabel=(*il).first;
          if ((vlabel != "0") || (nl<=1)) pot=0;
          else pot=ns*_ipsweight*nl;
          result+=pot;
      }
  }
  
  //DEBUG
  //cout << "exit" << endl;
  return(result);
  
}

void ArnaudIntraPSModel::buildTree( Tree & tr ) const
{

  tr.setSyntax( SIA_ARNAUD_INTRAPS_MODEL_SYNTAX );
  tr.setProperty( "intraps_weight", _ipsweight );
}


void 
ArnaudIntraPSModel::buildIntraPS( carto::AttributedObject* 
                                              /* parent */,
                                              Tree* ao, 
                                              const std::string & 
                                              /* filename */ )
{
  float	ipsweight = 1.0;
  ao->getProperty( "intraps_weight", ipsweight );
  ArnaudIntraPSModel	*sm 
    = new ArnaudIntraPSModel( ipsweight );

  ao->setProperty( "pointer", (Model *) sm );
  // parseModel( sm, parent, ao );
}



