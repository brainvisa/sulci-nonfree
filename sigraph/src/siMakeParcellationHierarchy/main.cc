/*
 *  Copyright (C) 2001-2004 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 */

#include <aims/distancemap/meshparcellation.h>
#include <aims/distancemap/meshdistance.h>
#include <aims/getopt/getopt.h>
#include <aims/vector/vector.h>
#include <si/fold/labelsTranslator.h>
#include <si/global/global.h>
#include <si/graph/attrib.h>
#include <si/graph/attrib.h>
#include <aims/def/path.h>
#include <graph/tree/tree.h>
#include <aims/io/writer.h>
#include <aims/io/reader.h>
#include <cartobase/object/sreader.h>
#include <graph/tree/twriter.h>
#include <cartobase/stream/sstream.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdlib.h>

using namespace aims;
using namespace aims::meshdistance;
using namespace sigraph;
using namespace carto;
using namespace std;
 
typedef float float3[3];

BEGIN_USAGE(usage)
  "-------------------------------------------------------------------------",
  "siMakeParcellationHierarchy   -g[yrus] <output gyrus hierarchy>          ",
  "                              -s[ulcus]<output sulcus hierarchy >        ",
  "                              -m[odel] <intput gyri model >              ",
  "                              -l[evel] <level model>		            ",
  "                              [-h[elp]]                                  ",
  "-------------------------------------------------------------------------",
  " Make sulci and gyri hierarchy for parcellation                          ",
  "-------------------------------------------------------------------------",
  "	gyri model          : Choice of the sulcus/sulcus relations ('*.gyr')",
  "     level model         : Choice of the level of description file ('*.trl')  ",
   "-------------------------------------------------------------------------",
END_USAGE

  
//
// Usage
//
void Usage( void )
{
  AimsUsage( usage );
}


