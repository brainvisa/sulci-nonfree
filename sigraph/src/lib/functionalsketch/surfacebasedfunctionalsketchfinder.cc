/*
 *  Copyright (C) 2003-2005 CEA - LSIS
 *
 *  This software and supporting documentation were developed by
 *
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

#include <si/functionalsketch/surfacebasedfunctionalsketchfinder.h>
#include <si/functionalsketch/surfacebasedfunctionalsketchattrib.h>
#include <si/functionalsketch/surfacebasedfunctionalsketchmodel.h>
#include <si/graph/cgraph.h>
#include <si/graph/vertexclique.h>
#include <si/domain/domain.h>
#include <aims/io/reader.h>
#include <aims/mesh/surface.h>
#include <aims/mesh/texture.h>
#include <iostream>
#include <float.h>

using namespace sigraph;
using namespace aims;
using namespace carto;
using namespace std;

map<uint,float> getDistMap( AimsSurfaceTriangle *mesh,  std::map<unsigned, std::set<unsigned> >    *neighbours,  uint dep) 
{ 

  unsigned i; 
 
  std::multimap<float,unsigned>    front1, front2; 
  std::multimap<float,unsigned>    *cfront = &front1, *nfront = &front2, *tmpf; 
  std::multimap<float,unsigned>::iterator    iv, fv; 
  std::set<unsigned>                neigh; 
  std::set<unsigned>::iterator        in, fn; 
  float                    d, d2, l; 
  Point3df                pos; 
  float dist=0; 

  front1.insert( std::pair<float,unsigned>( 0, dep ) ); 

  map<uint,float> distmap;
  map<uint,float>::iterator distit;
  distmap[dep]=0.0;
  while(  dist<=20.0 ) 
  { 
    nfront->clear(); 
    neigh.clear(); 
 
    for( iv=cfront->begin(), fv=cfront->end(); iv!=fv; ++iv ) 
    { 
      i = (*iv).second; 
      d = (*iv).first; 
      for( in=(*neighbours)[i].begin(), fn=(*neighbours)[i].end(); in!=fn; ++in ) 
      { 
        distit = distmap.find(*in);
        if (distit == distmap.end()) distmap[*in] = FLT_MAX;
        d2 = distmap[*in]; 
        pos = (*mesh).vertex()[i] - (*mesh).vertex()[*in]; 
        l = sqrt( pos[0] * pos[0] + pos[1] * pos[1] + pos[2] * pos[2] ); 
        if( d2 > d + l ) 
        { 
          distmap[*in] = d+l;
//           tex.item( *in ) = d + l; 
          neigh.insert( *in ); 
//                     result_lim[0].item( *in )=tex.item( *in ); 
          dist=d+l;
        } 
      } 
    } 

   
    for( in=neigh.begin(), fn=neigh.end(); in!=fn; ++in ) 
    nfront->insert( std::pair<float,unsigned>( distmap[*in], *in ) ); 
 
    tmpf = cfront; 
    cfront = nfront; 
    nfront = tmpf; 
  } 
     
//   float dfinal=distmap[ind]; 
     
//   (*neighbours).clear(); 
  front1.clear(); 
  front2.clear(); 
  (*cfront).clear(); 
  (*nfront).clear(); 
  (*tmpf).clear(); 
  neigh.clear(); 
//   tex.item(dep) = FLT_MAX;
//     std::cout<<"SulcusCorticalSnake_energy : MeshDistance_adapt : return "<<vect[0]<<" et "<<vect[1]<<std::endl; 
  return( distmap ); 
}


SurfaceBasedFunctionalSketchFinder::SurfaceBasedFunctionalSketchFinder( MGraph & mg ) : ModelFinder( mg )
{
}


SurfaceBasedFunctionalSketchFinder::~SurfaceBasedFunctionalSketchFinder()
{
}

AttributedObject* SurfaceBasedFunctionalSketchFinder::selectModel( const Clique* cl )
{
  AttributedObject  *mod = 0;
  string       mt;

  if( !cl->getProperty( SIA_MODEL_TYPE, mt ) )
  {
    cerr << "SurfaceBasedFunctionalSketchFinder::selectModel: clique with no type\n";
  }

  //cout << "SurfaceBasedFunctionalSketchFinder::selectModel : looking at clique with model : " << mt;

  if (mt=="surfacebasedfunctionalsketch_similarity")
  {
      //cout << "\tSimilarity" << endl;
      string str, mtype;
      //cl->getProperty( SIA_LABEL, str );
      //set<Vertex*>     sv = _mgraph.getVerticesWith( SIA_LABEL, str );
      set<Vertex*>  sv = _mgraph.vertices();
      set<Vertex*>::iterator  im, em = sv.end();
      for( im=sv.begin(); im!=em; ++im )
      {
          mtype = "";
          (*im)->getProperty( SIA_MODEL_TYPE, mtype );
          if( mtype == "surfacebasedfunctionalsketch_similarity" )
          {
              mod = *im;
              break;
          }
      }
  }
  else if (mt=="surfacebasedfunctionalsketch_datadriven")
  {

      //cout << "\tDatadriven" << endl;

      string str, mtype;
      //l->getProperty( SIA_LABEL, str );
      //set<Vertex*>     sv = _mgraph.getVerticesWith( SIA_LABEL, str );
      set<Vertex*>  sv = _mgraph.vertices();
      set<Vertex*>::iterator  im, em = sv.end();
      for( im=sv.begin(); im!=em; ++im )
      {
          mtype = "";
          (*im)->getProperty( SIA_MODEL_TYPE, mtype );
          if( mtype == "surfacebasedfunctionalsketch_datadriven" )
          {
              mod = *im;
              break;
          }
      }
  }
  else if (mt=="surfacebasedfunctionalsketch_intraprimalsketch")
  {
      //cout << "\tIntraprimalsketch" << endl;
      string str, mtype;
/*      cl->getProperty( SIA_LABEL, str );
      set<Vertex*>  sv = _mgraph.getVerticesWith( SIA_LABEL, str );*/
      set<Vertex*>  sv = _mgraph.vertices();
      set<Vertex*>::iterator  im, em = sv.end();
      for( im=sv.begin(); im!=em; ++im )
      {
          mtype = "";
          (*im)->getProperty( SIA_MODEL_TYPE, mtype );
          if( mtype == "surfacebasedfunctionalsketch_intraprimalsketch" )
          {
              mod = *im;
              break;
          }
      }
  }
  else if (mt=="surfacebasedfunctionalsketch_lowerscalebest")
  {

      //cout << "\tLowerscalebest" << endl;


      string str, mtype;
//       cl->getProperty( SIA_LABEL, str );
//       set<Vertex*>    sv = _mgraph.getVerticesWith( SIA_LABEL, str );
      set<Vertex*>  sv = _mgraph.vertices();
      set<Vertex*>::iterator  im, em = sv.end();
      for( im=sv.begin(); im!=em; ++im )
      {
          mtype = "";
          (*im)->getProperty( SIA_MODEL_TYPE, mtype );
          if( mtype == "surfacebasedfunctionalsketch_lowerscalebest" )
          {
              mod = *im;
              break;
          }
      }
  }
  return mod;
}

