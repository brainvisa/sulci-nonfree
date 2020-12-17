#include <si/fold/fattrib.h>
#include <si/fold/frgraph.h>
#include <si/fold/frgReader.h>
#include <si/fold/frgWriter.h>
#include <si/model/topAdaptive.h>
#include <aims/getopt/getopt2.h>

using namespace carto;
using namespace sigraph;
using namespace aims;
using namespace std;


int mergemodels( GraphObject* v1, const GraphObject *v2, double fac )
{
  Model	*m1, *m2;
  if( !v1->getProperty( SIA_MODEL, m1 ) || !v2->getProperty( SIA_MODEL, m2 ) 
      || !m1->isAdaptive() || !m2->isAdaptive() )
    return 0;
  TopAdaptive	*ta1 = (TopAdaptive *) m1->topModel();
  TopAdaptive	*ta2 = (TopAdaptive *) m2->topModel();
  if( !ta1 || !ta2 )
    return 0;
  double	con1 = ta1->confidenceFactor();
  double	con2 = ta2->confidenceFactor();

  // cout << con1 << " / " << con2 << endl;
  if( con1 < con2 * fac )
    {
      v1->setProperty( SIA_MODEL, m2->clone() );
      return 1;
    }
  return -1;
}


int main( int argc, const char **argv )
{
  try
    {
      AimsApplication	app( argc, argv, "Merges two model graphss by taking " 
                             "the best elements of each" );
      string	mgraph1name, mgraph2name, outname;
      double	compfac = 1.;
      app.addOption( mgraph1name, "-i", "1st model graph" );
      app.addOption( mgraph2name, "-j", "2nd model graph" );
      app.addOption( outname, "-o", "output model graph" );
      app.addOption( compfac, "-f", "comparison factor: a model from the 2nd " 
                     "graph will replace the one in the 1st one if conf1 < " 
                     "conf2 * factor, conf1 and conf2 being the confidence " 
                     "factors of both models [default: 1]", true );
      app.initialize();

      FRGraph	mg1, mg2;
      FrgReader	mr1( mgraph1name );
      cout << "reading model graph " << mgraph1name << "...\n";
      mr1 >> mg1;
      FrgReader	mr2( mgraph2name );
      cout << "reading model graph " << mgraph2name << "...\n";
      mr2 >> mg2;
      cout << "models read.\n";

      Graph::iterator		iv, jv, ev = mg1.end();
      Vertex::iterator		ie, ee;
      set<Edge *>::iterator	je, fe;
      string			label1, label2, label3;
      Vertex			*v1, *v2;
      Edge			*e1, *e2;
      unsigned			count = 0, changed = 0, abort = 0;
      int			mm;

      // loop on nodes
      for( iv=mg1.begin(); iv!=ev; ++iv )
        if( (*iv)->getProperty( SIA_LABEL, label1 ) )
          {
            // find corresponding node in mg2
            v1 = *iv;
            set<Vertex *>	sv = mg2.getVerticesWith( SIA_LABEL, label1 );
            if( sv.size() == 1 )
              {
                v2 = *sv.begin();
                mm = mergemodels( v1, v2, compfac );
                if( mm == 0 )
                  ++abort;
                else
                  {
                    ++count;
                    if( mm > 0 )
                      {
                        cout << "taking vertex model " << label1 << " from " 
                             << mgraph2name << endl;
                        ++changed;
                      }
                  }

                // relations from v1
                set<Edge *>	se( v2->begin(), fe=v2->end() );
                for( ie=v1->begin(), ee=v1->end(); ie!=ee; ++ie )
                  if( (*ie)->getProperty( SIA_LABEL1, label2 ) 
                      && label2 == label1 
                      && (*ie)->getProperty( SIA_LABEL2, label2 ) )
                    {
                      // find corresponding edge from v2
                      e1 = *ie;
                      for( je=se.begin(), fe=se.end(); je!=fe; ++je )
                        if( (*je)->getProperty( SIA_LABEL1, label3 ) 
                            && label3 == label1 
                            && (*je)->getProperty( SIA_LABEL2, label3 ) 
                            && label3 == label2 )
                          {
                            e2 = *je;
                            mm = mergemodels( e1, e2, compfac );
                            if( mm == 0 )
                              ++abort;
                            else
                              {
                                ++count;
                                if( mm > 0 )
                                  {
                                    cout << "taking edge model " << label1 
                                         << "-" << label2 << " from " 
                                         << mgraph2name << endl;
                                    ++changed;
                                  }
                              }
                            se.erase( je );
                            break;
                          }
                    }
              }
            else if( sv.size() >= 2 )
              throw runtime_error( string( "several nodes of label " ) 
                                   + label1 + " in model graph " 
                                   + mgraph2name );
          }

      FrgWriter	mw( outname );
      mw << mg1;

      cout << "kept " << count - changed << " (" 
           << float(count - changed) * 100 / count 
           << "%) from M1, replaced with " 
           << changed << " (" << float(changed) * 100 / count 
           << "%) from M2, could not compare " << abort << " (" 
           << float(abort) * 100 / (count + abort) << "%) models" << endl;
    }
  catch( user_interruption & )
    {
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
    }
}


