/*
 *  Copyright (C) 2004 CEA
 *
 *  This software and supporting documentation were developed by
 *   CEA/DSV/SHFJ
 *   4 place du General Leclerc
 *   91401 Orsay cedex
 *   France
 *
 */

#include <si/functionalsketch/functionalsketchmodel.h>
#include <si/functionalsketch/functionalsketchattrib.h>
#include <si/graph/vertexclique.h>
#include <graph/tree/tree.h>
#include <aims/math/mathelem.h>
#include <iostream>

using namespace sigraph;
using namespace carto;
using namespace std;

//---------------------------------------------------------------------------------------
//-------------- similarity model-------------------------------------------------------
//---------------------------------------------------------------------------------------

FunctionalSketchSimilarityModel::FunctionalSketchSimilarityModel( float simweight )
  : Model(), _simweight( simweight )
{
}


FunctionalSketchSimilarityModel::~FunctionalSketchSimilarityModel()
{
}


FunctionalSketchSimilarityModel &
FunctionalSketchSimilarityModel::operator =
( const FunctionalSketchSimilarityModel & x )
{
  *(Model *) this = x;
  return *this;
}


Model* FunctionalSketchSimilarityModel::clone() const
{
  return new FunctionalSketchSimilarityModel( *this );
}


double FunctionalSketchSimilarityModel::prop( const Clique* cl )
{
  // cout << "FunctionalSketchSimilarityModel::prop\n";

  // REM : adaptation of SulcalSketchSimilarityModel
  // a little crappy still... to be improved

  double            pot = 0;
  const VertexClique          *vcl = (const VertexClique *) cl;

    // OLD WAY : ONE BIG CLIQUE
/*

  VertexClique::const_iterator     ic, ec = vcl->end();
  string            label, vlabel, vlabel2;

  map<string, set<Vertex *> > vertices;
  string            subject;

  //cout << "Similarity clique with " << vcl->size() << " nodes" << endl;

  //cout << "Sorting nodes" << endl;
  // Sorting clique nodes by subjects
  for( ic=vcl->begin(); ic!=ec; ++ic )
  {
      (*ic)->getProperty( SIA_SUBJECT, subject );
      vertices[ subject ].insert( *ic );
  }

//   // Test : have we computed overlaps yet ? If not let's do it
  carto::rc_ptr<OverlapMap> overlap;
  //OverlapMap *overlap;
  int index1, index2;
  std::map<std::string, std::set<Vertex *> >::iterator is, js, es = vertices.end();
  std::set<Vertex *>::iterator     iv, ev, jv, ejv;
  //vector<float>             barywhite, barylindon;
  std::vector<float>         bbmin_1, bbmin_2, bbmax_1, bbmax_2;
//   float                    distance, tmin1, tmin2, tmax1, tmax2;
  double overlap_x, overlap_y, overlap_z, rec, sim;
  int no_overlap;


  if ( !cl->getProperty("overlaps", overlap) )
  {
      std::cout << "Overlaps between blobs have never been computed, doing it now (it takes time...)" << endl;
      overlap.reset(new OverlapMap);
      //overlap=new OverlapMap;
      for( is=vertices.begin(); is!=es; ++is )
      {
          std::cout << "subject " << is->first << ": " << is->second.size()  << " vertices\n";
          for( iv=is->second.begin(), ev=is->second.end(); iv!=ev; ++iv )
          {
              (*iv)->getProperty( "index", index1 );
              (*iv)->getProperty( "boundingbox_max", bbmax_1);
              (*iv)->getProperty( "boundingbox_min", bbmin_1);
              for( js=is, ++js; js!=es; ++js )
              {
                  //std::cout << "\tMatching with subject " << js->first << endl;
                  for( jv=js->second.begin(), ejv=js->second.end(); jv!=ejv; ++jv )
                  {
                      (*jv)->getProperty( "index", index2 );
                      (*jv)->getProperty( "boundingbox_max", bbmax_2);
                      (*jv)->getProperty( "boundingbox_min", bbmin_2);
                      no_overlap=0;
                      if (bbmin_1[0]<=bbmin_2[0])
                          if (bbmax_1[0]<bbmin_2[0]) no_overlap=1;
                          else overlap_x= (bbmax_2[0] < bbmax_1[0] ? bbmax_2[0] : bbmax_1[0]) - bbmin_2[0] +1;
                      else
                          if (bbmax_2[0]<bbmin_1[0]) no_overlap=1;
                          else overlap_x= (bbmax_1[0] < bbmax_2[0] ? bbmax_1[0] : bbmax_2[0]) - bbmin_1[0] +1;
                      if (no_overlap==0)
                      {
                          if (bbmin_1[1]<=bbmin_2[1])
                              if (bbmax_1[1]<bbmin_2[1]) no_overlap=1;
                              else overlap_y= (bbmax_2[1] < bbmax_1[1] ? bbmax_2[1] : bbmax_1[1]) - bbmin_2[1] +1;
                          else
                              if (bbmax_2[1]<bbmin_1[1]) no_overlap=1;
                              else overlap_y= (bbmax_1[1] < bbmax_2[1] ? bbmax_1[1] : bbmax_2[1]) - bbmin_1[1] +1;
                          if (no_overlap==0)
                          {
                              if (bbmin_1[2]<=bbmin_2[2])
                                  if (bbmax_1[2]<bbmin_2[2]) no_overlap=1;
                                  else overlap_z= (bbmax_2[2] < bbmax_1[2] ? bbmax_2[2] : bbmax_1[2]) - bbmin_2[2] +1;
                              else
                                  if (bbmax_2[2]<bbmin_1[2]) no_overlap=1;
                                  else overlap_z= (bbmax_1[2] < bbmax_2[2] ? bbmax_1[2] : bbmax_2[2]) - bbmin_1[2] +1;
                              if (no_overlap==0)
                              {
                                  rec=overlap_x*overlap_y*overlap_z;
                                  double div=( ((bbmax_1[0]-bbmin_1[0])*(bbmax_1[1]-bbmin_1[1])*(bbmax_1[2]-bbmin_1[2]) +1)
                                          + ((bbmax_2[0]-bbmin_2[0])*(bbmax_2[1]-bbmin_2[1])*(bbmax_2[2]-bbmin_2[2]) +1) );

                                  rec=2 * rec / div;
                                  (overlap->value)[std::pair<Vertex *,Vertex * >((*iv),(*jv))]=rec;
                              }
                          }
                      }
                  }
              }
          }
      }
      const_cast<Clique*>(cl)->setProperty("overlaps", overlap);
  }

  //DEBUG
  //cout << "FunctionalSketchSimilarityModel::prop" << endl;

// Trying a new -hopefully faster- way. Old way commented below
// Now we just go through the overlap map and use the pre-computed overlaps...
// That way we don't need to go through all vertices and check if they are in the map.

  std::map<std::pair<Vertex *, Vertex * >,double>::iterator itOver, endOver=(overlap->value).end();
  std::pair<Vertex *, Vertex * > neigh;

  for (itOver=(overlap->value).begin(); itOver!=endOver; ++itOver)
  {
      neigh=(*itOver).first;
      rec=(*itOver).second;
      Vertex *n1, *n2;
      n1=neigh.first;
      n2=neigh.second;
      n1->getProperty( SIA_LABEL, vlabel );
      n2->getProperty( SIA_LABEL, vlabel2 );
      if ((vlabel != "0") && (vlabel==vlabel2))
      {
          sim=_simweight*(exp(-rec)-1)/(1-exp(-1.0));
          pot+=sim;
      }
  }
*/


// NEW WAY : MANY SMALL CLIQUES

    double rec;
    string vlabel1, vlabel2;
    VertexClique::const_iterator     ic;

    if (vcl->size() != 2)
    {
          std::cerr << "ERROR : a similarity clique is of order " << vcl->size() << " (should be 2)" << endl;
          exit(1);
    }
    ic=vcl->begin();
    (*ic)->getProperty( SIA_LABEL, vlabel1 );
    ++ic;
    (*ic)->getProperty( SIA_LABEL, vlabel2 );
     if ((vlabel1 != "0") && (vlabel2 != "0") && (vlabel1 == vlabel2))
     {
          vcl->getProperty("overlap", rec);
          pot=_simweight*(exp(-rec)-1)/(1-exp(-1.0));
     }

// EVEN OLDER WAY ;: OBSOLETE

  // Parcours de la liste des sujets
  //     Parcours de la liste des noeuds du sujet
  //       Parcours de la liste des sujets suivants
  //         R�cup�ration des noeuds de ces sujets-l�
  //   You know what I mean...
  //cout << "Let's go" << endl;
//   for( is=vertices.begin(); is!=es; ++is )
//   {
//       //cout << "subject " << is->first << ": " << is->second.size()  << " vertices\n";
//       for( iv=is->second.begin(), ev=is->second.end(); iv!=ev; ++iv )
//       {
//           (*iv)->getProperty( SIA_LABEL, vlabel );
//           if (vlabel != "0")
//           {
//               (*iv)->getProperty( "index", index1 );
//               for( js=is, ++js; js!=es; ++js )
//               {
//                   //cout << "\tMatching node with subject " << js->first << endl;
//                   for( jv=js->second.begin(), ejv=js->second.end(); jv!=ejv; ++jv )
//                   {
//                       (*jv)->getProperty( SIA_LABEL, vlabel2 );
//                       if (vlabel2 == vlabel)
//                       {
//                           (*jv)->getProperty( "index", index2 );
//                           if ((overlap->value).find(std::pair<Vertex *, Vertex *>((*iv),(*jv))) != (overlap->value).end())
//                           {
//                               rec=(overlap->value)[std::pair<Vertex *, Vertex *>((*iv),(*jv))];
//                               //cout << "REC = " << rec << endl;
//                               sim=_simweight*(exp(-rec)-1)/(1-exp(-1.0));
//                           }
//                           else if  ((overlap->value).find(std::pair<Vertex *, Vertex *>((*jv),(*iv))) != (overlap->value).end())
//                           {
//                               rec=(overlap->value)[std::pair<Vertex *, Vertex *>((*jv),(*iv))];
//                               //cout << "REC (===) = " << rec << endl;
//                               sim=_simweight*(exp(-rec)-1)/(1-exp(-1.0));
//                           }
//                           else
//                               sim=0;
//                           pot+= sim;
//                       }
//                   }
//               }
//           }
//       }
//   }


  //DEBUG
  //cout << "exit" << endl;

  return pot;
}

