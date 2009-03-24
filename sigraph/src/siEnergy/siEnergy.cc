
#include <cstdlib>
#include <si/global/global.h>
#include <si/fold/frgReader.h>
#include <si/fold/foldReader.h>
#include <si/graph/anneal.h>
#include <si/finder/modelFinder.h>
#include <aims/getopt/getopt2.h>
#include <cartobase/config/verbose.h>
#include <iostream>

using namespace sigraph;
using namespace carto;
using namespace aims;
using namespace std;


int main( int argc, const char** argv )
{
  try
    {
      AimsApplication	app( argc, argv, "Energy of a labelled graph\n" 
                             "In verbose mode, potentials of each individual " 
                             "clique is displayed" );
      string	graphname, mgname, transl;
      float	wt_fac = 0;

      app.addOption( mgname, "-m", "model graph" );
      app.addOption( graphname, "-i", "data graph" );
      app.addOption( wt_fac, "-w", "weight factor (0: keep as is (default), " 
          "<0: remove existing weights according to neighbours number)", true );
      app.addOption( transl, "-t", "labels translation file", true );
      app.initialize();

      FRGraph	model;
      FGraph	graph;

      cout << "weight_factor : " << wt_fac << endl;

      try
        {
          FrgReader	mr( mgname );
          mr >> model;
        }
      catch( exception & e )
        {
          cerr << e.what() << endl;
          exit( 1 );
        }
      try
        {
          FoldReader	fr( graphname );
          fr >> graph;
        }
      catch( exception & e )
        {
          cerr << e.what() << endl;
          exit( 1 );
        }

      if( !transl.empty() )
        si().setLabelsTranslPath( transl );

      if( wt_fac > 0 )
        model.setWeights( wt_fac );
      else if( wt_fac < 0 )
        {
          cout << "remove weights\n";
          model.removeWeights();
        }

      try
        {
          model.modelFinder().initCliques( graph );
        }
      catch( exception & e )
        {
          cerr << e.what() << endl;
          exit( 1 );
        }

      Anneal	an( graph, model );
      //an.init();

      an.processAllPotentials();
      cout << "Total graph energy : " << an.initialEnergy() << endl;
      if( carto::verbose )
        {
          const set<Clique *>		& cl = graph.cliques();
          set<Clique *>::const_iterator	ic, ec = cl.end();
          string			label1, label2;
          double			pot;
          
          for( ic=cl.begin(); ic!=ec; ++ic )
            {
              if( (*ic)->getProperty( "label", label1 ) )
                cout << "Clique: " << label1 << " : ";
              else if( (*ic)->getProperty( "label1", label1 ) 
                       && (*ic)->getProperty( "label2", label2 ) )
                cout << "Clique: " << label1 << "-" << label2 << " : ";
              else cout << "Clique: <unnamed> : ";
              if( (*ic)->getProperty( "potential", pot ) )
                cout << pot << endl;
              else
                cout << "<no potential>" << endl;
            }
        }
    }
  catch( user_interruption & )
    {
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
    }
}


