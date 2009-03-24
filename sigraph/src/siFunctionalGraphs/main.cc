/*
 *  Copyright (C) 1998-2004 CEA
 *
 *  This software and supporting documentation were developed by
 *
 *
 *   CEA/DSV/SHFJ
 *   4 place du General Leclerc
 *   91401 Orsay cedex
 *   France
 */

#include <cstdlib>
#include <aims/getopt/getopt2.h>
#include <aims/io/reader.h>
#include <aims/io/writer.h>
#include <aims/io/process.h>
#include <aims/io/finder.h>
#include <cartobase/object/sreader.h>
#include <si/global/global.h>
#include <si/fold/fgraph.h>
#include <si/fold/frgraph.h>
#include <si/fold/foldReader.h>
#include <si/fold/foldWriter.h>
#include <si/fold/frgReader.h>
#include <si/graph/anneal.h>
#include <si/finder/modelFinder.h>
#include <si/graph/attrib.h>
#include <si/fold/annealConnectExtension.h>
#include <si/graph/annealConfigurator.h>
#include <si/graph/multigraph.h>
#include <aims/mesh/surfaceOperation.h>
#include <aims/mesh/surfacegen.h>
#include <cartobase/exception/parse.h>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#if defined(__linux)
#include <fpu_control.h>
#elif defined(__sun) || defined(__sgi)
#include <ieeefp.h>
#endif

using namespace aims;
using namespace carto;
using namespace sigraph;
using namespace std;

class Site{
  public :
    int index;
    int graph_index;
    string subject;
    int label;
    float tValue;
    float t;
    float tmin;
    float tmax;
    float trep;
    Point3df gravitycenter;
    uint node;
};