// double FunctionalSketchSimilarityModel::prop( const Clique* cl, const std::map<Vertex*, std::string> & changes )
// {
//      double result=0, oldpot=0, pot=0;
//      int ns;
//
//      if ( !cl->getProperty("overlaps", overlap) )
//      {
//           cerr << "PROBLEM : SimilarityModel::prop(difference) -> overlap maps have not been computed" << endl;
//           exit(0);
//      }
//
//      cl->getProperty(SIA_POTENTIAL, oldpot);
//      cl->getProperty( "num_subjects", ns );
//      // DEBUG
//      std::cout << "SIMILARITY : change" << endl;
//
//      std::map<Vertex*, std::string>::const_iterator itChange=changes.begin();
//      for ( ;itChange!=changes.end(); ++itChange )
//      {
//
//      }
//
//      std::cout << "Just a temporary comment" << endl;
//
//      return result;
// }

void FunctionalSketchSimilarityModel::buildTree( Tree & tr ) const
{
  tr.setSyntax( SIA_FUNCTIONALSKETCH_SIMILARITY_MODEL_SYNTAX );
  tr.setProperty( "similarity_weight", _simweight );
}


void
FunctionalSketchSimilarityModel::buildSimilarity( carto::AttributedObject*
                                              /* parent */,
                                              Tree* ao,
                                              const std::string &
                                              /* filename */ )
{
  float   simweight = 1;
  ao->getProperty( "similarity_weight", simweight );
  FunctionalSketchSimilarityModel  *sm
    = new FunctionalSketchSimilarityModel( simweight );

  ao->setProperty( "pointer", (Model *) sm );
  // parseModel( sm, parent, ao );
}

