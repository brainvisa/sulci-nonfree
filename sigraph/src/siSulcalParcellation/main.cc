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
#include <aims/data/data_g.h>
#include <aims/io/io_g.h>
#include <aims/math/math_g.h>
#include <iomanip>
#include <aims/getopt/getopt.h>
#include <aims/vector/vector.h>
#include <aims/mesh/texture.h>
#include <aims/io/reader.h>
#include <aims/io/writer.h>
#include <aims/connectivity/meshcc.h>
#include <fstream>
#include <float.h>
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
 
typedef float float3[3];

BEGIN_USAGE(usage)
  "-------------------------------------------------------------------------",
  "siSulcalParcellation   -i[nput] <grey/white mesh>                                   ",
  "                  -s[sulci] <input_texture>                              ",
  "                  -g[raph] <output_gyri_graph>                           ",
  "                  -m[odel] <gyri model >                                 ",
  "                  -b[rain] <brain mesh >                                 ",
  "                  -o[utput] <output_gyri_texture>                          ",
  "                  -p[arcelfile] <output_gyri_volume>                        ",
  "                  [-v[alue] <grey value in input_grey_white vol>]                       ",
  "                  [-V[olume] <input_grey_white volume>]                       ",
  "                  [-T[ime] <input sulci texture time> default = 0]               ",
  "                  [-t[raduction] <traduction_file>  ] ",
 "                   [--connexity ]                                       ",
 "                   [--3D ]                                       ",
  "                  [-h[elp]]                                              ",
  "-------------------------------------------------------------------------",
  " Compute parcellation in sulcal regions                           ",
  " The input sulci texture must be defined by siMeshSulciProjection        ",
  "-------------------------------------------------------------------------",
  "     meshfilein          : input *.tri or *.mesh file                    ",
  "     input_texture       : definition of the sulci texture (given by siMeshSulciProjection",
  "	gyri model          : Choice of the sulcus/sulcus relations ('gyri.gyr')",
  "     traduction_file     : correspondance label string->short_label      ",
  "                           required by siParcellation.		    ",
  "     connexity           : connexity or geodesic euclidean distance      ",
  "     3D                  : compute 3D cortical ribbon gyrus graph            ",
  "     output_vor_texture : output timetexture file (sulci and gyri texture)   ",
  "     0: sulci, 1 : sulcal regions", 
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
  char	*brainfile=0,*parcelvolfile=0,
    *volfile=0, *graphfile = 0, 
    *meshfile = 0, *intexfile = 0, 
    *outtexfile = 0, *model=0;
  float dist = FLT_MAX;
  map< set<short>,set<unsigned>,SetCompare<short> > 		label_sulci_Vert;
  bool connexity = false, graph=false;
  unsigned	time=0;
  char	*sulcitraductionfile = (char*)"sulcitraduction.txt";
  int val_domain = 100;

  PluginLoader::load();

  //
  // Parser of options
  //
  AimsOption opt[] = {
    { 'h',"help"         ,AIMS_OPT_FLAG  ,( void* )Usage           ,AIMS_OPT_CALLFUNC,0},
    { 'i',"input"        ,AIMS_OPT_STRING,&meshfile       ,0                ,1},
    { 'b',"brain"        ,AIMS_OPT_STRING,&brainfile       ,0               ,0},
    { 'o',"output"       ,AIMS_OPT_STRING,&outtexfile     ,0                ,1},
    { 'g',"graph"        ,AIMS_OPT_STRING,&graphfile      ,0                ,1},
    { 's',"sulci"        ,AIMS_OPT_STRING,&intexfile      ,0                ,1},
    { 'p',"parcelvol"    ,AIMS_OPT_STRING,&parcelvolfile  ,0                ,0},
    { 'v',"valdomain"    ,AIMS_OPT_INT   ,&val_domain     ,0                ,0},
    { 'V',"volume"       ,AIMS_OPT_STRING,&volfile        ,0                ,0},
    { 'm',"model"        ,AIMS_OPT_STRING,&model          ,0                ,1},
    { ' ',"sulcitraduction"   ,AIMS_OPT_STRING,&sulcitraductionfile ,0                ,1},
    { 'T',"Time"         ,AIMS_OPT_INT   ,&time           ,0                ,0},
    { ' ',"connexity"    ,AIMS_OPT_FLAG  ,&connexity      ,0                ,0},    
    { ' ',"3D"       	 ,AIMS_OPT_FLAG  ,&graph          ,0                ,0},    
    { 0  ,0              ,AIMS_OPT_END   ,0               ,0                ,0}};

  AimsParseOptions( &argc, argv, opt, usage );

  //
  // read triangulation
  //
  cout << "reading white triangulation   : " << flush;
  AimsSurfaceTriangle surface;
  Reader<AimsSurfaceTriangle> triR( meshfile );
  triR >> surface;
  cout << "done" << endl;
 
  //
  // read input texture
  //
  cout << "reading texture   : " << flush;
  TimeTexture<short>	inpTex; // objects def (labels >0)
  Reader<TimeTexture<short> > texR( intexfile );
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
  ifstream 	tf(sulcitraductionfile);
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
  GyriAndSulci = GyrusModel2GyriAndSulci(model);
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
  Writer<TimeTexture<short> >	texW( outtexfile );
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
  string	base2( graphfile );
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

      cout << "Reading cortex volume   : " << volfile << endl;
      AimsData<short> greyVol;
      Reader<AimsData<short> > triGV( volfile );
      triGV >> greyVol;

      cout << "reading brain triangulation   : " << flush;
      AimsSurfaceTriangle brain;
      Reader<AimsSurfaceTriangle> triB( brainfile );
      triB >> brain;
      cout << "done" << endl;

      cout << "Computing 3D parcellisation" << endl;
      Graph		k("RoiArg");
      AimsData<short>    gyriVol;
      gyriVol = MeshParcellation2Volume( greyVol,outTex[1],surface[0],(short)val_domain,0 );
      braintex[0] = VolumeParcellation2MeshParcellation(gyriVol,brain[0],0);
      t2g.makeGraph(k,brain,braintex[0],trans_inv);

      cout << "Computing the volumic  graph " << graphfile << "\n";
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
          Writer<Graph>	agw2( graphfile  );
          agw2.write( *m );
        }
      catch( exception & e )
        {
          cerr << e.what() << endl;
          throw( e );
        } 

      cout << "Write gyri volume\n";
      Writer<AimsData<short> >	imaW( parcelvolfile );
      imaW <<  gyriVol ;   
  
    }
  else
    {
      Writer<Graph>	agw2( graphfile  );
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

 