int main( int argc, char** argv )
{
  char	 *gyrifile = 0, *model=0, *level = 0, *sulcifile;
  

  //
  // Parser of options
  //
  AimsOption opt[] = {
    { 'h',"help"         ,AIMS_OPT_FLAG  ,( void* )Usage           ,AIMS_OPT_CALLFUNC,0},
    { 'g',"gyri"       ,AIMS_OPT_STRING,&gyrifile     ,0                ,1},
    { 's',"sulci"       ,AIMS_OPT_STRING,&sulcifile     ,0                ,1},
    { 'l',"level"        ,AIMS_OPT_STRING,&level          ,0                ,1},
    { 'm',"model"        ,AIMS_OPT_STRING,&model          ,0                ,1},
    { 0  ,0              ,AIMS_OPT_END   ,0               ,0                ,1}};

  AimsParseOptions( &argc, argv, opt, usage );

  
  // read sulco-gyral relation file 
  cout << "Read gyri model file \n";
  map<string,set<string> > GyriAndSulci;
  map<string,set<string> >::iterator ig,eg;
  set<string>              gyris;
  set<string>::iterator   is,es,iss,ess;  
  string gyrus;
  GyriAndSulci = GyrusModel2GyriAndSulci(model);
  int r,v,b;

  Tree gyriH(true,"hierarchy");
  gyriH.setProperty("graph_syntax",string("RoiArg"));

  Tree *L =  new Tree(true,"fold_name") ;
  L->setProperty("name", string("left_hemishpere"));
  Tree *R =  new Tree(true,"fold_name") ;
  R->setProperty("name", string("right_hemishpere"));
  Tree *B =  new Tree(true,"fold_name") ;
  B->setProperty("name", string("brain"));
  Tree *ba =  new Tree(true,"fold_name") ;
  ba->setProperty("name", string("background"));
  Tree *un =  new Tree(true,"fold_name") ;
  un->setProperty("name", string("unknown"));
  vector<int> vec;
  vec.push_back(256);
  vec.push_back(180);
  vec.push_back(180);
  un->setProperty("color", vec);
  vec.clear();
  vec.push_back(0);
  vec.push_back(0);
  vec.push_back(0);
  ba->setProperty("color", vec);

  for (ig = GyriAndSulci.begin(), eg = GyriAndSulci.end(); ig != eg; ++ig )
    {
      gyrus = ig->first  ;
      string::size_type pos = gyrus.rfind((string)"_left");
      if ( pos != string::npos )
	{
	  gyrus = gyrus.erase(pos,gyrus.length());
	  gyris.insert(gyrus);
	}
      
    }
  for ( is=gyris.begin(), es = gyris.end(); is != es; ++is )
    {
      Tree *t =  new Tree(true,"fold_name") , *t2 =  new Tree(true,"fold_name");
      vector<int> vec;
      gyrus = *is  ;
      r=1+(int) (255.0*rand()/(RAND_MAX+1.0));
      v=1+(int) (255.0*rand()/(RAND_MAX+1.0));
      b=1+(int) (255.0*rand()/(RAND_MAX+1.0));
      vec.push_back(r);
      vec.push_back(v);
      vec.push_back(b);
      t->setProperty("name", gyrus + string("_left") );
      t->setProperty("color", vec);
      L->insert(t);
      t2->setProperty("name", gyrus + string("_right") );
      t2->setProperty("color", vec);
      R->insert(t2);	
    }
  B->insert(L);
  B->insert(R);
  gyriH.insert(B);
  gyriH.insert(ba);

  try
    {
      SyntaxSet	ss;
      SyntaxReader	sr( Path::singleton().syntax() + "/hierarchy.stx" );
      sr >> ss;
      TreeWriter tw(gyrifile,ss);
      tw << gyriH;
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
      throw;
    }

    
  
  FoldLabelsTranslator				levelTrans( level );
  map<string, string>::const_iterator		il, el = levelTrans.end();
  map<string, set<string> >  			oldSulci2newSulci;
  string                                         sulcusOld,sulcusNew;
  set<string>                                   sulcis;
  set<string>::iterator                         se = sulcis.end();

  for (ig = GyriAndSulci.begin(), eg = GyriAndSulci.end(); ig != eg; ++ig )
    for ( is = (ig->second).begin(), es = (ig->second).end(); is != es; ++is )
      {    
	sulcusNew = *is;
	string::size_type pos = sulcusNew.rfind(string("_left"));
	if ( pos != string::npos )
	  {
	    sulcusNew = sulcusNew.erase(pos,sulcusNew.length());
	    sulcis.insert(sulcusNew);
	  }
      }

  for (il = levelTrans.begin(); il != el; ++il   )
    {
      sulcusOld = il->first;
      sulcusNew = il->second;
      string::size_type pos = sulcusOld.rfind(string("_left"));
      if ( pos != string::npos )
	{
	  sulcusOld = sulcusOld.erase(pos,sulcusOld.length());
	  string::size_type pos = sulcusNew.rfind(string("_left"));
	  sulcusNew = sulcusNew.erase(pos,sulcusNew.length());
	  if (sulcis.find(sulcusNew) != se)
	    if ( sulcusOld != sulcusNew )
	      oldSulci2newSulci[sulcusNew].insert(sulcusOld);
	}
    }

  Tree sulciH(true,"hierarchy");
  sulciH.setProperty("graph_syntax",string("CorticalFoldArg"));
  Tree *sL =  new Tree(true,"fold_name") ;
  sL->setProperty("name", string("left_hemishpere"));
  Tree *sR =  new Tree(true,"fold_name") ;
  sR->setProperty("name", string("right_hemishpere"));
  Tree *sB =  new Tree(true,"fold_name") ;
  sB->setProperty("name", string("brain"));
  Tree *trouve_un_autre_nom =  new Tree(true,"fold_name") ;
  trouve_un_autre_nom->setProperty("name", string("unknown"));
  vec.clear();
  vec.push_back(256);
  vec.push_back(180);
  vec.push_back(180);
  trouve_un_autre_nom->setProperty("color", vec);
  for ( is= sulcis.begin(), es = sulcis.end(); is != es; ++is )
    {
      vector<int> vec;
      r=1+(int) (255.0*rand()/(RAND_MAX+1.0));
      v=1+(int) (255.0*rand()/(RAND_MAX+1.0));
      b=1+(int) (255.0*rand()/(RAND_MAX+1.0));
      vec.push_back(r);
      vec.push_back(v);
      vec.push_back(b);
      sulcusNew = *is ;
      Tree *t =  new Tree(true,"fold_name");
      t->setProperty("name", sulcusNew + string("_left") );
      t->setProperty("color", vec);
      for ( iss = oldSulci2newSulci[sulcusNew].begin(), ess = oldSulci2newSulci[sulcusNew].end(); iss != ess; ++iss )
	{ 
	  Tree *t2 =  new Tree(true,"fold_name");
	  t2->setProperty("name", *iss + string("_left") );
	  //t2->setProperty("color", vec);
	  t->insert(t2);
	}
      sL->insert(t);
      sulcusNew = *is;
      Tree *u =  new Tree(true,"fold_name");
      u->setProperty("name", sulcusNew + string("_right") );
      u->setProperty("color", vec);

      for ( iss = oldSulci2newSulci[sulcusNew].begin(), ess = oldSulci2newSulci[sulcusNew].end(); iss != ess; ++iss )
	{ 
	  Tree *t2 =  new Tree(true,"fold_name");
	  t2->setProperty("name", *iss + string("_right") );
	  //t2->setProperty("color", vec);
	  u->insert(t2);
	}
      sR->insert(u);

    } 
  
  sB->insert(sR);
  sB->insert(sL);
  sulciH.insert(sB);
  sulciH.insert(trouve_un_autre_nom);

  //Description of the model
  map<string,set<string> >::iterator isub,isue;
   for (isub = oldSulci2newSulci.begin(),isue = oldSulci2newSulci.end() ; isub != isue; ++isub )
    {
      cout << "paragraph{"
	   << isub->first << "}  : "  
	   << isub->first << " , " ;
      for (is = (isub->second).begin(),es = (isub->second).end() ; is != es ; ++is  )
	cout << *is << " , " ;
      cout << endl;
    }
  
  try
    {
      SyntaxSet	ss;
      SyntaxReader	sr( Path::singleton().syntax() + "/hierarchy.stx" );
      sr >> ss;
      TreeWriter tw2(sulcifile,ss);
      tw2 << sulciH;
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
      throw;
    }
}