//---------------------------------------------------------------------------------------
//-------------- data driven model ------------------------------------------------------
//---------------------------------------------------------------------------------------

FunctionalSketchDataDrivenModel::FunctionalSketchDataDrivenModel( float ddweight, float ddh, float ddx1, float ddx2 )
  : Model(), _ddweight( ddweight ), _ddh( ddh ), _ddx1( ddx1 ), _ddx2( ddx2 )
{
}

FunctionalSketchDataDrivenModel::~FunctionalSketchDataDrivenModel()
{
}

FunctionalSketchDataDrivenModel &
FunctionalSketchDataDrivenModel::operator =
( const FunctionalSketchDataDrivenModel & x )
{
  *(Model *) this = x;
  _ddweight = x._ddweight;
  _ddh = x._ddh;
  _ddx1 = x._ddx1;
  _ddx2 = x._ddx2;
  return *this;
}

Model* FunctionalSketchDataDrivenModel::clone() const
{
  return new FunctionalSketchDataDrivenModel( *this );
}

double FunctionalSketchDataDrivenModel::prop( const Clique* cl )
{
  const VertexClique          *vcl = (const VertexClique *) cl;
  VertexClique::const_iterator     ic, ec = vcl->end();
  set<Vertex *>     vertices;
  string            vlabel;
  int                    ns;
  double            result=0, pot;
  float maxInt;

  //DEBUG
  //cout << "FunctionalSketchDataDrivenModel::prop" << endl;

  cl->getProperty( "num_subjects", ns );

  for( ic=vcl->begin(); ic!=ec; ++ic )
  {
      (*ic)->getProperty( SIA_LABEL, vlabel );
      if( vlabel != "0" )
      {
          (*ic)->getProperty("maxIntensity", maxInt); // CHOIX DELA  MESURE !!!
          if (maxInt < _ddx1) pot=_ddweight;
          else if (maxInt > _ddx2) pot=_ddh;
          else pot = _ddweight - (maxInt- _ddx1) * (_ddweight - _ddh)/(double(_ddx2 - _ddx1));
          result+= pot;
      }
  }

  //DEBUG
  //cout << "exit" << endl;

  return(ns*result);

}

