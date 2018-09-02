/*
 *  Copyright (C) 2001-2004 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 */

#include <si/fold/labelsTranslator.h>
#include <si/graph/vertexclique.h> 
#include <si/fold/foldReader.h>
#include <si/fold/fgraph.h>
#include <si/global/global.h>
#include <aims/distancemap/projection.h>
#include <aims/distancemap/meshparcellation.h>
#include <aims/mesh/curv.h>
#include <aims/io/reader.h>
#include <aims/io/writer.h>
#include <aims/getopt/getopt2.h>
#include <aims/vector/vector.h>
#include <aims/mesh/texture.h>
#include <aims/io/reader.h>
#include <aims/io/writer.h>
#include <aims/io/finder.h>
#include <aims/data/data.h>
#include <aims/bucket/bucket.h>
#include <aims/data/pheader.h>
#include <graph/graph/graph.h>
#include <cartobase/exception/assert.h>
#include <cartobase/smart/rcptr.h>
#include <cartobase/plugin/plugin.h>

using namespace aims;
using namespace sigraph;
using namespace carto;
using namespace std;


typedef float float3[3];


int main( int argc, const char** argv )
{
  Reader<AimsSurfaceTriangle> triR;
  Reader<Graph> fr;
  Writer<TimeTexture<short> > texW;
  Reader<TimeTexture<float> > ctexR;

  string volfile, sname, level, modelfile;
  long  ncc = 4; 
  float  K = 2, demin = 5,dpmin = 2, radius = 1.5, rmadius = 0, alpha_reg = 1;
  bool connexity = false;
  string traductionfile = "traduction.txt";

  AimsApplication app( argc, argv,
                       "Projects sulcus bottom points (voxels) on mesh "
                       "texture (nodes). Only sulci present in the "
                       "<gyri model> file  are projected.  Their label "
                       "correspond to the hierarchy defined in the "
                       "<level model> file.\n"
                       "There are two alternative approaches for the "
                       "projection.\n"
                       "The first one uses a metric combining an euclidean "
                       "distance and map (curvature or depth,..) "
                       "information.\n"
                       "The second one uses tangent plane." );
  app.addOption( triR, "-i", "input mesh file" );
  app.addOption( fr, "-g", "Graph of the sulci *.arg" );
  app.addOption( texW, "-o", "output projected sulci texture *.tex" );
  app.addOption( modelfile, "-m",
                 "model file: choice of the sulcus/sulcus relations" );
  app.addOption( level, "-l",
                 "Choice of the level of description file *.def" );
  app.addOption( volfile, "-v", "volume file (to use its dimensions" );
  app.addOption( sname, "-s",
                 "attribute for the name of the sulci in the graph (name or "
                 "label)" );
  app.addOption(ctexR, "-c", "input curvature texture", false );
  app.addOption( K, "-K",
                 "curvature coef.: influence of the curvature (default: 2.)",
                 false );
  app.addOption( traductionfile, "-t",
                 "correspondance label string->short_label required by "
                 "siParcellation. (default: traduction.txt)", true );
  app.addOption( demin, "-e",
                 "dmin: euclidean distance threshold (default = 5 mm): "
                 "max distance between a voxel and its projection", true );
  app.addOption( dpmin, "-p", "plane distance threshold (default = 2 mm)",
                 true );
  app.addOption( radius, "-V",
                 "volume_radius: Radius for the sulci closing in mm "
                 "(default: 1.5)", true );
  app.addOption( rmadius, "-M",
                 "closing radius for mesh (default = 1.5*volume_radius)",
                 true );
  app.addOption( alpha_reg, "-a",
                 "estimation_ratio: distance threshold for detecting outliers "
                 "for affine projection estimation. (default = 1).\n"
                 "Only points closer than (estimation_ratio * dmin) are used "
                 "for the estimation", true );
  app.addOption( connexity, "--connexity",
                 "connexity: underlying mesh metric: use euclidean "
                 "(instead of mesh connexity by default)", true );
  app.addOption( ncc, "-n",
                 "minimal number of point in each connected component "
                 "(dfault = 4)", true );

  app.alias( "--input", "-i" );
  app.alias( "--graph", "-g" );
  app.alias( "--output", "-o" );
  app.alias( "--curvature", "-c" );
  app.alias( "--model", "-m" );
  app.alias( "--level", "-l" );
  app.alias( "--volume", "-v" );
  app.alias( "--sulciname", "-s" );
  app.alias( "--translation", "-t" );
  app.alias( "--traduction", "-t" );
  app.alias( "--emin", "-e" );
  app.alias( "--pmin", "-p" );
  app.alias( "--Volume_radius", "-V" );
  app.alias( "--Mesh_radius", "-M" );
  app.alias( "--alpha", "-a" );
  app.alias( "--numbercc", "-n" );

  try
  {
    app.initialize();

    if( rmadius == 0 )
      rmadius = radius * 1.5;

    //
    // read triangulation
    //
    cout << "reading triangulation   : " << flush;
    AimsSurfaceTriangle surface;
    triR >> surface;
    cout << "done" << endl;

    //
    // read volume
    //
    cout << "reading volume info\n";
    Finder	f;
    ASSERT( f.check( volfile ) );
    const PythonHeader	*hd
    = dynamic_cast<const PythonHeader *>( f.header() );
    ASSERT( hd );
    vector<int>   bb;
    ASSERT( hd->getProperty( "volume_dimension", bb ) );



    //Read the graph and convert to labelled volume
    cout << "Read the graph\n";
    string					bname = "aims_bottom",ssname = "aims_ss",othername= "aims_other";
    string					synt;
    FGraph					fg;
    AimsData<short>				surface_vol( bb[0],bb[1],bb[2],1,1 ),bottom_vol( bb[0],bb[1],bb[2],1,1 );
    map<string,short>				trans;
    map <short,string>    			transInv;
    Graph::iterator				iv, fv=fg.end();
    string					name;
    vector<float>					vsz;
    set<string>					slab;
    FoldLabelsTranslator				levelTrans( level );
    map<string, string>::const_iterator		il, fl = levelTrans.end();
    set<string>::const_iterator			is, fs=slab.end();
    short						i;
    rc_ptr<BucketMap<Void> >			bck;
    BucketMap<Void>::Bucket::const_iterator	ib, fb;
    AimsVector<short, 3>				pos;
    vector<int>					CA;

    try
      {
        //Read the graph
        fr.read( fg );
        //fg.loadBuckets( gname, true );
        cout << "graph read\n";

        fg.getProperty( "anterior_commissure", CA );
        ASSERT( fg.getProperty( "voxel_size", vsz ) );
        bottom_vol.setSizeXYZT( vsz[0], vsz[1], vsz[2], 1. );
        bottom_vol = 0;
        bottom_vol.fillBorder(0);
        cout << "CA: " << Point3df(CA[0] * vsz[0],CA[1] * vsz[1],CA[2] * vsz[2])  << endl;

        //Check if there is some ventricule in the graph
        //otherwise,  the biggest (graph) connected component is labelled ventricule_left or right
        set< CComponent * > setCC;
        set< CComponent * >::iterator iscc,escc;
        CComponent  ventricleV;
        CComponent unknownV;
        CComponent::iterator icc,ecc;
        bool ventricleIn = false ;
        string v = "ventricle";
        unsigned sizeCC = 0;
        for ( iv = fg.begin(); iv != fv; ++iv )
          {
            (*iv)->getProperty(sname,name );
            if ( name == "ventricle_left" ||   name == "ventricle_right")
                ventricleIn = true;
            if ( name == "unknown" ||   name == "unknown")
                unknownV.insert(*iv);
            slab.insert(name);
          }

        for ( is = slab.begin(), fs = slab.end(); is != fs; ++is   )
          if (is->rfind("left") != string::npos)
            {
              v += "_left";
              break;
            }
          else
            if (is->rfind("right") != string::npos)
              {
                v += "_right";
                break;
              }

          slab.clear();
          if (!ventricleIn)
          {
            VertexClique VC;
            unsigned nCC = 0;
            nCC = VC.connectivity(unknownV,&setCC,(string)"junction");
            cout << nCC << " connected component with the " << sname << " 'unknown' in the the fold graph \n";
            cout << "Giving the " << sname << " "  << v << " to the biggest one \n";
            for (iscc = setCC.begin(), escc = setCC.end(); iscc != escc; ++iscc)
              if (sizeCC < (*iscc)->size() )
                {
                  ventricleV = *(*iscc);
                  sizeCC = (*iscc)->size();
                }
            cout << ventricleV.size() << " " << sizeCC << endl;
            for ( icc = ventricleV.begin(), ecc = ventricleV.end(); icc != ecc; ++icc  )
              (*icc)->setProperty(sname,v);
          }




        //Write translation file
        ofstream	namefile( traductionfile.c_str() );
        for( il=levelTrans.begin(); il!=fl; ++il )
          slab.insert( (*il).second );
        slab.insert("unknown");
        for( i = 1, is=slab.begin(); is!=fs; ++is, ++i )
          {
            trans[*is] = i;
            transInv[i] = *is;
            namefile << *is << "\t" << i << endl;
          }
        trans["unknown"] = i;
        transInv[i] = "unknown";
        namefile << "unknown" << "\t" << i << endl;
        cout << "Write " << traductionfile << " file\n";
        levelTrans.translate( fg, sname, "name" );


        //Def volume of sulcal lines
        cout << "Definition of the volume of labels...\n";
        Vertex::const_iterator ie, fe;
        for( iv=fg.begin(); iv!=fv; ++iv )
          {
            ASSERT( (*iv)->getProperty( "name", name ) );
            i = trans[name];
            if( i != 0 )
              {
                if( (*iv)->getProperty( bname, bck ) )
                  {
                    if( ( synt.empty() || (*iv)->getSyntax() == synt ) )
                      {
                        BucketMap<Void>::Bucket	& bl = (*bck)[0];
                        for( ib=bl.begin(), fb=bl.end(); ib!=fb; ++ib )
                          {
                            pos = ib->first;
                            bottom_vol( pos[0], pos[1], pos[2] ) = i;
                          }
                      }
                  }
                else
                  {
                    for( ie=(*iv)->begin(), fe=(*iv)->end(); ie!=fe; ie++)
                      {
                        if( ( synt.empty() || (*ie)->getSyntax() == synt )
                            && (*ie)->getProperty( bname, bck ) )
                          {
                            BucketMap<Void>::Bucket	& bl = (*bck)[0];
                            for( ib=bl.begin(), fb=bl.end(); ib!=fb; ++ib )
                              {
                                pos = ib->first;
                                bottom_vol( pos[0], pos[1], pos[2] ) = i;
                              }
                          }
                      }
                  }
              }
          }

        //Def volume of sulcal surface (sulci)
        surface_vol = bottom_vol.clone();
        for( iv=fg.begin(); iv!=fv; ++iv )
          {
            ASSERT( (*iv)->getProperty( "name", name ) );
            i = trans[name];
            if( i != 0 )
              {
                if( (*iv)->getProperty( ssname, bck ) )
                  {
                    if( ( synt.empty() || (*iv)->getSyntax() == synt ) )
                      {
                        BucketMap<Void>::Bucket	& bl = (*bck)[0];
                        for( ib=bl.begin(), fb=bl.end(); ib!=fb; ++ib )
                          {
                            pos = ib->first;
                            surface_vol( pos[0], pos[1], pos[2] ) = i;
                          }
                      }
                  }
                else
                  {
                    for( ie=(*iv)->begin(), fe=(*iv)->end(); ie!=fe; ie++)
                      {
                        if( ( synt.empty() || (*ie)->getSyntax() == synt )
                            && (*ie)->getProperty( ssname, bck ) )
                          {
                            BucketMap<Void>::Bucket	& bl = (*bck)[0];
                            for( ib=bl.begin(), fb=bl.end(); ib!=fb; ++ib )
                              {
                                pos = ib->first;
                                surface_vol( pos[0], pos[1], pos[2] ) = i;
                              }
                          }
                      }
                  }
              }
          }

        for( iv=fg.begin(); iv!=fv; ++iv )
          {
            ASSERT( (*iv)->getProperty( "name", name ) );
            i = trans[name];
            if( i != 0 )
              {
                if( (*iv)->getProperty( othername, bck ) )
                  {
                    if( ( synt.empty() || (*iv)->getSyntax() == synt ) )
                      {
                        BucketMap<Void>::Bucket	& bl = (*bck)[0];
                        for( ib=bl.begin(), fb=bl.end(); ib!=fb; ++ib )
                          {
                            pos = ib->first;
                            surface_vol( pos[0], pos[1], pos[2] ) = i;
                          }
                      }
                  }
                else
                  {
                    for( ie=(*iv)->begin(), fe=(*iv)->end(); ie!=fe; ie++)
                      {
                        if( ( synt.empty() || (*ie)->getSyntax() == synt )
                            && (*ie)->getProperty( othername, bck ) )
                          {
                            BucketMap<Void>::Bucket	& bl = (*bck)[0];
                            for( ib=bl.begin(), fb=bl.end(); ib!=fb; ++ib )
                              {
                                pos = ib->first;
                                surface_vol( pos[0], pos[1], pos[2] ) = i;
                              }
                          }
                      }
                  }
              }
          }

      }
    catch( exception & e )
      {
        cerr << e.what() << endl;
        exit( 1 );
      }


    // read gyri model
    set<string>	labels;
    cout << "Read gyri model file \n";
    map<string,set<string> > GyriAndSulci;
    GyriAndSulci = meshdistance::GyrusModel2GyriAndSulci( modelfile.c_str() );
    labels = meshdistance::GyrusModel2SetOfSulci(GyriAndSulci,slab);
    //labels = meshdistance::GyrusModel2SetOfSulci(modelfile,slab);


    //Order the triangulation nodes
    vector< list<unsigned> > neighbourso(surface[0].vertex().size());
    cout << "Ordering the triangulation nodes (clockwise)" << flush;
    neighbourso = AimsMeshOrderNode(surface[0]);
    cout << "done\n";

  cout << "mesh vertices : " << surface[0].vertex().size() << endl;
  cout << "mesh polygons : " << surface[0].polygon().size() << endl;
  TimeTexture<short>	outTex;

    if( !ctexR.fileName().empty() )
      {
        //
        // read input curv texture
        //
        cout << "reading curvature texture   : " << flush;
        TimeTexture<float>	curvTex;
        ctexR >> curvTex;
        cout << "done" << endl;
        cout << "texture dim   : " << curvTex[0].nItem() << endl;
        outTex = meshdistance::SulcusVolume2Texture( surface[0], curvTex[0], bottom_vol ,surface_vol,
                                                      K , demin , ncc, transInv, labels,radius,rmadius,alpha_reg,
                                                      connexity,neighbourso);
      }
    else
      {
        ASSERT (dpmin != 0);
        outTex = meshdistance::SulcusVolume2Texture( surface[0], bottom_vol ,surface_vol,Point3df(CA[0] * vsz[0],CA[1] * vsz[1],CA[2] * vsz[2]),
                                                  demin , dpmin, ncc, transInv, labels,radius,rmadius,alpha_reg,
                                                  connexity,neighbourso);
      }
    cout << "writing texture : " << flush;
    TimeTexture<short> otex;
    otex[0] = outTex[0];

    texW << otex;
    //texW << outTex;
    cout << "done" << endl;

    return EXIT_SUCCESS;
  }
  catch( user_interruption & )
  {
  }
  catch( exception & e )
  {
    cerr << e.what() << endl;
  }
  return EXIT_FAILURE;
}


