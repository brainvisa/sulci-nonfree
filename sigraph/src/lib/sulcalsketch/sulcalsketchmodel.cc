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

#include <si/sulcalsketch/sulcalsketchmodel.h>
#include <si/sulcalsketch/sulcalsketchattrib.h>
#include <si/graph/vertexclique.h>
#include <graph/tree/tree.h>
#include <aims/math/mathelem.h>
#include <iostream>

using namespace sigraph;
using namespace aims;
using namespace std;

SulcalSketchSimilarityModel::SulcalSketchSimilarityModel( float distthresh, 
                                                          float distslope, 
                                                          float distweight, 
                                                          float sclslope, 
                                                          float sclweight )
  : Model(), _distthresh( distthresh ), _distslope( distslope ), 
    _distweight( distweight ), _sclslope( sclslope ), _sclweight( sclweight )
{
}


SulcalSketchSimilarityModel::~SulcalSketchSimilarityModel()
{
}


SulcalSketchSimilarityModel & 
SulcalSketchSimilarityModel::operator = 
( const SulcalSketchSimilarityModel & x )
{
  *(Model *) this = x;
  return *this;
}


Model* SulcalSketchSimilarityModel::clone() const
{
  return new SulcalSketchSimilarityModel( *this );
}


double SulcalSketchSimilarityModel::prop( const Clique* cl )
{
  // cout << "SulcalSketchSimilarityModel::prop\n";

  const VertexClique		*vcl = (const VertexClique *) cl;
  VertexClique::const_iterator	ic, ec = vcl->end();
  string			label, vlabel;
  double			pot = 0;
  map<string, set<Vertex *> >	vertices;
  string			subject;

  // cout << "clique with " << vcl->size() << " nodes" << endl;

  cl->getProperty( SIA_LABEL, label );
  for( ic=vcl->begin(); ic!=ec; ++ic )
    {
      (*ic)->getProperty( SIA_LABEL, vlabel );
      if( label == vlabel && (*ic)->getProperty( SIA_SUBJECT, subject ) )
        vertices[ subject ].insert( *ic );
    }

  // cout << "label: " << label << ", subjects: " << vertices.size() << endl;
  map<string, set<Vertex *> >::iterator	is, js, es = vertices.end();
  set<Vertex *>::iterator	iv, ev, jv, ejv;
  vector<float>			barywhite, barylindon;
  float				distance, tmin1, tmin2, tmax1, tmax2, dscale;

  // debug
  unsigned			n = 0;

  /* Parcours de la liste des sujets
       Parcours de la liste des noeuds du sujet
         Parcours de la liste des sujets suivants
           Récupérartion des noeuds de ces sujets-là
     You know what I mean...
   */
  for( is=vertices.begin(); is!=es; ++is )
    {
      /* cout << "subject " << is->first << ": " << is->second.size() 
         << " vertices\n"; */
      for( iv=is->second.begin(), ev=is->second.end(); iv!=ev; ++iv )
        {
          (*iv)->getProperty( "refgravity_center", barywhite );
          (*iv)->getProperty( "tmin", tmin1 );
          (*iv)->getProperty( "tmax", tmax1 );
          // cout << "barywhite: " << barywhite.size() << endl;
          for( js=is, ++js; js!=es; ++js )
            for( jv=js->second.begin(), ejv=js->second.end(); jv!=ejv; ++jv )
              {
                ++n;
                // distance between gravity centers
                (*jv)->getProperty( "refgravity_center", barylindon );
                // cout << "barylindon: " << barylindon.size() << endl;
                distance = sqrt( sqr( barywhite[0] - barylindon[0] ) 
                                 + sqr( barywhite[1] - barylindon[1] ) 
                                 + sqr( barywhite[2] - barylindon[2] ) );
                pot += _distweight * atan( _distslope 
                                           * ( distance - _distthresh ) ) 
                  * 2./M_PI;
                // scale distance
                (*jv)->getProperty( "tmin", tmin2 );
                (*jv)->getProperty( "tmax", tmax2 );
                dscale = 0;
                if( tmin2 > tmax1 && tmax1 > 0 )
                  dscale = log( tmin2 / tmax1 );
                else if( tmin1 > tmax2 && tmax2 > 0 )
                  dscale = log( tmin1 / tmax2 );
                pot += _sclweight * atan( _sclslope * dscale ) * 2./M_PI;
              }
        }
    }
  cout << "nb of pairs: " << n << "  \r" << flush;

  return pot;
}