double FunctionalSketchDataDrivenModel::prop( const Clique* cl, const std::map<Vertex*, std::string> & changes )
{

     // ici on r�cup�re l'ancien pot, on ne travaille que sur les noeuds qui ont chang�,
     // et on calcule le nouveau pot par diff�rence avec l'ancien

     int ns;
     string lab, oldlab;
     double oldpot=0, pot=0, val=0;
     Vertex *vert;
     float maxInt;

     cl->getProperty(SIA_POTENTIAL, oldpot);
     cl->getProperty( "num_subjects", ns );
     // DEBUG
     //std::cout << "DataDrivenModel->change" << endl;

     std::map<Vertex*, std::string>::const_iterator itChange=changes.begin();
     for ( ;itChange!=changes.end(); ++itChange )
     {
          vert=(*itChange).first;
          oldlab=(*itChange).second;
          vert->getProperty( SIA_LABEL, lab );
          if ((lab != "0") && (oldlab == "0"))
          {
               vert->getProperty("maxIntensity", maxInt); // CHOIX DELA  MESURE !!!
               if (maxInt < _ddx1) val=_ddweight;
               else if (maxInt > _ddx2) val=_ddh;
               else val=_ddweight - (maxInt- _ddx1) * (_ddweight - _ddh)/(double(_ddx2 - _ddx1));
               pot+=val;
          }
          else if ((lab == "0") && (oldlab != "0"))
          {
               vert->getProperty("maxIntensity", maxInt); // CHOIX DELA  MESURE !!!
               if (maxInt < _ddx1) val=_ddweight;
               else if (maxInt > _ddx2) val=_ddh;
               else val=_ddweight - (maxInt- _ddx1) * (_ddweight - _ddh)/(double(_ddx2 - _ddx1));
               pot-=val;
          }
     }

     return(oldpot+(ns*pot));

}

void FunctionalSketchDataDrivenModel::buildTree( Tree & tr ) const
{

  tr.setSyntax( SIA_FUNCTIONALSKETCH_DATADRIVEN_MODEL_SYNTAX );

  tr.setProperty( "datadriven_weight", _ddweight );
  tr.setProperty( "datadriven_value_h", _ddh );
  tr.setProperty( "datadriven_value_x1", _ddx1 );
  tr.setProperty( "datadriven_value_x2", _ddx2 );
}