void SurfaceBasedFunctionalSketchFinder::initCliques( CGraph &data, bool /* verbose */,
                                      bool /* withCache */,
                                      bool /* translateLabels */,
                                      bool /* checkLabels */,
                                      const SelectionSet * /* sel */ )
{
  Graph::iterator   iv, jv, ev = data.end();
  const MGraph      & mg = mGraph();
  Graph::const_iterator  imv, emv = mg.end();

  map<string, VertexClique *>    cliquesdata;
  VertexClique *                 cliqueslowscale = 0;
  map<string, VertexClique *>    cliquesintraPS;

  string            label, label1, mtype, subject;
  VertexClique           *vc;
  set<Clique *>               & scl = data.cliques();
  Domain            *fd;
  Vertex            *v;
  vector<string>         *pl;

  int nbcliqueSim=0;
  std::vector<float>         bbmin_1, bbmin_2, bbmax_1, bbmax_2, bc1, bc2;
  double rec;
  float tmin_1, tmin_2, tmax_1, tmax_2;
  int no_overlap=0;
  vector<int> vl;
  string			voidl = SIV_VOID_LABEL;

  _mgraph.getProperty( SIA_VOID_LABEL, voidl );
  
  mg.getProperty("label_list", vl);
  cout << "Label_list.size()=" << vl.size() << " / void : " << voidl << endl;
  uint i;
//   for (i=0; i<2; i++)
//       vl.push_back(i);

  cout << "Init cliques : label list = ";
  vector<int>::iterator ivl;
  for (ivl=vl.begin(); ivl!=vl.end(); ++ivl)
      cout << " " << (*ivl);
  cout << endl;
  
  
  
  // LECTURE DU MAILLAGE "ATLAS" ET CREATION DE LA STRUCTURE ALTERNATIVE
  
  AimsSurfaceTriangle mesh;
  string meshpath = "/home/grg/data/somato/ite1/V1/tri/V1_Lwhite.mesh";
  string latpath = "/home/grg/data/somato/ite1/V1/surface/V1_L_lat.tex";
  string longpath = "/home/grg/data/somato/ite1/V1/surface/V1_L_lon.tex";
//   meshpath = "/home/gregory/data/somato/ite1/V1/tri/V1_Lwhite.mesh";
//   latpath = "/home/gregory/data/somato/ite1/V1/surface/V1_L_lat.tex";
//   longpath = "/home/gregory/data/somato/ite1/V1/surface/V1_L_lon.tex";
  Reader<AimsSurfaceTriangle> r(meshpath);
  r.read(mesh);
  TimeTexture<float> lat;
  Reader<TimeTexture<float> > rlat(latpath);
  rlat.read(lat);
  TimeTexture<float> longit;
  Reader<TimeTexture<float> > rlongit(longpath);
  rlongit.read(longit);
  
  std::map<unsigned, std::set<unsigned> >    neighbours; 
  unsigned v1, v2, v3; 
 
  for( i=0; i<mesh.polygon().size(); ++i ) 
  { 
    v1 = mesh.polygon()[i][0]; 
    v2 = mesh.polygon()[i][1]; 
    v3 = mesh.polygon()[i][2]; 

    neighbours[v1].insert( v2 ); 
    neighbours[v2].insert( v1 ); 

    neighbours[v1].insert( v3 ); 
    neighbours[v3].insert( v1 ); 

    neighbours[v2].insert( v3 ); 
    neighbours[v3].insert( v2 ); 
  } 
  
  vector<std::map<uint,float> > distmap;
  std::map<uint,float>::iterator distit;
  
  uint ns=mesh.vertex().size();
  for (uint i=0;i<ns;i++){
    cout << "\b\b\b\b\b\b\b\b\b\b\b\b\b" << i << "/" << ns << flush;
    distmap.push_back(getDistMap(&mesh,&neighbours,i));
  }
  cout << endl;
  float x, y;
  float precisionX=10.0, precisionY=10.0;

  

     // DABORD STRUCTURE POUR REPRESENTER LE MAILLAGE EN FONCTION DES COORDONNEES
     // CECI POUR NE PAS PARCOURIR TOUTE LA LISTE DES POINTS DU MAILLAGE A CHAQUE
     // POINT DE L'ATLAS

  std::vector<std::set<uint> > polyVert(ns), polyVtmp(ns);
     
  std::cout << "Building alternate representation of input mesh" << endl;
  std::map<float, std::vector<std::pair<float, uint> > > mesh2;
  for (uint imesh=0;imesh<ns;imesh++)
  {
    
    x=lat[0].item(imesh);
    y=longit[0].item(imesh);
    mesh2[x].push_back(std::pair<float,uint>(y,imesh));
  }



  cout << "Computing all cliques. It might be long, be patient..." << endl;

  //ancien
  //mg.getAttribute("label_list", vl);
  for( imv=mg.begin(); imv!=emv; ++imv )
  {

      mtype = "";
      (*imv)->getProperty( SIA_MODEL_TYPE, mtype );

      // Old way
//       if ( mtype == "surfacebasedfunctionalsketch_similarity" )
//       {
//           vc = new VertexClique;
//           cliquessim = vc;
//           vc->setProperty( SIA_MODEL_TYPE,
//                             string( "functionalsketch_similarity" ) );
//           scl.insert( vc );
//       }
//       //else

      // end of old way

      if (mtype == "surfacebasedfunctionalsketch_lowerscalebest" )
      {
          vc = new VertexClique;
          cliqueslowscale = vc;
          vc->setProperty( SIA_MODEL_TYPE,
                           string( "surfacebasedfunctionalsketch_lowerscalebest") );
          scl.insert(  vc );
      }
  }

  

  for (iv=data.begin(); iv!=ev; ++iv)
  {    
      v = *iv;
      if( v->getProperty( SIA_POSSIBLE_LABELS, pl ) )
           delete pl;
      pl = new vector<string>;
      vector<string> pl2;
//       pl->push_back( voidl );
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

              mtype = "";
              (*imv)->getProperty( SIA_MODEL_TYPE, mtype );
              if (mtype == "surfacebasedfunctionalsketch_datadriven")
              {
                  v->getProperty( SIA_SUBJECT, subject );
                  VertexClique *& c = cliquesdata[ subject ];
                  if( c == 0 )
                  {
                      c = new VertexClique;
                      c->setProperty( SIA_MODEL_TYPE,
                                      string( "surfacebasedfunctionalsketch_datadriven" ) );
                      scl.insert( c );
                  }
                  c->addVertex( v );    // noeud dans la clique
              }
              else if (mtype == "surfacebasedfunctionalsketch_similarity")
              {
                  string subject1, subject2;
                  (*iv)->getProperty( SIA_SUBJECT, subject1 );
                  (*iv)->getProperty( "boundingbox_max", bbmax_1);
                  (*iv)->getProperty( "boundingbox_min", bbmin_1);
                  (*iv)->getProperty( "gravity_center", bc1);

                  (*iv)->getProperty( "tmin", tmin_1);
                  (*iv)->getProperty( "tmax", tmax_1);
                  
                  x=bc1[0];
                  y=bc1[1];
//                   cout << x << ";" << y << endl;
                  double dist, distMin=10000.0;
                  uint dep=0,arr=0;
                  float xbis, ybis;
                  int flagbis=0;
                  std::map<float, std::vector<std::pair<float, uint > > >::iterator meshIt=mesh2.begin();
                  std::vector<std::pair<float, uint> >::iterator yIt=((*meshIt).second).begin();
                  dep=(*yIt).second;
                  for ( ; (meshIt!=mesh2.end()) && (flagbis==0) ; ++meshIt)
                  {
                    xbis=(*meshIt).first;
                    if (xbis > (x+precisionX))
                      flagbis=1;
                    else if (xbis >= (x-precisionX))
                    {
                      yIt=((*meshIt).second).begin();
                      for ( ; yIt!=((*meshIt).second).end(); ++yIt)
                      {
                        ybis=(*yIt).first;
                        if ((ybis>=(y-precisionY)) && (ybis<=(y+precisionY)))
                        {
                          dist=(x-xbis)*(x-xbis) + (y-ybis)*(y-ybis);
                          if (dist<distMin)
                          {
                            distMin=dist;
                            dep=(*yIt).second;
                          }
                        }
                      }
                    }
                  }

                  for (jv=iv, ++jv; jv!=ev; ++jv)
                  {
//                     cout << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b" << jjj++ << "/" << data.order() << flush;
                    
                      (*jv)->getProperty( SIA_SUBJECT, subject2 );
                      if (subject1 != subject2)
                      {
//                         cout<< "TESTSIMILARITY" << endl;
//                           cout << subject2 << endl;
                          (*jv)->getProperty( "boundingbox_max", bbmax_2);
                          (*jv)->getProperty( "boundingbox_min", bbmin_2);
                          (*jv)->getProperty( "gravity_center", bc2);

                          (*jv)->getProperty( "tmin", tmin_2);
                          (*jv)->getProperty( "tmax", tmax_2);
                          no_overlap=0;
                          distMin=10000.0;
                          x=bc2[0];
                          y=bc2[1];
                          flagbis=0;
                          meshIt=mesh2.begin();
                          yIt = ((*meshIt).second).begin();
                          arr =(*yIt).second;
                            
                          for (; (meshIt!=mesh2.end()) && (flagbis==0) ; ++meshIt)
                          {
                            xbis=(*meshIt).first;
                            if (xbis > (x+precisionX))
                              flagbis=1;
                            else if (xbis >= (x-precisionX))
                            {
                              yIt=((*meshIt).second).begin();
                              for ( ; yIt!=((*meshIt).second).end(); ++yIt)
                              {
                                ybis=(*yIt).first;
                                if ((ybis>=(y-precisionY)) && (ybis<=(y+precisionY)))
                                {
                                  dist=(x-xbis)*(x-xbis) + (y-ybis)*(y-ybis);
                                  if (dist<distMin)
                                  {
                                    distMin=dist;
                                    arr=(*yIt).second;
                                  }
                                }
                              }
                            }
                          }
//                           cout << dep << ";" << arr << ";" << endl ;
                          distit = distmap[dep].find(arr);
                          if (distit != distmap[dep].end()) 
                            rec = (*distit).second;
                          else 
                            rec = 999.0;

//                           if (rec < 998.0) cout << rec << endl;
                          float simdis=10.0;

                          if (rec < simdis &&  !((tmin_2 > tmax_1) || (tmin_1 > tmax_2))){
                            vc = new VertexClique;
                            nbcliqueSim++;
                            vc->setProperty( SIA_MODEL_TYPE, string( "surfacebasedfunctionalsketch_similarity" ) );
                            vc->addVertex( (*iv) );
                            vc->addVertex( (*jv) );
                            vc->setProperty("overlap", rec);
                            scl.insert( vc );
                          }
                      }
                  }
              // end of new way
              }
              else if (mtype == "surfacebasedfunctionalsketch_lowerscalebest")
              {
                  cliqueslowscale->addVertex( v );     // noeud dans la clique
              }
              else if (mtype == "surfacebasedfunctionalsketch_intraprimalsketch")
              {
                  v->getProperty( SIA_SUBJECT, subject );
                  VertexClique *& c = cliquesintraPS[ subject ];
                  if( c == 0 )
                  {
                      c = new VertexClique;
                      c->setProperty( SIA_MODEL_TYPE,
                                      string( "surfacebasedfunctionalsketch_intraprimalsketch" ) );
                      scl.insert( c );
                  }
                  c->addVertex( v );    // noeud dans la clique
              }
//               else
//                   cerr << "problem : model type " << mtype << " unknown" << endl;
//           }
//           else
//           {
//               cout << "Test : on est PAS dans le domaine" << endl;
//           }          else
      }
  }

  cout << endl;

//                   fclose(overDebug);


  int     nsuj = cliquesdata.size();


  std::cout << "Found " << nsuj << " subjects" << endl;
  std::cout << "There is " << nbcliqueSim << " similarity cliques" << endl;
// 
  map<string, VertexClique *>::iterator  idc, edc = cliquesdata.end();
  for( idc=cliquesdata.begin(); idc!=edc; ++idc )
      idc->second->setProperty( "num_subjects", nsuj );
// 
  map<string, VertexClique *>::iterator iic, eic = cliquesintraPS.end();
  for( iic=cliquesintraPS.begin(); iic!=eic; ++iic )
      iic->second->setProperty( "num_subjects", nsuj );

  cliqueslowscale->setProperty( "num_subjects", nsuj );
}
