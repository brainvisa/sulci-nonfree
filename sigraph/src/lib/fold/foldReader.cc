#include <si/global/global.h>
#include <si/fold/foldReader.h>
#include <si/model/mReader.h>
#include <graph/graph/graph.h>
#include <aims/bucket/bucket_g.h>
#include <aims/def/general.h>
#include <aims/def/path.h>
#include <si/fold/fattrib.h>
#include <cartobase/smart/rcptr.h>
#include <cartobase/exception/file.h>
#include <cartobase/exception/ioexcept.h>

using namespace sigraph;
using namespace aims;
using namespace carto;
using namespace std;


SyntaxSet	FoldReader::syntax( FoldReader::initSyntax
				    ( Path::singleton().syntax() + 
				      "/graph.stx" ) );


FoldReader::FoldReader( const string & filename ) 
  : ExoticGraphReader( filename, FoldReader::syntax )
{
  if( syntax.empty() )
    initSyntax( si().basePath() + "/config/fold.stx" );
}


FoldReader::~FoldReader()
{
}


void FoldReader::read( FGraph & gr, long subobj )
{
  gr.clearAll();

  ExoticGraphReader::read( gr );
  if( gr.getSyntax() != "CorticalFoldArg" )
    {
      cerr << "Input graph is not a cortical folds graph" << endl;
      throw wrong_format_error( "Input is not a CorticalFoldArg", name() );
    }
  if( subobj != 0 )
    {
      gr.loadBuckets( name(), ( subobj < 0 ) );
    }
}


void FoldReader::parse( Graph & /*sg*/, AttributedObject* go )
{
  string		nm, synt, filename;

  synt = go->getSyntax();

  if( synt == SIA_GRAPH_SYNTAX )	// nouvelle version, nouvelle syntaxe
    {
      string	ver;

      cout << "Reading FGraph version ";
      //	détection de la version
      if( !go->getProperty( SIA_VERSION, ver ) )
	{
	  ver = SIV_VERSION_0_9;
	  go->setProperty( SIA_VERSION, string( SIV_VERSION_0_9 ) );
	}
      cout << ver << endl;
    }

  else if( synt == SIA_0_8_GRAPH_SYNTAX )	// vieilles version ( <= 0.8 )
    {
      go->setProperty( SIA_VERSION, string( SIV_VERSION_0_8 ) );

      vector<float> 	rot;
      bool		tal_ok = false;

      if( go->getProperty( SIA_TALAIRACH_ROTATION, rot ) 
	  && rot.size() == 9 )
	{
	  vector<float>	scale;

	  if( go->getProperty( SIA_TALAIRACH_SCALE, scale ) 
	      && scale.size() == 3 
	      && go->getProperty( SIA_TALAIRACH_TRANSLATION, scale ) 
	      && scale.size() == 3 )
	    {
	      vector< vector<float> >	mrot;

	      mrot.push_back( vector<float>() );
	      mrot.push_back( vector<float>() );
	      mrot.push_back( vector<float>() );

	      mrot[0].push_back( rot[0] );
	      mrot[0].push_back( rot[1] );
	      mrot[0].push_back( rot[2] );

	      mrot[1].push_back( rot[3] );
	      mrot[1].push_back( rot[4] );
	      mrot[1].push_back( rot[5] );

	      mrot[2].push_back( rot[6] );
	      mrot[2].push_back( rot[7] );
	      mrot[2].push_back( rot[8] );

	      go->setProperty( SIA_0_8_TALAIRACH_M_ROTATION, mrot );
	      tal_ok = true;
	    }
	}
      if( !tal_ok )
	cerr << "Warning: Talairach transform not present or unusable." << endl;
    }

  else	// pas graphe, peut-être noeuds

    {
#if 0	// obsolete, older version 0.8 not supported anymore
      string	ver;
      sg.getProperty( SIA_VERSION, ver );
      FGraph	& fg = (FGraph &) sg;

      if( ver == SIV_VERSION_0_8 )	// on lit les buckets seuleemnt pour 
	{				// la vieille version 0.8
	  if( go->getProperty( SIA_SS_FILENAME, filename ) )
	    {
	      fg.readBucket( go, SIA_SS_BUCKET, name(), filename );
	      computeGravCenter( sg, go, SIA_SS_BUCKET );
	    }

	  if( go->getProperty( SIA_BOTTOM_FILENAME, filename ) )
	    fg.readBucket( go, SIA_BOTTOM_BUCKET, name(), filename );

	  if( go->getProperty( SIA_JUNCTION_FILENAME, filename ) )
	    {
	      fg.readBucket( go, SIA_JUNCTION_BUCKET, name(), filename );
	      computeGravCenter( sg, go, SIA_JUNCTION_BUCKET );
	    }

	  if( go->getProperty( SIA_CORTICAL_FILENAME, filename ) )
	    {
	      fg.readBucket( go, SIA_CORTICAL_BUCKET, name(), filename );
	      computeGravCenter( sg, go, SIA_CORTICAL_BUCKET );
	    }

	  vector<float>	grav;
	  float		x, y, z;

	  if( go->getProperty( SIA_GRAVITY_CENTER, grav ) 
	      && grav.size() == 3 )
	    {
	      vector< vector<float> >	mrot;
	      vector<float>			scale, transl, tg, vsz;

	      if( sg.getProperty( SIA_0_8_TALAIRACH_M_ROTATION, mrot ) 
		  && mrot.size() == 3 
		  && sg.getProperty( SIA_TALAIRACH_SCALE, scale ) 
		  && sg.getProperty( SIA_TALAIRACH_TRANSLATION, transl ) )
		{
		  x = grav[0] + transl[0];
		  y = grav[1] + transl[1];
		  z = grav[2] + transl[2];

		  tg.push_back( ( mrot[0][0] * x 
				  + mrot[0][1] * y
				  + mrot[0][2] * z ) 
				* scale[0] );
		  tg.push_back( ( mrot[1][0] * x 
				  + mrot[1][1] * y
				  + mrot[1][2] * z ) 
				* scale[1] );
		  tg.push_back( ( mrot[2][0] * x
				  + mrot[2][1] * y 
				  + mrot[2][2] * z ) 
				* scale[2] );

		  // normale dans Talairach

		  vector<float>	norm, ntal;

		  if( go->getProperty( SIA_NORMAL, norm ) 
		      && norm.size() == 3 )
		    {
		      ntal.push_back( ( mrot[0][0] * norm[0] 
					+ mrot[0][1] * norm[1] 
					+ mrot[0][2] * norm[2] ) * scale[0] );
		      ntal.push_back( ( mrot[1][0] * norm[0] 
					+ mrot[1][1] * norm[1] 
					+ mrot[1][2] * norm[2] ) * scale[0] );
		      ntal.push_back( ( mrot[2][0] * norm[0] 
					+ mrot[2][1] * norm[1] 
					+ mrot[2][2] * norm[2] ) * scale[0] );
		      float d = sqrt( ntal[0] * ntal[0] + ntal[1] * ntal[1] 
				      + ntal[2] * ntal[2] );
		      if( d != 0 )
			{
			  ntal[0] /= d;
			  ntal[1] /= d;
			  ntal[2] /= d;
			}
		      //  go->setProperty( "normal_Tal", ntal ); !! CHANGE
		      go->setProperty( SIA_REFNORMAL, ntal );
		    }
		}
	      else	// pas de Talairach : prendre l'identité
		{
		  tg.push_back( grav[0] );
		  tg.push_back( grav[1] );
		  tg.push_back( grav[2] );
		}

	      //  go->setProperty( "gravity_center_Tal", tg );	!! CHANGE
	      go->setProperty( SIA_REFGRAVITY_CENTER, tg );
	    }
	}
#endif
    }
}