void
FunctionalSketchDataDrivenModel::buildDataDriven( carto::AttributedObject*
                                              /* parent */,
                                              Tree* ao,
                                              const std::string &
                                              /* filename */ )
{
  float   ddweight = 1.0, ddh=0.1, ddx1=1.0, ddx2=2.0;
  ao->getProperty( "datadriven_weight", ddweight );
  ao->getProperty( "datadriven_value_h", ddh );
  ao->getProperty( "datadriven_value_x1", ddx1 );
  ao->getProperty( "datadriven_value_x2", ddx2 );

  FunctionalSketchDataDrivenModel  *sm
    = new FunctionalSketchDataDrivenModel( ddweight, ddh, ddx1, ddx2 );

  ao->setProperty( "pointer", (Model *) sm );
  // parseModel( sm, parent, ao );
}

//---------------------------------------------------------------------------------------
//-------------- lower scale best model -------------------------------------------------
//---------------------------------------------------------------------------------------

FunctionalSketchLowerScaleModel::FunctionalSketchLowerScaleModel( float weight )
  : Model(), _lsweight( weight )
{
}


FunctionalSketchLowerScaleModel::~FunctionalSketchLowerScaleModel()
{
}


FunctionalSketchLowerScaleModel &
FunctionalSketchLowerScaleModel::operator =
( const FunctionalSketchLowerScaleModel & x )
{
  *(Model *) this = x;
  _lsweight = x._lsweight;
  return *this;
}


Model* FunctionalSketchLowerScaleModel::clone() const
{
  return new FunctionalSketchLowerScaleModel( *this );
}


