/*
 *  Copyright (C) 2003-2005 CEA - LSIS
 *
 *  This software and supporting documentation were developed by
 *
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
 
#include <si/sulcalSketch_Arnaud/arnaudfinder.h>
#include <si/sulcalSketch_Arnaud/arnaudattrib.h>
#include <si/graph/cgraph.h>
#include <si/graph/vertexclique.h>
#include <si/domain/domain.h>
#include <iostream>

using namespace sigraph;
using namespace aims;
using namespace carto;
using namespace std;


ArnaudFinder::ArnaudFinder( MGraph & mg ) : ModelFinder( mg )
{
}


ArnaudFinder::~ArnaudFinder()
{
}

AttributedObject* ArnaudFinder::selectModel( const Clique* cl )
{
  AttributedObject	*mod = 0;
  string		mt;
  
  if( !cl->getProperty( SIA_MODEL_TYPE, mt ) )
  {
      cerr << "ArnaudFinder::selectModel: clique with no type\n";
  }
  
  //cout << "ArnaudFinder::selectModel : looking at clique with model : " << mt;
  
  if (mt=="arnaud_similarity")
  {
      //cout << "\tSimilarity" << endl;
      string str, mtype;
      //cl->getProperty( SIA_LABEL, str );
      //set<Vertex*>	sv = _mgraph.getVerticesWith( SIA_LABEL, str );
      set<Vertex*>	sv = _mgraph.vertices();
      set<Vertex*>::iterator	im, em = sv.end();
      for( im=sv.begin(); im!=em; ++im )
      {
          mtype = "";
          (*im)->getProperty( SIA_MODEL_TYPE, mtype );
          if( mtype == "arnaud_similarity" )
          {
              mod = *im;
              break;
          }
      }
  }
  else if (mt=="arnaud_datadriven")
  {
  
      //cout << "\tDatadriven" << endl;
      
      string str, mtype;
      //l->getProperty( SIA_LABEL, str );
      //set<Vertex*>	sv = _mgraph.getVerticesWith( SIA_LABEL, str );
      set<Vertex*>	sv = _mgraph.vertices();
      set<Vertex*>::iterator	im, em = sv.end();
      for( im=sv.begin(); im!=em; ++im )
      {
          mtype = "";
          (*im)->getProperty( SIA_MODEL_TYPE, mtype );
          if( mtype == "arnaud_datadriven" )
          {
              mod = *im;
              break;
          }
      }
  }
  else if (mt=="arnaud_intraprimalsketch")
  {
      //cout << "\tIntraprimalsketch" << endl;
      string str, mtype;
/*      cl->getProperty( SIA_LABEL, str );
      set<Vertex*>	sv = _mgraph.getVerticesWith( SIA_LABEL, str );*/
      set<Vertex*>	sv = _mgraph.vertices();
      set<Vertex*>::iterator	im, em = sv.end();
      for( im=sv.begin(); im!=em; ++im )
      {
          mtype = "";
          (*im)->getProperty( SIA_MODEL_TYPE, mtype );
          if( mtype == "arnaud_intraprimalsketch" )
          {
              mod = *im;
              break;
          }
      }
  }
  else if (mt=="arnaud_lowerscalebest")
  {
  
      //cout << "\tLowerscalebest" << endl;
  
  
      string str, mtype;
//       cl->getProperty( SIA_LABEL, str );
//       set<Vertex*>	sv = _mgraph.getVerticesWith( SIA_LABEL, str );
      set<Vertex*>	sv = _mgraph.vertices();      
      set<Vertex*>::iterator	im, em = sv.end();
      for( im=sv.begin(); im!=em; ++im )
      {
          mtype = "";
          (*im)->getProperty( SIA_MODEL_TYPE, mtype );
          if( mtype == "arnaud_lowerscalebest" )
          {
              mod = *im;
              break;
          }
      }
  }
  return mod;
}

