/*
 *  Copyright (C) 2001-2004 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 */

#include <aims/distancemap/projection.h>
#include <aims/distancemap/meshparcellation.h>
#include <aims/distancemap/thickness.h>
#include <aims/distancemap/stlsort.h>
#include <aims/mesh/curv.h>
#include <aims/getopt/getopt2.h>
#include <aims/vector/vector.h>
#include <aims/mesh/texture.h>
#include <aims/io/reader.h>
#include <aims/io/writer.h>
#include <aims/io/finder.h>
#include <si/fold/labelsTranslator.h>
#include <aims/data/pheader.h>
#include <si/fold/foldReader.h>
#include <si/fold/fgraph.h>
#include <si/global/global.h>
#include <aims/data/data.h>
#include <aims/bucket/bucket.h>

using namespace aims;
using namespace sigraph;
using namespace carto;
using namespace std;


typedef float float3[3];


int main( int argc, const char** argv )
{
  Reader<AimsSurfaceTriangle> triR;
  Reader<AimsSurfaceTriangle> triR2;
  Writer<Volume<float> > texV;

  string mrifile, outvolfile, sname, level, modelfile, gname;
  float  demin = 10, dpmin = 2;
  long  ncc = 5; 

  AimsApplication app( argc, argv,
                       "Give different measures related to the sulci operture "
                       "and cortex thickness" );
  app.addOption( triR, "-G", "input grey/white mesh file" );
  app.addOption( triR2, "-L", "input pial mesh file" );
  app.addOption( gname, "-g", "sulcus graph" );
  app.addOption( modelfile, "-m",
                 "gyri model: Choice of the sulcus/sulcus relations" );
  app.addOption( level, "-l",
                 "level model: choice of the level of description file "
                 "*.def" );
  app.addOption( mrifile, "-i", "input MRI volume (for dimensions)" );
  app.addOption( texV, "-v", "output thickness volume" );
  app.addOption( sname, "-s",
                 "Attribute for the name of the sulci in the graph "
                 "(name or label)" );
  app.addOption( ncc, "-n", "nb of connected components (default=5)", true );
  app.addOption( demin, "--de",
                 "distance threshold (default=10 mm): max distance between "
                 "the voxel and its projection", true );
  app.addOption( dpmin, "--dp", "distance threshold (default=10 mm)", true );

  app.alias( "--grey", "-G" );
  app.alias( "--white", "-G" );
  app.alias( "--CSF", "-L" );
  app.alias( "--LCR", "-L" );
  app.alias( "--graph", "-g" );
  app.alias( "--model", "-m" );
  app.alias( "--level", "-l" );
  app.alias( "--input", "-i" );
  app.alias( "--volume", "-v" );
  app.alias( "--sulciname", "-s" );
  app.alias( "--numbercc", "-n" );
  app.alias( "--nbcc", "-n" );

  try
  {
    app.initialize();

    //
    // read triangulation
    //
    cout << "reading triangulation   : " << flush;
    AimsSurfaceTriangle gw_surface;
    triR >> gw_surface;
    AimsSurfaceTriangle wlcr_surface;
    triR2 >> wlcr_surface;
    cout << "done" << endl;

    //
    // read volume
    //
    cout << "reading volume info\n";
    Finder	f;
    ASSERT( f.check( mrifile ) );
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
    VolumeRef<short>		surface_vol( bb[0], bb[1], bb[2], 1, 1 );
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

    try
      {
        //Read the graph
        cout << "create FoldReader\n";
        FoldReader	fr( gname );
        if( !fr )
          {
            cerr << "erreur ouverture graphe\n";
            exit(1);
          }

        cout << "read graph...\n";
        fr >> fg;
        fg.loadBuckets( gname, true );

        ASSERT( fg.getProperty( "voxel_size", vsz ) );
        surface_vol.setVoxelSize( vsz[0], vsz[1], vsz[2], 1. );
        surface_vol.fill( 0 );
        surface_vol.fillBorder(0);

        for( il=levelTrans.begin(); il!=fl; ++il )
          {
            slab.insert( (*il).second );
          }
        for( i = 1, is=slab.begin(); is!=fs; ++is, ++i )
          {
            trans[*is] = i;
            transInv[i] = *is;
          }

        levelTrans.translate( fg, sname, "name" );

        Vertex::const_iterator ie, fe;


        //Def volume of sulcal surface (sulci)
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
                            surface_vol.at( pos[0], pos[1], pos[2] ) = i;
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
                                surface_vol.at( pos[0], pos[1], pos[2] ) = i;
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

    std::map<string,Point3df> sulciBBmin,sulciBBmax;
    std::map<string,Point3df>::iterator isBB; //,esBB;
    vector<int>   bbmin,bbmax;
    Point3df temp;
    for( iv=fg.begin(); iv!=fv; ++iv )
      {
        if( ! (*iv)->getProperty( "boundingbox_min" , bbmin ) )
          cerr << "Could not find the bounding box min of the node " << *iv << endl;
        if( ! (*iv)->getProperty( "boundingbox_max" , bbmax ) )
          cerr << "Could not find the bounding box max of the node " << *iv << endl;
        if (! (*iv)->getProperty( "name", name ))
          cerr << "Could not find the " << sname << " of the node " << *iv << endl;
        isBB = sulciBBmin.find(name);
        if (isBB == sulciBBmin.end() )
          sulciBBmin[name] = Point3df((isBB->second)[0],(isBB->second)[1],(isBB->second)[2]) ;
        else
          sulciBBmin[name] = aims::meshdistance::Point3dfMin(isBB->second, Point3df(bbmin[0],bbmin[1],bbmin[2]));
        isBB = sulciBBmax.find(name);
        if (isBB == sulciBBmax.end() )
          sulciBBmax[name] = Point3df((isBB->second)[0],(isBB->second)[1],(isBB->second)[2]) ;
        else
          sulciBBmax[name] = aims::meshdistance::Point3dfMax(isBB->second,Point3df(bbmin[0],bbmin[1],bbmin[2]));
      }

    set<unsigned> points;
    set<unsigned>::iterator ip,ep;
    const vector<Point3df>		& vert= gw_surface[0].vertex();
    unsigned n = vert.size();
    TimeTexture<short> tex(1,n);
    cout << sulciBBmin["S.C._left"] << " " << sulciBBmax["S.C._left"] << endl;
    points = aims::meshdistance::SubMesh(gw_surface[0],sulciBBmin["S.C._left"],sulciBBmax["S.C._left"]);
    for (ip = points.begin(), ep = points.end(); ip != ep ; ++ip )
        tex[0].item(*ip) = (short)1;

    cout << "writing texture : " << flush;
    Writer<TimeTexture<short> >	texT("/tmp/pt.tex");
    texT << tex;
    cout << "done" << endl;


    // read gyri model
    set<string>	labels;
    cout << "Read gyri model file \n";
    map<string,set<string> > GyriAndSulci;
    GyriAndSulci = meshdistance::GyrusModel2GyriAndSulci(modelfile.c_str());
    labels = meshdistance::GyrusModel2SetOfSulci(GyriAndSulci,slab);


    cout << "grey/white mesh vertices : " << gw_surface[0].vertex().size() << endl;
    cout << "white/LCR mesh vertices : " << wlcr_surface[0].vertex().size() << endl;
    TimeTexture<short>	outTex;


    map<Point3df, pair<float,float> , Point3dfCompare > sulci2gw;
    //map<Point3df, pair<float,float> , Point3dfCompare > sulci2wlcr;

    VolumeRef<float>    vol( surface_vol.getSize() );
    vol.setVoxelSize( surface_vol.getVoxelSize() );


    meshdistance::SulcusOperture( gw_surface[0],sulci2gw,surface_vol,
                                  demin , dpmin, ncc, transInv, labels);

    vol =  meshdistance::OpertureParameters(gw_surface[0],sulci2gw,surface_vol);



    //meshdistance::SulcusOperture( wlcr_surface[0],
    //				sulci2wlcr,surface_vol,
    //			demin , dpmin, ncc, transInv, labels);



    cout << "writing volume : " << flush;
    texV.write( *vol );
    cout << "done" << endl;


    return( 0 );
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