int main(int argc, const char **argv)
{
     std::string fileGraphs,outFile;

     AimsApplication     app( argc, argv, "Analyse les resultats d'un protocole fonctionnel a partir du fichier de config utilise pour etiqueter les graphes");

     app.addOption( fileGraphs, "-i", "config.tree");
     app.addOption( outFile, "-o", "test.hie");
     app.alias( "--input", "-i" );
     app.alias( "--output", "-o" );

     try
     {
       app.initialize();
     }
     catch( user_interruption & )
     {
       return( EXIT_FAILURE );
     }
     catch( exception & e )
     {
       cerr << e.what() << endl;
       return( EXIT_FAILURE );
     }

     SyntaxReader pr( si().basePath() + "/config/siRelax.stx" );
     carto::SyntaxSet       ps;
     pr.read( ps );

     TreeReader   tr( fileGraphs, ps );
     Tree         t;
     tr >> t;

     std::string          graphFile;
     std::vector<std::string>  graphFiles;
     t.getProperty( "graphFile", graphFile );

     string::size_type pos = graphFile.find( '|' );
     string::size_type pos2 = 0;

     // color map definition for blobs with positive label
     std::vector< std::vector<int> > colors(40);
     for (int i=0; i<=40; i++)
          colors[i]=std::vector<int>(3);
     colors[0][0]=30; colors[0][1]=20; colors[0][2]=24;
     colors[1][0]=0; colors[1][1]=255; colors[1][2]=255;
     colors[2][0]=255; colors[2][1]=255; colors[2][2]=0;
     colors[3][0]=255; colors[3][1]=0; colors[3][2]=255;
     colors[4][0]=0; colors[4][1]=60; colors[4][2]=255;
     colors[5][0]=0; colors[5][1]=255; colors[5][2]=60;
     colors[6][0]=255; colors[6][1]=60; colors[6][2]=0;
     colors[7][0]=60; colors[7][1]=255; colors[7][2]=0;
     colors[8][0]=255; colors[8][1]=0; colors[8][2]=60;
     colors[9][0]=60; colors[9][1]=0; colors[9][2]=255;
     colors[10][0]=127; colors[10][1]=127; colors[10][2]=0;
     colors[11][0]=0; colors[11][1]=127; colors[11][2]=127;
     colors[12][0]=127; colors[12][1]=0; colors[12][2]=127;
     colors[13][0]=150; colors[13][1]=170; colors[13][2]=20;
     colors[14][0]=150; colors[14][1]=20; colors[14][2]=170;
     colors[15][0]=170; colors[15][1]=150; colors[15][2]=20;
     colors[16][0]=170; colors[16][1]=20; colors[16][2]=150;
     colors[17][0]=20; colors[17][1]=150; colors[17][2]=170;
     colors[18][0]=20; colors[18][1]=170; colors[18][2]=150;
     colors[19][0]=20; colors[19][1]=70; colors[19][2]=10;
     colors[20][0]=255; colors[20][1]=70; colors[20][2]=20;
     colors[21][0]=0; colors[21][1]=10; colors[21][2]=10;
     colors[22][0]=10; colors[22][1]=125; colors[22][2]=0;
     colors[23][0]=10; colors[23][1]=0; colors[23][2]=10;
     colors[24][0]=0; colors[24][1]=60; colors[24][2]=10;
     colors[25][0]=0; colors[25][1]=125; colors[25][2]=60;
     colors[26][0]=125; colors[26][1]=60; colors[26][2]=0;
     colors[27][0]=60; colors[27][1]=125; colors[27][2]=0;
     colors[28][0]=10; colors[28][1]=0; colors[28][2]=60;
     colors[29][0]=60; colors[29][1]=0; colors[29][2]=10;
     colors[30][0]=30; colors[30][1]=20; colors[30][2]=24;
     colors[31][0]=0; colors[31][1]=125; colors[31][2]=200;
     colors[32][0]=200; colors[32][1]=200; colors[32][2]=0;
     colors[33][0]=200; colors[33][1]=0; colors[33][2]=200;
     colors[34][0]=0; colors[34][1]=60; colors[34][2]=200;
     colors[35][0]=0; colors[35][1]=200; colors[35][2]=60;
     colors[36][0]=200; colors[36][1]=60; colors[36][2]=0;
     colors[37][0]=60; colors[37][1]=200; colors[37][2]=0;
     colors[38][0]=200; colors[38][1]=0; colors[38][2]=60;
     colors[39][0]=60; colors[39][1]=0; colors[39][2]=200;     


     while( pos != string::npos )
     {
          graphFiles.push_back( graphFile.substr( pos2, pos - pos2 ) );
          pos2 = pos + 1;
          pos = graphFile.find( '|', pos2 );
     }
     graphFiles.push_back( graphFile.substr( pos2, graphFile.length() - pos2 ) );

     std::vector<std::string>:: iterator itG=graphFiles.begin(), itGe=graphFiles.end();
     std::map<string, std::set<string> > mapLabel;

     vector<Site> sites;
     
     cout << "Going through labelled graphs..." << endl;
     for ( ; itG!=itGe; ++itG)
     {
          string oneG=(*itG), subject;
          Graph gFunc;

          aims::Reader<Graph > funcR(oneG);
          funcR.read(gFunc);
          Graph gFunc2;
          funcR.read(gFunc2);          
          string oneG2(oneG + "test.arg");          
          gFunc.getProperty("sujet", subject);
          
          std::set<Vertex *> nodes=gFunc.vertices(),nodes2=gFunc2.vertices();
          std::set<Vertex *>::iterator itN=nodes.begin();
          std::set<Vertex *>::iterator itN2=nodes2.begin();
          Vertex *node;
          string label,index;
          float trep;
          AimsSurfaceTriangle nodemesh = *(new AimsSurfaceTriangle);

          // je parcours les noeuds
          // si le noeud courant a pour topbif DISAPPEAR alors on lance la fonction de traitement avec chacun de ses éventuels blobs de bottombif + une abscisse.
          set<Vertex*> neigh;
          set<Vertex*>::iterator neighit;
          set<Edge*> edges;
          set<Edge*>::iterator edgesit;

          vector<float> bc1, bc2;
          float tmin_1, tmax_1, tvalue1;
          string subject1;          
          string type;
//           for ( ; itN!=nodes.end(); ++itN, ++itN2){
//             node =*itN;
//             node->getProperty("index", index);
// 
//             neigh = node->neighbours();
//             cout << neigh.size() << endl;
//             for (neighit = neigh.begin();neighit != neigh.end() ;neighit++){
              
//             }
//             for (edgesit = node->begin();edgesit!=node->end();edgesit++){
//               (*edgesit)->getProperty("type",type);
//               cout << type << endl;
//               if (type == "APPEAR"){
//                 (*edgesit)->getProperty("blob_up",label);
//                 (*edgesit)->getProperty("blob_down",type);
//                 cout << index << " " << label << " " << type << endl;
//               }
//             }
//             
//           }
          
          for ( itN=nodes.begin(); itN!=nodes.end(); ++itN, ++itN2)
          {
               node=(*itN);
               node->getProperty("label", label);
               node->getProperty("trep", trep);
               if (label != "0"){
                    mapLabel[label].insert(subject);
                    (*itN)->getProperty( "subject", subject1 );
                    (*itN)->getProperty( "gravity_center", bc1);
                    (*itN)->getProperty( "tmin", tmin_1);
                    (*itN)->getProperty( "tmax", tmax_1);
//                (*itN)->getProperty( "trep", trep);
                    (*itN)->getProperty( "t", tvalue1);
                    Site s;
                    s.tValue = tvalue1;
                    s.gravitycenter = Point3df(bc1[0],bc1[1],bc1[2]);
                    s.label = atoi(label.data());
                    s.subject = subject1;
                    sites.push_back(s);
               }
               node->setProperty("name", label);

          }

          aims::Writer<Graph> funcW(oneG);
          funcW.write(gFunc);
//           aims::Writer<Graph> funcW2(oneG2);
//           funcW2.write(gFunc2);
     }
     
     cout << "There was " << mapLabel.size() << " non null labels";
     cout << "Hierarchy being generated. Report :" << endl;
     std::map<string, std::set<string> >::iterator itLabel;
     std::set<string> setSubj;
     std::set<string>::iterator itSubj;

     FILE *fileH;
     fileH=fopen(outFile.data(),"w");
     fprintf(fileH, "*BEGIN TREE 1.0 hierarchy\n");
     fprintf(fileH, "graph_syntax PrimalSketchArg\n\n");
     fprintf(fileH, "*BEGIN TREE bloblabel_name\n");
     fprintf(fileH, "name functional_blobs\n\n");
     cout << "mouf"<< endl; 
//      Tree    *treeB = new Tree( true, "functional_blobs" );
//      Hierarchy    *hie = new Hierarchy( treeB) ;
//
//      hie->attributed()->setProperty( "graph_syntax", "PrimalSketchArg" );
     int lindex=0;
     for (itLabel=mapLabel.begin(); itLabel!=mapLabel.end(); ++itLabel)
     {
//           Tree * newT = new Tree(true,"bloblabel_name");
//           static_cast <Tree*> ( hie->attributed() )-> insert(newT) ;
//           newT->setProperty("name", (*itLabel).first);
//           newT->setProperty("label", (*itLabel).first);

          fprintf(fileH, "*BEGIN TREE bloblabel_name\n");
          fprintf(fileH, "name\t%s\n", (*itLabel).first.c_str());
          fprintf(fileH, "color\t%i %i %i\n", colors[lindex][0], colors[lindex][1], colors[lindex][2]);
          fprintf(fileH, "label\t%s\n", (*itLabel).first.c_str());
          for (uint i=0;i<sites.size();i++){
            if (sites[i].label == atoi((*itLabel).first.c_str())) {
              fprintf(fileH, "blob\t%s %f-%f %f\n", sites[i].subject.data(), sites[i].tValue, sites[i].gravitycenter[0], sites[i].gravitycenter[1]);
            }
          }
          fprintf(fileH, "*END\n\n");
          lindex++;
          cout << "Label " << (*itLabel).first << " :\t";
          setSubj=(*itLabel).second;
          for (itSubj=setSubj.begin(); itSubj!=setSubj.end(); ++itSubj)
               cout << (*itSubj) << "\t";
          cout << endl;
     }
     fprintf(fileH, "*END\n\n");
     fprintf(fileH, "*END\n");
     fclose(fileH);

     exit(0);
}