void FoldReader::computeGravCenter(  Graph & sg, AttributedObject* go, 
				     const string & attrib )
{
  rc_ptr<BucketMap<Void> >		bck;

  if( !go->getProperty( attrib, bck ) )
    return;

  BucketMap<Void>::Bucket::const_iterator	ib, fb = (*bck)[0].end();
  vector<float>					g;
  unsigned					n = 0;

  g.push_back( 0. );
  g.push_back( 0. );
  g.push_back( 0. );

  for( ib=(*bck)[0].begin(); ib!=fb; ++ib, ++n )
    {
      const AimsVector<short,3> & loc = ib->first;

      g[0] += loc[0];
      g[1] += loc[1];
      g[2] += loc[2];
    }

  if( n > 0 )
    {
      g[0] /= n;
      g[1] /= n;
      g[2] /= n;
    }

  vector<float>	vsz;

  if( sg.getProperty( SIA_VOXEL_SIZE, vsz ) && vsz.size() == 3 )
    {
      g[0] *= vsz[0];
      g[1] *= vsz[1];
      g[2] *= vsz[2];
    }

  go->setProperty( SIA_GRAVITY_CENTER, g );
}


// ---------------


LowLevelFoldArgReader::LowLevelFoldArgReader()
  : LowLevelArgReader()
{
}


LowLevelFoldArgReader::~LowLevelFoldArgReader()
{
}


Graph* LowLevelFoldArgReader::read( const string & filename, int subobj )
{
  FoldReader	r( filename );
  FGraph	*g = new FGraph;
  try
    {
      r.read( *g, subobj );
    }
  catch( exception & )
    {
      delete g;
      throw;
    }

  return g;
}