void SulcalSketchSimilarityModel::buildTree( Tree & tr ) const
{
  tr.setSyntax( SIA_SULCALSKETCH_SIMILARITY_MODEL_SYNTAX );
  tr.setProperty( "distance_threshold", _distthresh );
  tr.setProperty( "distance_slope", _distslope );
  tr.setProperty( "distance_weight", _distweight );
  tr.setProperty( "scaledistance_slope", _sclslope );
  tr.setProperty( "scaledistance_weight", _sclweight );
}


void 
SulcalSketchSimilarityModel::buildSimilarity( carto::AttributedObject* 
                                              /* parent */,
                                              Tree* ao, 
                                              const std::string & 
                                              /* filename */ )
{
  float	distthresh, distslope = 1, distweight = 1, sclslope = 1, sclweight = 1;
  ao->getProperty( "distance_threshold", distthresh );
  ao->getProperty( "distance_slope", distslope );
  ao->getProperty( "distance_weight", distweight );
  ao->getProperty( "scaledistance_slope", sclslope );
  ao->getProperty( "scaledistance_weight", sclweight );
  SulcalSketchSimilarityModel	*sm 
    = new SulcalSketchSimilarityModel( distthresh, distslope, distweight );

  ao->setProperty( "pointer", (Model *) sm );
  // parseModel( sm, parent, ao );
}


SulcalSketchDataDrivenModel::SulcalSketchDataDrivenModel( float weight )
  : Model(), _weight( weight )
{
}


SulcalSketchDataDrivenModel::~SulcalSketchDataDrivenModel()
{
}


SulcalSketchDataDrivenModel & 
SulcalSketchDataDrivenModel::operator = 
( const SulcalSketchDataDrivenModel & x )
{
  *(Model *) this = x;
  _weight = x._weight;
  return *this;
}


Model* SulcalSketchDataDrivenModel::clone() const
{
  return new SulcalSketchDataDrivenModel( *this );
}


double SulcalSketchDataDrivenModel::prop( const Clique* cl )
{
  const VertexClique		*vcl = (const VertexClique *) cl;
  VertexClique::const_iterator	ic, ec = vcl->end();
  string			label, vlabel;
  unsigned			n = 0;
  int				ns;

  cl->getProperty( SIA_LABEL, label );
  cl->getProperty( "num_subjects", ns );

  for( ic=vcl->begin(); ic!=ec; ++ic )
    {
      (*ic)->getProperty( SIA_LABEL, vlabel );
      if( label == vlabel )
        ++n;
    }
  //cout << "label " << label << ": n: " << n << ", weight: " << _weight << endl;
  if( n > 1 )
    return sqr(n-1) * _weight * ns;
  else
    return 0;
}


void SulcalSketchDataDrivenModel::buildTree( Tree & tr ) const
{
  tr.setSyntax( SIA_SULCALSKETCH_DATADRIVEN_MODEL_SYNTAX );
  tr.setProperty( "weight", _weight );
}


void 
SulcalSketchDataDrivenModel::buildDataDriven( carto::AttributedObject* 
                                              /* parent */,
                                              Tree* ao, 
                                              const std::string & 
                                              /* filename */ )
{
  float	weight = 1;
  ao->getProperty( "weight", weight );
  SulcalSketchDataDrivenModel	*sm 
    = new SulcalSketchDataDrivenModel( weight );

  ao->setProperty( "pointer", (Model *) sm );
  // parseModel( sm, parent, ao );
}


