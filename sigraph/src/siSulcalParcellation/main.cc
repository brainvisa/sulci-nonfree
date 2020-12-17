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
#include <aims/distancemap/meshdistance.h>
#include <aims/distancemap/projection.h>
#include <aims/data/data.h>
#include <aims/math/math_g.h>
#include <aims/getopt/getopt2.h>
#include <aims/mesh/texture.h>
#include <aims/io/reader.h>
#include <aims/io/writer.h>
#include <aims/connectivity/meshcc.h>
#include <si/fold/labelsTranslator.h>
#include <si/global/global.h>
#include <aims/mesh/tex2graph_d.h>
#include <aims/io/aimsGraphW.h>
#include <aims/def/path.h>
#include <graph/graph/graph.h>
#include <graph/graph/gwriter.h>
#include <graph/graph/greader.h>
#include <cartobase/object/sreader.h>
#include <si/fold/foldReader.h>
#include <aims/graph/graphmanip.h>
#include <cartobase/plugin/plugin.h>

using namespace aims;
using namespace aims::meshdistance;
using namespace sigraph;
using namespace carto;
using namespace std;


int main( int argc, const char** argv )
{
  Reader<AimsSurfaceTriangle> triR;
  Reader<AimsSurfaceTriangle> triB;
  Writer<TimeTexture<short> > texW;
  Writer<Graph>	agw2;
  Reader<TimeTexture<short> > texR;
  Writer<AimsData<short> > imaW;
  Reader<AimsData<short> > triGV;

  string model;
  float dist = FLT_MAX;
  map< set<short>,set<unsigned>,SetCompare<short> > 		label_sulci_Vert;
  bool connexity = false, graph=false;
  unsigned	time=0;
  string sulcitraductionfile = "sulcitraduction.txt";
  int val_domain = 100;

  AimsApplication app( argc, argv,
                       "Compute parcellation in sulcal regions. "
                       "The input sulci texture must be defined by "
                       "siMeshSulciProjection" );
  app.addOption( triR, "-i", "grey/white mesh file" );
  app.addOption( texW, "-o",
                 "gyri texture: output timetexture file (sulci and gyri "
                 "texture), 0: sulci, 1: sulcal regions" );
  app.addOption( agw2, "-g", "output gyri graph" );
  app.addOption( texR, "-s",
                 "input sulci texture (given by siMeshSulciProjection)" );
  app.addOption( model, "-m",
                 "gyri model: choice of the sulcus/sulcus relations "
                 "('gyri.gyr')" );
  app.addOption( graph, "--3D", "compute 3D cortical ribbon gyrus graph",
                 true );
  app.addOption( triB, "-b", "brain mesh file (used if --3D is used)", true );
  app.addOption( imaW, "-p", "output gyri volume, used if --3D is used",
                 true );
  app.addOption( val_domain, "-v",
                 "grey value in input_grey_white vol (default = 100 )", true );
  app.addOption( triGV, "-V",
                 "input_grey_white: input grey/white volume, used if --3D "
                 "is used", true );
  app.addOption( sulcitraductionfile, "--sulcitraduction",
                 "correspondance label string->short_label "
                 "required by siParcellation. (default: sulcitraduction.txt",
                 true );
  app.addOption( time, "-T", "input sulci texture timepoint (default = 0)",
                 true );
  app.addOption( connexity, "--connexity",
                 "connexity or geodesic euclidean distance: use euclidean "
                 "(default = mesh )", true );

  app.alias( "--input", "-i" );
  app.alias( "--brain", "-b" );
  app.alias( "--output", "-o" );
  app.alias( "--graph", "-g" );
  app.alias( "--sulci", "-s" );
  app.alias( "--model", "-m" );
  app.alias( "--parcelvol", "-p" );
  app.alias( "--valdomain", "-v" );
  app.alias( "--value", "-v" );
  app.alias( "--volume", "-V" );
  app.alias( "--Volume", "-V" );
  app.alias( "--translation", "--sulcitraduction" );
  app.alias( "--traduction", "--sulcitraduction" );
  app.alias( "--Time", "-T" );

  try
  {
    app.initialize();

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
    TimeTexture<short>	outTex(2,n);

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

    cout << "Surfacic sulcal parcellation\n";
    set<short>			       	setBack,setFor;
    setBack.insert(0);
    setFor.insert(MESHDISTANCE_FORBIDDEN);
    outTex[1] = MeshVoronoiT<short>( surface[0], outTex[0], 0 ,MESHDISTANCE_FORBIDDEN, dist, connexity,true );


    // Writting the textures
    cout << "writing texture : " << flush;
    texW <<  outTex ;
    cout << "done" << endl;

    //Surfacic gyrus graph
    cout << "Compute surfacic sulcal parcellation graph\n";
    Graph		g("RoiArg");
    Tex2Graph<short>	t2g;
    map<string,set<string> >::iterator igs,egs;
    set<string>::iterator ist,est;
    set<short> setTemp;

    trans_inv.insert(pair<short,string>(0,"background"));

    t2g.makeGraph(g,surface,outTex[1],trans_inv);
    GraphManip::completeGraph(g); //compute mesh area for each surface patch
    Graph::iterator igr,egr;
    string name;
    std::map<string,float>::iterator ig, eg;
    string	base2( agw2.fileName() );
    string::size_type	pos2 = base2.rfind( '/' );
    if( pos2 != string::npos )
      base2.erase( 0, pos2+1 );
    pos2 = base2.rfind( '.' );
    if( pos2 != string::npos )
      base2.erase( pos2, base2.length() - pos2 );
        base2 += ".data";

    //3D gyri parcellation
    if (graph)
      {
        TimeTexture<short> braintex;

        cout << "Reading cortex volume   : " << triGV.fileName() << endl;
        AimsData<short> greyVol;
        triGV >> greyVol;

        cout << "reading brain triangulation   : " << flush;
        AimsSurfaceTriangle brain;
        triB >> brain;
        cout << "done" << endl;

        cout << "Computing 3D parcellisation" << endl;
        Graph		k("RoiArg");
        AimsData<short>    gyriVol;
        gyriVol = MeshParcellation2Volume( greyVol,outTex[1],surface[0],(short)val_domain,0 );
        braintex[0] = VolumeParcellation2MeshParcellation(gyriVol,brain[0],0);
        t2g.makeGraph(k,brain,braintex[0],trans_inv);

        cout << "Computing the volumic  graph " << agw2.fileName() << "\n";
        Graph		*h = GraphManip::graphFromVolume(gyriVol,(short)0, &trans_inv);
        GraphManip::volume2Buckets(*h);
        std::map<string,float> gyrusVolume;
        eg = gyrusVolume.end();
        map <short, float>     stat_gyri_vol;
        map <short,float>::iterator istf,estf;
        stat_gyri_vol = VolumeParcel(gyriVol);
        for (istf = stat_gyri_vol.begin(), estf=stat_gyri_vol.end(); istf != estf; ++istf)
          if ( istf->first != val_domain )
            gyrusVolume[ trans_inv[istf->first]  ] = istf->second;
        for (igr = h->begin(), egr = h->end(); igr != egr; ++igr)
          {
            if ( (*igr)->getProperty("name",name) )
              {
                ig = gyrusVolume.find(name);
                if (ig != eg)
                  (*igr)->setProperty("size", gyrusVolume[name] );
                else
                  cerr << "Could not find the patch bucket with the name " << name << endl;
              }
            else
              cerr << "Could not find the name of a patch \n " ;

          }

        string key = "name";
        Graph *m = GraphManip::mergeGraph(key,g,k,true,true);
        m = GraphManip::mergeGraph(key,*m,*h);
        m->setProperty( "filename_base", base2 );

        cout << "Write cortex ribbon sulcal graph \n";
        try
          {
            agw2.write( *m );
          }
        catch( exception & e )
          {
            cerr << e.what() << endl;
            throw( e );
          }

        cout << "Write gyri volume\n";
        imaW <<  gyriVol ;

      }
    else
      {
        try
          {
              Graph *m;
              m = &g;
              m->setProperty( "filename_base", base2 );
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
  catch( user_interruption &)
  {
  }
  catch( exception & e )
  {
    cerr << e.what() << endl;
  }
  return EXIT_FAILURE;
}

 
