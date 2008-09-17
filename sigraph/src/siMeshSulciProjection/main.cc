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
#include <aims/io/io_g.h>
#include <aims/math/math_g.h>
#include <aims/getopt/getopt.h>
#include <aims/vector/vector.h>
#include <aims/mesh/texture.h>
#include <aims/io/reader.h>
#include <aims/io/writer.h>
#include <aims/data/data.h>
#include <aims/bucket/bucket.h>
#include <aims/data/pheader.h>
#include <graph/graph/graph.h>
#include <cartobase/smart/rcptr.h>
#include <cartobase/plugin/plugin.h>
#include <fstream>
#include <iomanip>
#include <list>
#include <vector>

using namespace aims;
using namespace sigraph;
using namespace carto;
using namespace std;


typedef float float3[3];

BEGIN_USAGE(usage)
  "-------------------------------------------------------------------------",
  "siMeshSulciProjection    -i[nput] <meshfilein>                           ",
  "                         -m[odel]    <gyri model>			    ",
  "                         -l[evel]    <level model>		            ",
  "                         -s 		<sulci names>		            ",
  "                         -g[raph] <sulcus graph>                         ",
  "                         [-t[raduction] <traduction_file> default = traduction.txt]          ",
  "                         [-n[umbercc] <number_connex_components> default = 4]        ",
  "                         [-a[lpha_reg]    <estimation_ratio > default = 1]	    ",
  "                         [-e[min]     <euclidean distance threshold> default = 5 mm]  ",        
  "                         [-p[min]     <plane distance threshold> default = 2 mm]  ",        
  "                         [-V[olume_radius] <closing volume radius> default = 1.5 mm ]    ",
  "                         [-M[esh_radius] <closing radius for mesh > default = 1.5*r ]    ",
  "                         [-o[utput] <output_sulc_texture>]               ",
  "                         [--connexity <metric> default =mesh connexity",
  "                         [-c[urvature] <curvature map>]                  ",
  "                         [-K          <curvature  coef.> default = 2]    ",
  "                         [-h[elp]]                                       ",
  "-------------------------------------------------------------------------",
  " Project sulcus bottom points (voxels) on mesh texture (nodes).          ",
  " Only sulci present in the <gyri model> file  are projected.             ",
  " Their label correspond to the hierarchy defined in the <level model> file",
  " There are two alternative approach for the projection.   ",
  " The first one uses a metric combining an euclidean distance and map (curvature or depth,..) information. ",
  " The second one uses tangent plane.                                ",
  "-------------------------------------------------------------------------",
  "     meshfilein          : input *.tri or *.mesh file                    ",
  "	gyri model          : Choice of the sulcus/sulcus relations         ",
  "     level model         : Choice of the level of description file *.def ",
  "     sulci name          : Attribute for the name of the sulci in the graph (name or label) ",
  "     sulcus graph        : Graph of the sulci *.arg                      ",
  "     traduction_file     : correspondance label string->short_label      ",
  "                           required by siParcellation.		    ",
  "     number_connex_components : minimal number of point in each connected component ",
  "     estimation_ration   : Distance threshold for detecting outliers for affine projection estimation.",
  "                           Only points closer than (estimation_ration * dmin) are used for the estimation",
  "     dmin                : max distance between a voxel and its projection",
  "     input_curvature     : input *.tex curvature                         ",
  "     curvature coef.: influence of the curvature          ",
  "	closing radius	    : Radius for the sulci closing in mm            ",
  "     output_sulc_texture : Projected sulci texture *.tex                 ",
  "     connexity           : Underlying mesh metric : euclidean/mesh connexity                 ",
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
  PluginLoader::load();

  char  *volfile = 0,*sname = 0, *level = 0, *meshfile = 0,  *outtexfile = 0, *curvtexfile = 0, *modelfile=0, *gname = 0;
  long  ncc = 4; 
  float  K = 2, demin = 5,dpmin = 2, radius = 1.5,rmadius = 1.5* radius,alpha_reg = 1 ;
  bool connexity = false;
  char	*traductionfile = (char*)"traduction.txt";
  //
  // Parser of options
  //
  AimsOption opt[] = {
    { 'h',"help"         ,AIMS_OPT_FLAG  ,( void* )Usage           ,AIMS_OPT_CALLFUNC,0},
    { 'i',"input"        ,AIMS_OPT_STRING,&meshfile       ,0                ,1},
    { 'g',"graph"	 ,AIMS_OPT_STRING,&gname	  ,0                ,1},
    { 'o',"output"       ,AIMS_OPT_STRING,&outtexfile     ,0                ,1},
    { 'c',"curvature"    ,AIMS_OPT_STRING,&curvtexfile    ,0                ,0},
    { 'm',"model"        ,AIMS_OPT_STRING,&modelfile      ,0                ,1},
    { 'l',"level"        ,AIMS_OPT_STRING,&level          ,0                ,1},
    { 'v',"volume"       ,AIMS_OPT_STRING,&volfile        ,0                ,1},
    { 's',"sulciname"    ,AIMS_OPT_STRING,&sname          ,0                ,1},
    { 'K',"K"            ,AIMS_OPT_FLOAT ,&K              ,0                ,0},
    { 't',"traduction"   ,AIMS_OPT_STRING,&traductionfile ,0                ,0},
    { 'e',"demin"        ,AIMS_OPT_FLOAT ,&demin          ,0                ,0},
    { 'p',"dpmin"        ,AIMS_OPT_FLOAT ,&dpmin          ,0                ,0},
    { 'V',"Volume_radius",AIMS_OPT_FLOAT ,&radius         ,0                ,0},
    { 'M',"Mesh_radius"  ,AIMS_OPT_FLOAT ,&rmadius        ,0                ,0},
    { 'a',"alpha"        ,AIMS_OPT_FLOAT ,&alpha_reg      ,0                ,0},
    { ' ',"connexity"    ,AIMS_OPT_FLAG  ,&connexity      ,0                ,0},    
    { 'n',"numbercc"     ,AIMS_OPT_INT   ,&ncc            ,0                ,0},
    { 0  ,0              ,AIMS_OPT_END   ,0               ,0                ,0}};

  AimsParseOptions( &argc, argv, opt, usage );
  

  //
  // read triangulation
  //
  cout << "reading triangulation   : " << flush;
  AimsSurfaceTriangle surface;
  Reader<AimsSurfaceTriangle> triR( meshfile );
  triR >> surface;
  cout << "done" << endl;

  //
  // read volume
  //
  cout << "reading volume info\n";
  Finder	f;
  assert( f.check( volfile ) );
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
      Reader<Graph>	fr( gname );
      fr.read( fg );
      //fg.loadBuckets( gname, true );
      cout << "graph read\n";

      fg.getProperty( "anterior_commissure", CA );
      assert( fg.getProperty( "voxel_size", vsz ) );
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
      ofstream	namefile( traductionfile );
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
      cout << "Write " << (string)traductionfile << " file\n";
      levelTrans.translate( fg, sname, "name" ); 
       

      //Def volume of sulcal lines
      cout << "Definition of the volume of labels...\n";
      Vertex::const_iterator ie, fe;
      for( iv=fg.begin(); iv!=fv; ++iv )
	{
	  assert( (*iv)->getProperty( "name", name ) );
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

      for( iv=fg.begin(); iv!=fv; ++iv )
	{
	  assert( (*iv)->getProperty( "name", name ) );
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
  GyriAndSulci = meshdistance::GyrusModel2GyriAndSulci(modelfile);
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
   
  if (curvtexfile != 0)
    {
      //
      // read input curv texture
      //
      cout << "reading curvature texture   : " << flush;
      TimeTexture<float>	curvTex;
      Reader<TimeTexture<float> > ctexR( curvtexfile );
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

  Writer<TimeTexture<short> >	texW( outtexfile);
  texW << otex;
  //texW << outTex;
  cout << "done" << endl;

  return( 0 );
}