double FunctionalSketchLowerScaleModel::prop( const Clique* cl )
{
  const VertexClique          *vcl = (const VertexClique *) cl;
  VertexClique::const_iterator     ic, ec = vcl->end();
  string            vlabel;
  int                    ns;
  float             trep;
  double            result=0;

  //DEBUG
  //cout << "FunctionalSketchLowerScaleModel::prop" << endl;

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

double FunctionalSketchLowerScaleModel::prop( const Clique* cl, const std::map<Vertex*, std::string> & changes )
{
     // ici on r�cup�re l'ancien pot, on ne travaille que sur les noeuds qui ont chang�,
     // et on calcule le nouveau pot par diff�rence avec l'ancien

     int ns;
     string lab, oldlab;
     double oldpot=0, pot=0;
     Vertex *vert;
     float trep;


     cl->getProperty(SIA_POTENTIAL, oldpot);
     cl->getProperty( "num_subjects", ns );
     // DEBUG
     //std::cout << "LowerScaleModel -> change" << endl;

     std::map<Vertex*, std::string>::const_iterator itChange=changes.begin();
     for ( ;itChange!=changes.end(); ++itChange )
     {
          vert=(*itChange).first;
          oldlab=(*itChange).second;
          vert->getProperty( SIA_LABEL, lab );
          if ((lab != "0") && (oldlab == "0"))
          {
               vert->getProperty("trep", trep);
               pot+= trep;
          }
          else if ((lab == "0") && (oldlab != "0"))
          {
               vert->getProperty("trep", trep);
               pot-= trep;
          }
     }
     return(oldpot+(_lsweight*ns*pot));
}

void FunctionalSketchLowerScaleModel::buildTree( Tree & tr ) const
{

  tr.setSyntax( SIA_FUNCTIONALSKETCH_LOWERSCALEBEST_MODEL_SYNTAX );
  tr.setProperty( "scale_weight", _lsweight );

}


void
FunctionalSketchLowerScaleModel::buildLowerScale( carto::AttributedObject*
                                              /* parent */,
                                              Tree* ao,
                                              const std::string &
                                              /* filename */ )
{

  float lsweight=1.0;
  ao->getProperty( "scale_weight", lsweight );

  FunctionalSketchLowerScaleModel  *sm
    = new FunctionalSketchLowerScaleModel( lsweight );

  ao->setProperty( "pointer", (Model *) sm );
  // parseModel( sm, parent, ao );
}

//---------------------------------------------------------------------------------------
//-------------- intraPS  model -------------------------------------------------------
//---------------------------------------------------------------------------------------

FunctionalSketchIntraPSModel::FunctionalSketchIntraPSModel( float weight )
  : Model(), _ipsweight( weight )
{
}


FunctionalSketchIntraPSModel::~FunctionalSketchIntraPSModel()
{
}


FunctionalSketchIntraPSModel &
FunctionalSketchIntraPSModel::operator =
( const FunctionalSketchIntraPSModel & x )
{
  *(Model *) this = x;
  _ipsweight = x._ipsweight;
  return *this;
}


Model* FunctionalSketchIntraPSModel::clone() const
{
  return new FunctionalSketchIntraPSModel( *this );
}


double FunctionalSketchIntraPSModel::prop( const Clique* cl )
{
     rc_ptr<std::map<string, int> > countLab, countLab2;
     const VertexClique          *vcl = (const VertexClique *) cl;
     VertexClique::const_iterator     ic, ec = vcl->end();
     string            vlabel;
     double result=0, pot=0;

     if (!cl->getProperty("countLabels", countLab)) // if table does not exist it needs to be created
     {
          cerr << "IntraPSModel : starting update from prop" << endl;
          result=update(cl);
     }
     else // if it exists we leave it and compute a new one
          // (only update() is allowed to change the actual table)
     {
          countLab2.reset( new std::map<string, int> );
          for( ic=vcl->begin(); ic!=ec; ++ic )
          {
               (*ic)->getProperty( SIA_LABEL, vlabel);
               if (vlabel != "0")
               {
                    if ((*countLab2).find(vlabel)==(*countLab2).end())
                         (*countLab2)[vlabel]=1;
                    else
                         (*countLab2)[vlabel]=(*countLab2)[vlabel]+1;
               }
          }

          // computing potential
          std::map<string, int>::iterator itC=(*countLab2).begin();
         int nl, ns;
          cl->getProperty( "num_subjects", ns );

          for ( ;itC!=(*countLab2).end(); ++itC)
          {
               nl=(*itC).second;
               if (nl<=1)
                    pot=0.0;
               else
                    pot=double(ns*_ipsweight*nl);
              result+=pot;
          }
     }
     return(result);
}

double FunctionalSketchIntraPSModel::prop( const Clique* cl, const std::map<Vertex*, std::string> & changes )
{
     double result, oldpot;
     rc_ptr<std::map<string, int> > countTable;
     std::map<string, int> countLab;
     int ns;
     Vertex *vert;
     string lab, oldLab;

     cl->getProperty( "num_subjects", ns );
     cl->getProperty(SIA_POTENTIAL, oldpot);


     //computing potential
     if (!cl->getProperty("countLabels", countTable)) // if table does not exist let's create it
     {
          std::cerr << "IntraPSModel : update from prop(change)" << endl;
          result=update(cl);
          std::cerr << "out" << endl;
     }
     else
     {
          result=0.0;
          std::map<Vertex*, std::string>::const_iterator itChange=changes.begin();
          for ( ;itChange!=changes.end(); ++itChange )
          {
               vert=(*itChange).first;
               oldLab=(*itChange).second;
               vert->getProperty( SIA_LABEL, lab );

               if (lab!="0")
               {
                    if (countLab.find(lab) != countLab.end())
                         countLab[lab]=countLab[lab]+1;
                    else
                         countLab[lab]=1;
               }
               if (oldLab!="0")
               {
                    if (countLab.find(oldLab) == countLab.end())
                         countLab[oldLab]=-1;
                    else
                         countLab[oldLab]=countLab[oldLab]-1;
               }
          }


          std::map<string, int>::iterator itC=(*countTable).begin();
          double pot=0;
          int nl;

          for ( ;itC!=(*countTable).end(); ++itC)
          {
               nl=(*itC).second;
               lab=(*itC).first;
               if (countLab.find(lab) != countLab.end())
                    nl=nl+countLab[lab];
               if (nl < 0)
               {
                    cerr << "Problem in functionalSketchIntraPSModel::prop() : negative sum of labels" << endl;
                    exit(1);
               }
               else if (nl<=1)
                    pot=0.0;
               else
                    pot=double(ns*_ipsweight*nl);
               result+=pot;
          }
     }
     return(result);
}

double FunctionalSketchIntraPSModel::update( const Clique* cl )
{

     // Computing a table that contains the number of occurence
     // of each positive label in the clique
     // REM : remember : one clique per subject;

     //cout << "On passe dans update() " << endl;
     rc_ptr<std::map<string, int> > countTable;
     if (!cl->getProperty("countLabels", countTable)) // if table does not exist we allocate it first
     {
          cout << "FunctionalSketchIntraPSModel::update => countTable did not exist..." << endl;
          countTable.reset( new std::map<string, int> );
          const_cast<Clique*>(cl)->setProperty("countLabels", countTable);
     }
     (*countTable).clear();

     const VertexClique              *vcl = (const VertexClique *) cl;
     VertexClique::const_iterator     ic, ec = vcl->end();
     string                           vlabel;

     for( ic=vcl->begin(); ic!=ec; ++ic )
     {
          (*ic)->getProperty( SIA_LABEL, vlabel);
          if (vlabel != "0")
          {
               if ((*countTable).find(vlabel)==(*countTable).end())
                    (*countTable)[vlabel]=1;
               else
                    (*countTable)[vlabel]=(*countTable)[vlabel]+1;
          }

     }

     // computing potential
     std::map<string, int>::iterator itC=(*countTable).begin();
     double result=0.0, pot=0.0;
     int nl, ns;
     cl->getProperty( "num_subjects", ns );

     for ( ;itC!=(*countTable).end(); ++itC)
     {
          nl=(*itC).second;
          if (nl<=1)
               pot=0.0;
          else
               pot=double(ns*_ipsweight*nl);
          result+=pot;
     }
     return(result);

  }

  // No need for the following function at the moment
// double FunctionalSketchIntraPSModel::update( const Clique* cl, const std::map<Vertex*, std::string> & changes )
// {
//      double result=0;
//
//      int ns;
//      string lab, oldlab;
//      double oldpot=0, pot=0;
//      Vertex *vert;
//
//      cl->getProperty(SIA_POTENTIAL, oldpot);
//      cl->getProperty( "num_subjects", ns );
//      // DEBUG
//      //std::cout << "LowerScaleModel -> change" << endl;
//
//      std::map<string, int> *countTable, newCount;
//      if (!cl->getProperty("countLabels", countTable)) // if table does not exist
//      {
//           // Houston, we have a problem
//           cerr << "IntraPSModel::update(change) -> countLabels does not exist" << endl;
//           exit(1))
//      }
//
//      std::map<Vertex*, std::string>::const_iterator itChange=changes.begin();
//      for ( ;itChange!=changes.end(); ++itChange )
//      {
//           vert=(*itChange).first;
//           oldlab=(*itChange).second;
//           vert->getProperty( SIA_LABEL, lab );
//           if ((lab != "0") && (oldlab == "0"))
//           {
//
//                vert->getProperty("trep", trep);
//                pot+= trep;
//           }
//           else if ((lab == "0") && (oldlab != "0"))
//           {
//                vert->getProperty("trep", trep);
//                pot-= trep;
//           }
//      }
//      return(oldpot+(ns*pot));
/*
     return(result);
}*/



void FunctionalSketchIntraPSModel::buildTree( Tree & tr ) const
{

  tr.setSyntax( SIA_FUNCTIONALSKETCH_INTRAPS_MODEL_SYNTAX );
  tr.setProperty( "intraps_weight", _ipsweight );
}


void
FunctionalSketchIntraPSModel::buildIntraPS( carto::AttributedObject*
                                              /* parent */,
                                              Tree* ao,
                                              const std::string &
                                              /* filename */ )
{
  float   ipsweight = 1.0;
  ao->getProperty( "intraps_weight", ipsweight );
  FunctionalSketchIntraPSModel     *sm
    = new FunctionalSketchIntraPSModel( ipsweight );

  ao->setProperty( "pointer", (Model *) sm );
  // parseModel( sm, parent, ao );
}



