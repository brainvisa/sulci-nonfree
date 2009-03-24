/*
 *  Copyright (C) 2001-2004 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 */

#include <cstdlib>
#include <aims/distancemap/projection.h>
#include <aims/distancemap/meshparcellation.h>
#include <aims/distancemap/thickness.h>
#include <aims/distancemap/stlsort.h>
#include <aims/mesh/curv.h>
#include <aims/io/io_g.h>
#include <aims/math/math_g.h>
#include <aims/getopt/getopt.h>
#include <aims/vector/vector.h>
#include <aims/mesh/texture.h>
#include <aims/io/reader.h>
#include <aims/io/writer.h>
#include <si/fold/labelsTranslator.h>
#include <aims/data/pheader.h>
#include <si/fold/foldReader.h>
#include <si/fold/fgraph.h>
#include <si/global/global.h>
#include <aims/data/data.h>
#include <aims/bucket/bucket.h>
#include <cartobase/smart/rcptr.h>
#include <cartobase/plugin/plugin.h>
#include <iomanip>
#include <fstream>
#include <list>
#include <vector>

using namespace aims;
using namespace sigraph;
using namespace carto;
using namespace std;


typedef float float3[3];

BEGIN_USAGE(usage)
  "-------------------------------------------------------------------------",
  "siMeshSulciOperture      -G[rey]  <grey/white meshfilein>                ",
  "                         -L[CR]    <LCR/white meshfilein>	            ",
  "                         -m[odel]    <gyri model>			    ",
  "                         -l[evel]    <level model>		            ",
  "                         -s 		<sulci names>		            ",
  "                         [-n[bcc] <nb connected component> default=5]   ",
  "                         [--de <distance threshold  default=10 mm]    ",
  "                         [--dp <distance threshold  default=10 mm]    ",
  "                         -g[raph] <sulcus graph>                         ",
  "                         -[input] <mri volume>                          ",
  "                         -v[olume] <output thickness volume>                          ",
  "                         [-t[raduction] <traduction_file> default = traduction.txt]          ",
  "                         [-h[elp]]                                       ",
  "-------------------------------------------------------------------------",
  " Give different measures related to the sulci operture and cortex thickness    ",
  "-------------------------------------------------------------------------",
  "     meshfilein          : input *.tri or *.mesh file                    ",
  "	gyri model          : Choice of the sulcus/sulcus relations         ",
  "     level model         : Choice of the level of description file *.def ",
  "     sulci name          : Attribute for the name of the sulci in the graph (name or label) ",
  "     sulcus graph        : Graph of the sulci *.arg                      ",
  "     traduction_file     : correspondance label string->short_label      ",
  "                           required by siParcellation.		    ",
  "     dmin                : max distance between the voxel and its projection",
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
  char  *mrifile = 0, *outvolfile = 0,*sname = 0, 
    *level = 0, *gwmeshfile = 0, *wlcrmeshfile = 0,  *modelfile=0, *gname = 0;
  float  demin = 10, dpmin = 2;
  long  ncc = 5; 

  PluginLoader::load();

  //
  // Parser of options
  //
  AimsOption opt[] = {
    { 'h',"help"         ,AIMS_OPT_FLAG  ,( void* )Usage           ,AIMS_OPT_CALLFUNC,0},
    { 'G',"grey"         ,AIMS_OPT_STRING,&gwmeshfile     ,0                ,1},
    { 'L',"LCR"          ,AIMS_OPT_STRING,&wlcrmeshfile   ,0                ,1},
    { 'g',"graph"	 ,AIMS_OPT_STRING,&gname	  ,0                ,1},
    { 'm',"model"        ,AIMS_OPT_STRING,&modelfile      ,0                ,1},
    { 'l',"level"        ,AIMS_OPT_STRING,&level          ,0                ,1},
    { 'i',"input"        ,AIMS_OPT_STRING,&mrifile        ,0                ,1},
    { 'v',"volume"       ,AIMS_OPT_STRING,&outvolfile     ,0                ,1},
    { 'n',"numbercc"     ,AIMS_OPT_INT   ,&ncc            ,0                ,0},
    { 's',"sulciname"    ,AIMS_OPT_STRING,&sname          ,0                ,1},
    { ' ', "de"         ,AIMS_OPT_FLOAT ,&demin           ,0                ,0},
    { ' ',  "dp"         ,AIMS_OPT_FLOAT ,&dpmin           ,0                ,0},
    { 0  ,0              ,AIMS_OPT_END   ,0               ,0                ,0}};

  AimsParseOptions( &argc, argv, opt, usage );
  

  //
  // read triangulation
  //
  cout << "reading triangulation   : " << flush;
  AimsSurfaceTriangle gw_surface;
  Reader<AimsSurfaceTriangle> triR( gwmeshfile );
  triR >> gw_surface;
  AimsSurfaceTriangle wlcr_surface;
  Reader<AimsSurfaceTriangle> triR2( wlcrmeshfile );
  triR2 >> wlcr_surface;
  cout << "done" << endl;

  //
  // read volume
  //
  cout << "reading volume info\n";
  Finder	f;
  assert( f.check( mrifile ) );
  const PythonHeader	*hd 
    = dynamic_cast<const PythonHeader *>( f.header() );
  assert( hd );
  vector<int>   bb;
  assert( hd->getProperty( "volume_dimension", bb ) );
  
 
  
  //Read the graph and convert to labelled volume
  cout << "Read the graph\n";
  string					bname = "aims_bottom",ssname = "aims_ss",othername= "aims_other";
  string					synt;
  FGraph					fg;
  AimsData<short>				surface_vol( bb[0],bb[1],bb[2],1,1 );
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
     
      assert( fg.getProperty( "voxel_size", vsz ) );
      surface_vol.setSizeXYZT( vsz[0], vsz[1], vsz[2], 1. );
      surface_vol = 0;
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
      surface_vol = surface_vol.clone();
      for( iv=fg.begin(); iv!=fv; ++iv )
	{
	  assert( (*iv)->getProperty( "name", name ) );
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
  GyriAndSulci = meshdistance::GyrusModel2GyriAndSulci(modelfile);
  labels = meshdistance::GyrusModel2SetOfSulci(GyriAndSulci,slab);

  
  cout << "grey/white mesh vertices : " << gw_surface[0].vertex().size() << endl;
  cout << "white/LCR mesh vertices : " << wlcr_surface[0].vertex().size() << endl;
  TimeTexture<short>	outTex;


  map<Point3df, pair<float,float> , Point3dfCompare > sulci2gw;
  //map<Point3df, pair<float,float> , Point3dfCompare > sulci2wlcr;

  AimsData<float>    vol(surface_vol.dimX(),surface_vol.dimY(),surface_vol.dimZ(),1);
  float sx = surface_vol.sizeX(),sy = surface_vol.sizeY(),sz = surface_vol.sizeZ(), st = surface_vol.sizeT() ;
  vol.setSizeXYZT(sx,sy,sz,st);
  ASSERT(sx * sy * sz != 0);


  meshdistance::SulcusOperture( gw_surface[0],sulci2gw,surface_vol,
  				demin , dpmin, ncc, transInv, labels); 
  
  vol =  meshdistance::OpertureParameters(gw_surface[0],sulci2gw,surface_vol);

  
  
  //meshdistance::SulcusOperture( wlcr_surface[0],
  //				sulci2wlcr,surface_vol,
  //			demin , dpmin, ncc, transInv, labels); 
  


  cout << "writing volume : " << flush;
  Writer<AimsData<float> >	texV( outvolfile);
  texV << vol;
  cout << "done" << endl;
  
  
  return( 0 );
}

