/*
 *  Copyright (C) 2001-2006 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 */

#include <aims/distancemap/meshparcellation.h>
#include <aims/distancemap/projection.h>
#include <aims/getopt/getopt2.h>
#include <aims/mesh/texture.h>
#include <aims/io/reader.h>
#include <aims/io/writer.h>
#include <aims/connectivity/meshcc.h>
#include <si/fold/labelsTranslator.h>
#include <aims/mesh/tex2graph.h>
#include <aims/graph/graphmanip.h>
#include <cfloat>

using namespace aims;
using namespace aims::meshdistance;
using namespace sigraph;
using namespace carto;
using namespace std;


int main( int argc, const char** argv )
{
  Reader<AimsSurfaceTriangle> triR;
  Reader<AimsSurfaceTriangle> triB;
  Reader<TimeTexture<short> > texR;
  Writer<Graph> agw2;
  Writer<TimeTexture<short> > texW;
  Writer<Volume<short> > imaW;
  string model;
  Reader<Volume<short> > triGV;
  float dist = FLT_MAX;
  map< set<short>,set<unsigned>,SetCompare<short> > label_sulci_Vert;
  bool connexity = false, graph=false;
  unsigned	time=0;
  string sulcitraductionfile = "sulcitraduction.txt";
  string gyritraductionfile = "gyritraduction.txt";
  int val_domain = 100;

  AimsApplication app( argc, argv, "Compute parcellation in sulci regions and "
    "gyri. The input sulci texture must be defined by siMeshSulciProjection" );

  app.addOption( triR, "-i", "grey/white mesh" );
  app.alias( "--input", "-i" );
  app.addOption( texR, "-s", "input_texture: definition of the sulci texture "
    "(given by siMeshSulciProjection" );
  app.alias( "--sulci", "-s" );
  app.addOption( agw2, "-g", "output gyri graph" );
  app.alias( "--graph", "-g" );
  app.addOption( model, "-m", "gyri model: Choice of the sulcus/sulcus "
    "relations ('gyri.gyr')" );
  app.alias( "--model", "-m" );
  app.addOption( texW, "-o", "output timetexture file (sulci and gyri "
    "texture): 0: sulci, 1 : sulci regions, 2: gyri, 3: gyri & sulci" );
  app.alias( "--output", "-o" );
  app.addOption( triB, "-b", "brain mesh [used and required in 3D mode only]",
                 true );
  app.alias( "--brain", "-b" );
  app.addOption( imaW, "-p", "output gyri volume [used in 3D mode only]",
                 true );
  app.alias( "--parcelvol", "-p" );
  app.addOption( val_domain, "-v", "grey value in input_grey_white vol "
    "[default: 100]", true );
  app.alias( "--value", "-v" );
  app.addOption( triGV, "-V", "input grey white volume [used and required in "
    "3D mode only]", true );
  app.alias( "--Volume", "-V" );
  app.addOption( time, "-T", "input sulci texture time [default: 0]", true );
  app.alias( "--Time", "-T" );
  app.addOption( sulcitraductionfile, "--sulcitranslation", "sulci translation "
    "file: correspondance label string->short_label "
    "[default:\"sulcitraduction.txt\"]", true );
  app.alias( "--sulcitraduction", "--sulcitranslation" );
  app.addOption( gyritraductionfile, "--gyritranslation", "output gyri "
    "translation file: correspondance label string->short_label. This file "
    "will be written or completed if it already exists "
    "[default:\"gyritraduction.txt\"]", true );
  app.alias( "--gyritraduction", "--gyritranslation" );
  app.addOption( connexity, "--connectivity", "use connectivity (true) or "
    "geodesic euclidean distance (false) [default: false]", true );
  app.alias( "--connexity", "--connectivity" );
  app.addOption( graph, "--3D", "compute 3D cortical ribbon gyrus graph "
    "[default: false]", true );

  try
  {
    app.initialize();
  }
  catch( user_interruption & )
  {
    return 0;
  }
  catch( exception & e )
  {
    cerr << e.what() << endl;
    return 1;
  }

  //
  // read triangulation
  //
  cout << "reading white triangulation   : " << flush;
  AimsSurfaceTriangle surface;
  triR >> surface;
  cout << "done" << endl;
 
  //
  // read input texture
  //
  cout << "reading texture   : " << flush;
  TimeTexture<short>	inpTex; // objects def (labels >0)
  texR >> inpTex; 
  cout << "done" << endl;

  cout << "mesh vertices : " << surface[0].vertex().size() << endl;
  cout << "mesh polygons : " << surface[0].polygon().size() << endl;
  cout << "Object texture dim   : " << inpTex[0].nItem() << endl;
  
  if ( ! (  inpTex[0].nItem() ==  surface[0].vertex().size() ) )
    {
      cerr << "The triangulation and the textures must correspond to the same object \n";
      assert( 0 );
    }
  
  // read translation file
  cout << "Read translation file \n";
  ifstream 	tf(sulcitraductionfile.c_str());
  if (!tf)
    {
      cout << "File " << (string)sulcitraductionfile << " missing. Please first use siMeshSulciProjection...\n";
      assert(0);
    }
  string 	label;
  short		l;
  //short inc=0;
  map<string,short>	trans; // string ->  short 
  map<short,string>	trans_inv;
  while ( tf && !tf.eof() )
    {
      tf >> label >> l;
      if (!tf.eof() && (!label.empty() || label != "unknown" ) )	
	{
	  trans[label] = l;
	  trans_inv[l]=label;
	}
    }

  // read sulco-gyral relation file 
  cout << "Read gyri model file \n";
  set<set<short> > AllowedLabel;
  map<string,set<string> > GyriAndSulci;
  GyriAndSulci = GyrusModel2GyriAndSulci(model.c_str());
  AllowedLabel = GyrusModel2SetOfSetOfSulci(GyriAndSulci,trans);
 

  // sulci estimated with the linear projection 
  unsigned i , n=inpTex[0].nItem();
  TimeTexture<short>	outTex(5,n);  
  
  for (i=0; i<n; ++i)
    outTex[0].item(i)= inpTex[time].item(i) ;
  
  cout << "Extract sulcal regions" << endl; 
  set<short>            	sulci_lab,settemp;
  //  map<short,set<unsigned> >	lab_vox;
 
  // sulci_lab contains the labels of the projections on the triangulation 
  // of the edges(simple surfaces of the skeleton) of the sulci graphe
  // !! label 0 and -1 are reserved for the baground and the MESHDISTANCE_FORBIDDEN  
  for (i=0;i<n;++i)
    sulci_lab.insert(outTex[0].item(i));
  sulci_lab.erase(0);
  //cout << "Label correspondance:\n";
  //for (isl=sulci_lab.begin(), esl=sulci_lab.end(); isl != esl;++isl)
  //if (*isl != 0)
  // cout << trans_inv[*isl] << "-> " << *isl << endl ;
 
  cout << "1st voronoi diagram\n";
  outTex[1] = outTex[0];
  set<short>			       	setBack,setFor;
  setBack.insert(0);
  setFor.insert(MESHDISTANCE_FORBIDDEN);
  outTex[2] = MeshVoronoiT<short>( surface[0], outTex[1], 0 ,MESHDISTANCE_FORBIDDEN, dist, connexity,true );

 
  //Extraction of the frontiers of the first vornoi diagram
  //The label of the frontier is a set of the labels (of the seeds)
  //of the  neigbouring labelled area  
  Texture<set<short> > TexFrontSet,TexVor2Set,TexVor2Set2,SKIZ; 
  cout << "Extracting the boundaries of the sulci regions and select the gyrus seeds" << endl;
  SKIZ = MeshBorderVoronoi<short>(surface[0],outTex[2],setBack,setFor);
  TexFrontSet = gyrusSeedDefinition<short>(surface[0],outTex[2],setBack,setFor,AllowedLabel);
  //cout << "Dilate the gyrus seed in the SKIZ \n";
  //TexFrontSet = gyrusSeedDilationInSKIZ<std::set<short> >(surface[0],TexFrontSet,SKIZ,setBack,setFor);
  outTex[3] = border2Texture<short>( TexFrontSet , surface[0],setBack,setFor);

  
  //2nd voronoi
  //-> seeds are the labelled frontiers
  //-> sulci belong no more to the object
  cout << "Gyral parcellation \n";
  for (i=0;i<n;++i)
    if (outTex[0].item(i) > 0 )
      TexFrontSet.item(i) = setFor;
  TexVor2Set = MeshVoronoiT<set<short> >(surface[0], TexFrontSet,setBack,setFor,dist,connexity,true);
  for (i=0;i<n;++i)
    if (TexVor2Set.item(i) == setFor)
      TexVor2Set.item(i) = setBack;

  //Boundariy thinning 
  //Just a dilation to remove the boundaries but not the insula
  TexVor2Set2 = MeshVoronoiT<set<short> >( surface[0],TexVor2Set,setBack,setFor, 1 ,true ,true);
  outTex[5] = Voronoi2toTexture<short,short>( TexVor2Set2,surface[0],setBack,setFor );  
  outTex[4] = outTex[5];
  for (i=0;i<n;++i)
    if (outTex[0].item(i) != 0)
      outTex[5].item(i) = 0;

  
  // Writting the textures
  cout << "writing texture : " << flush;
  TimeTexture<short> otex;
  otex[0] = outTex[0];
  otex[1] = outTex[2];
  otex[2] = outTex[4];
  otex[3] = outTex[5];
  otex[4] =  outTex[3];
  texW <<  otex ;
  cout << "done" << endl;


  
  //Surfacic gyrus graph
  cout << "Compute surfacic gyral parcellation graph\n";
  Graph		g("RoiArg");
  Tex2Graph<set<short> >	t2g;
  Tex2Graph<short> t2g2;
  map<short,string> GyriLabel2GyriName;
  map<string,set<string> >::iterator igs,egs;
  set<string>::iterator ist,est;
  set<short> setTemp;
  map<set<short>,string> setSulciLabel2GyriName;

  trans_inv.insert(pair<short,string>(0,"background"));
  for ( igs = GyriAndSulci.begin(), egs = GyriAndSulci.end(); igs != egs; ++igs)   
    { 
      setTemp.clear();
      for ( ist = (igs->second).begin(), est = (igs->second).end(); ist != est; ++ist )
        setTemp.insert(trans[*ist]);
      setSulciLabel2GyriName[setTemp] = igs->first;
    }
  for (i=0;i<n;++i)
    GyriLabel2GyriName[outTex[4].item(i)] = setSulciLabel2GyriName[TexVor2Set2.item(i)] ; 
  GyriLabel2GyriName[0] = "background";
  setTemp.clear();
  setTemp.insert(0);
  setSulciLabel2GyriName[setTemp] = "background";



  //Writing map betwen gyri label and gyr name
  
  std::map<short,string>::iterator iG,eG;
  ofstream	namefile( gyritraductionfile.c_str() );
  for( iG = GyriLabel2GyriName.begin(), eG = GyriLabel2GyriName.end() ; iG !=eG ; ++iG )
    namefile << iG->first << "\t" << iG->second << endl;
  cout << "Write " << gyritraductionfile << " file\n";
    
  

  t2g.makeGraph(g,surface,TexVor2Set2,setSulciLabel2GyriName);
  GraphManip::completeGraph(g); //compute mesh area for each surface patch

  Graph::iterator igr,egr;
  string name;
  std::map<string,float>::iterator ig, eg;


  //3D gyri parcellation 
  if (graph)
    {
      if( triB.fileName().empty() )
        throw runtime_error( "--brain option is required in 3D mode" );
      if( triGV.fileName().empty() )
        throw runtime_error( "--Volume option is required in 3D mode" );

      TimeTexture<short> braintex;

      cout << "Reading cortex volume   : " << triGV.fileName() << endl;
      VolumeRef<short> greyVol( triGV.read() );

      cout << "reading brain triangulation   : " << flush;
      AimsSurfaceTriangle brain;
      triB >> brain;
      cout << "done" << endl;

      cout << "Computing 3D parcellisation" << endl;
      Graph		k("RoiArg");
      VolumeRef<short>    gyriVol;
      gyriVol = MeshParcellation2Volume( greyVol,otex[2],surface[0],(short)val_domain,0 );
      braintex[0] = VolumeParcellation2MeshParcellation(gyriVol,brain[0],0);
      t2g2.makeGraph(k,brain,braintex[0],GyriLabel2GyriName);

      cout << "Computing the volumic gyrus graph " << agw2.fileName() << "\n";
      Graph		*h = GraphManip::graphFromVolume(gyriVol,(short)0, &GyriLabel2GyriName);
      GraphManip::volume2Buckets(*h);

      //Compute volume of the gyri
      std::map<string,float> gyrusVolume;
      eg = gyrusVolume.end();
      map <short, float>     stat_gyri_vol;
      map <short,float>::iterator istf,estf;
      // <short> is needed for auto conversion VolumeRef -> AimsData, otherwise
      // the compiler can't find a conversion.
      stat_gyri_vol = VolumeParcel<short>(gyriVol);
      for (istf = stat_gyri_vol.begin(), estf=stat_gyri_vol.end();
           istf != estf; ++istf)
        if ( istf->first != val_domain )
          gyrusVolume[ GyriLabel2GyriName[istf->first]  ] = istf->second; 
      for (igr = h->begin(), egr = h->end(); igr != egr; ++igr)
        {
          if ( (*igr)->getProperty("name",name) ) 
            {
              ig = gyrusVolume.find(name);
              if (ig != eg)
                (*igr)->setProperty("size", gyrusVolume[name] );
              else 
                cerr << "Could not find the gyrus bucket with the name " << name << endl; 
            }
          else
            cerr << "Could not find the name of a gyrus \n " ; 

        }

      string key = "name";
      Graph *m = GraphManip::mergeGraph( key,g,k,true,false );
      m = GraphManip::mergeGraph(key,*m,*h);
      m->setProperty( "filename_base", "*" );

      cout << "Write cortex ribbon gyri graph \n";
      try
        {
          agw2.write( *m );
        }
      catch( exception & e )
        {
          cerr << e.what() << endl;
          throw( e );
        } 

      if( !imaW.fileName().empty() )
      {
        cout << "Write gyri volume\n";
        imaW.write( *gyriVol );
      }

    }
  else
    {
      try
        {
            Graph *m;
            m = &g;
            m->setProperty( "filename_base", "*" );
            agw2.write( *m );
        }
      catch( exception & e )
        {
          cerr << e.what() << endl;
          throw( e );
        }     

    }

  return( 0 );  
}

 
