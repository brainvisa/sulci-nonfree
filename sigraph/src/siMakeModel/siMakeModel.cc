
#include <cstdlib>
#include <si/fold/foldLabels.h>
#include <si/fold/fdParser.h>
#include <si/model/mReader.h>
#include <si/domain/domReader.h>
#include <si/fold/frgWriter.h>
#include <si/fold/frgReader.h>
#include <si/fold/domainBox.h>
#include <si/fold/domainBox2.h>
#include <si/fold/inertialDomainBox.h>
#include <si/fold/fattrib.h>
#include <cartobase/config/version.h>
#include <cartobase/stream/fileutil.h>
#include <aims/getopt/getopt2.h>
#include <iostream>

using namespace sigraph;
using namespace aims;
using namespace carto;
using namespace std;


int main( int argc, const char** argv )
{
  try
    {
      AimsApplication	app( argc, argv, 
                             "Creates a sulcal model graph" );
      string	modelname, labelsfile, adapfile, domfile, defmodfile, 
        defrelfile, ver, dataver;
      app.addOption( modelname, "-o", "output model filename (.arg)" );
      app.addOption( labelsfile, "-l", "labels translation filename" );
      app.addOption( adapfile, "-m", "template node model filename (.mod)" );
      app.addOption( domfile, "-d", "template domain filename (.dom)" );
      app.addOption( defmodfile, "-f", "fallback node model filename (.mod) " 
                     "used for unknown situations ('-' can be specified for " 
                     "none for compatibility with older sigraph releases)", 
                     true );
      app.addOption( defrelfile, "-r", "fallback relation model filename " 
                     "(.mod)", true );
      app.addOption( ver, "--mversion", string( "model version [default: " ) 
                     + cartobaseShortVersion() + "]", true );
      app.addOption( dataver, "--dversion", "data graphs compatibility " 
                     "version [default: 3.1]", true );

      app.initialize();

      //	Read Model
      Model		*ad = 0;
      MReader	& ar = FrgReader::defaultMReader();
      ar.open( adapfile );
      FDParser	fp;
      ar.addFactories( fp.factories() );
      ad = ar.readModel();
      assert( ad );
      ar.close();

      //	read default model
      Model		*defMod = 0;
      if( !defmodfile.empty() && defmodfile != "-" )
        {
	  ar.open( defmodfile );
	  defMod = ar.readModel();
	  ar.close();
        }

      //	read Domain
      Domain	*dom = 0;

      DomReader	dr( domfile );
      dom = dr.readDom();
      assert( dom );

      //	read labels
      FoldLabels	fl( labelsfile );

      //	create du graphe
      MGraph	*mg = fl.createModel( ad, dom, defMod );
      assert( mg );
      //cout << "model graph : " << mg << endl;
      if( !ver.empty() )
        mg->setProperty( SIA_MODEL_VERSION, ver );
      if( !dataver.empty() )
        mg->setProperty( SIA_MODEL_COMPAT_DATA_VERSION, dataver );
      mg->setProperty( "filename_base", string( "*" ) );

      //	read fallback relations model
      Model		*defRelMod = 0;
      if( !defrelfile.empty() )
        {
	  ar.open( defrelfile );
	  defRelMod = ar.readModel();
	  ar.close();

          string mname = FileUtil::basename( defrelfile );
          mg->setProperty( SIA_MODEL, (Model*) defRelMod );
          mg->setProperty( SIA_FAKEREL_MODEL, defRelMod );
          mg->setProperty( SIA_MODEL_FILE, mname );
        }

      //	write model graph
      cout << "Writing model...\n";
      FrgWriter	rw( modelname );
      cout << "graph name : " << modelname << endl;
      //cout << "(" << mg << ")" << endl;
      cout << "nodes : " << mg->order() << endl;
      cout << "rels  : " << mg->size() << endl;
      rw << *mg;
      cout << "OK\n";

      delete mg;
      delete defMod;
      delete dom;
      delete ad;

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