void ArnaudFinder::initCliques( CGraph &data, bool /* verbose */, 
                                      bool /* withCache */, 
                                      bool /* translateLabels */, 
                                      bool /* checkLabels */, 
                                      const SelectionSet * /* sel */ )
{
   Graph::iterator	iv, jv, ev = data.end();
  const MGraph		& mg = mGraph();
  Graph::const_iterator	imv, emv = mg.end();
  
  map<string, VertexClique *>    cliquesdata;
  VertexClique *	          cliqueslowscale = 0;
  map<string, VertexClique *>    cliquesintraPS;
  
  string			label, label1, mtype, subject;
  VertexClique			*vc;
  CGraph::CliqueSet		& scl = data.cliques();
  Domain			*fd;
  Vertex			*v;
  vector<string>		*pl;
  
  int nbcliqueSim=0;
  std::vector<float>		gc1, gc2;
  float				dist, distThreshold=20.0;
  vector<int> vl;
  string			voidl = SIV_VOID_LABEL;

  _mgraph.getProperty( SIA_VOID_LABEL, voidl );

  //mg.getProperty("label_list", vl);
  //cout << "Label_list.size()=" << vl.size() << endl;
  int i;
  for (i=0; i<21; i++)
      vl.push_back(i);
  
  cout << "Init cliques : label list = ";
  vector<int>::iterator ivl;
  for (ivl=vl.begin(); ivl!=vl.end(); ++ivl)
      cout << " " << (*ivl);
  cout << endl;
  
  cout << "Computing all cliques. It might be long, be patient..." << endl;

  //ancien 
  //mg.getAttribute("label_list", vl);

  for( imv=mg.begin(); imv!=emv; ++imv )
  {
      mtype = "";
      (*imv)->getProperty( SIA_MODEL_TYPE, mtype );
      
      // Old way
//       if ( mtype == "arnaud_similarity" )
//       {
//           vc = new VertexClique;
//           cliquessim = vc;
//           vc->setProperty( SIA_MODEL_TYPE, 
//                             string( "arnaud_similarity" ) );
//           scl.insert( vc );
//       }
      //else
      
      // end of old way
      
      if ( mtype == "arnaud_lowerscalebest" )
      {
          vc = new VertexClique;
          cliqueslowscale = vc;
          vc->setProperty( SIA_MODEL_TYPE, 
                            string( "arnaud_lowerscalebest") );
          scl.insert( rc_ptr<Clique>( vc ) );
      }
  }
  
  for (iv=data.begin(); iv!=ev; ++iv)
  {
      v = *iv;
      if( v->getProperty( SIA_POSSIBLE_LABELS, pl ) )
	      delete pl;
      pl = new vector<string>;
      vector<string> pl2;
      pl->push_back( voidl );
      v->setProperty( SIA_POSSIBLE_LABELS, pl ); 
      vector<int>::iterator ivl=vl.begin();
      for (; ivl!=vl.end(); ++ivl)
      {
          std::ostringstream lstr;
          lstr << (*ivl);
          //cout << "Adding label : " << (*ivl) << endl;
          pl->push_back(lstr.str());
      }
      
      for( imv=_mgraph.begin(); imv!=emv; ++imv )
	  {
	      (*imv)->getProperty( SIA_DOMAIN, fd );
//           if( fd->canBeFound( v, &data ) )
//           {
//               cout << "Test : on est dans le domaine" << endl;
              mtype = "";
	          (*imv)->getProperty( SIA_MODEL_TYPE, mtype );
              if (mtype == "arnaud_datadriven")
              {
                  v->getProperty( SIA_SUBJECT, subject );
                  VertexClique *& c = cliquesdata[ subject ];
                  if( c == 0 )
                  {
                      c = new VertexClique;
                      c->setProperty( SIA_MODEL_TYPE, 
                                       string( "arnaud_datadriven" ) );
                      scl.insert( rc_ptr<Clique>( c ) );
                  }
                  c->addVertex( v );	// noeud dans la clique
              }
              else if (mtype == "arnaud_similarity")
              {
                  (*imv)->getProperty("similarity_distance", distThreshold);
              //Old way
                  //cliquessim->addVertex( v );	// noeud dans la clique
              // Beginning of new way
              // REM : new way is one clique for each pair of matching blobs
              // Old way is one clique containing all blobs
                  string subject1, subject2;
                  (*iv)->getProperty( SIA_SUBJECT, subject1 );
                  (*iv)->getProperty( "gravity_center", gc1);
                  for (jv=iv, ++jv; jv!=ev; ++jv)
                  {
                      (*jv)->getProperty( SIA_SUBJECT, subject2 );
                      if (subject1 != subject2)
                      {
                          (*jv)->getProperty( "gravity_center", gc2);
                          dist=sqrt( (gc2[0]-gc1[0])*(gc2[0]-gc1[0]) + (gc2[1]-gc1[1])*(gc2[1]-gc1[1]) + (gc2[2]-gc1[2])*(gc2[2]-gc1[2]) );
                          
                          if (dist<=distThreshold)
                          {
                              vc = new VertexClique;
                              nbcliqueSim++;
                              vc->setProperty( SIA_MODEL_TYPE, string( "arnaud_similarity" ) );
                              vc->addVertex( (*iv) );
                              vc->addVertex( (*jv) );
                              vc->setProperty("distance", dist);
                              scl.insert( rc_ptr<Clique>( vc ) );
                          }
                      }
                  
                  }
              }
              else if (mtype == "arnaud_lowerscalebest")
              {
                  cliqueslowscale->addVertex( v );	// noeud dans la clique
              }
              else if (mtype == "arnaud_intraprimalsketch")
              {
                  v->getProperty( SIA_SUBJECT, subject );
                  VertexClique *& c = cliquesintraPS[ subject ];
                  if( c == 0 )
                  {
                      c = new VertexClique;
                      c->setProperty( SIA_MODEL_TYPE, 
                                       string( "arnaud_intraprimalsketch" ) );
                      scl.insert( rc_ptr<Clique>( c ) );
                  }
                  c->addVertex( v );	// noeud dans la clique
              }
              else
                  cerr << "problem : model type " << mtype << " unknown" << endl;
//           }
//           else
//           {
//               cout << "Test : on est PAS dans le domaine" << endl;
//           }          else
      }
  }

  int	ns = cliquesdata.size();
  
  std::cout << "Found " << ns << " subjects" << endl; 
  std::cout << "There is " << nbcliqueSim << " similarity cliques" << endl;
  
  map<string, VertexClique *>::iterator  idc, edc = cliquesdata.end();
  for( idc=cliquesdata.begin(); idc!=edc; ++idc )
      idc->second->setProperty( "num_subjects", ns );
      
  map<string, VertexClique *>::iterator iic, eic = cliquesintraPS.end();
  for( iic=cliquesintraPS.begin(); iic!=eic; ++iic )
      iic->second->setProperty( "num_subjects", ns );
      
  cliqueslowscale->setProperty( "num_subjects", ns );
}
