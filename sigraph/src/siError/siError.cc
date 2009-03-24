
#include <cstdlib>
#include <si/fold/fattrib.h>
#include <si/global/global.h>
#include <si/fold/foldReader.h>
#include <si/fold/labelsTranslator.h>
#include <aims/getopt/getopt2.h>
#include <cartobase/exception/parse.h>
#include <iostream>

using namespace carto;
using namespace sigraph;
using namespace aims;
using namespace std;


int main( int argc, const char **argv )
{
  try
    {
      AimsApplication	app( argc, argv, "Count labelling differences on " 
                             "auto/manual recognitions on a cortical folds " 
                             "graph. Graph must be labelled manually " 
                             "(\"name\" attribute) and automatically " 
                             "(\"label\" attribute)" );
      string	graphname, labelsname, refgname, autolabel = SIA_LABEL, 
        manuallabel = SIA_NAME;
      app.addOption( graphname, "-i", "data fold graph", true );
      app.addOption( labelsname, "-l", "labels translation file" );
      app.addOption( refgname, "-r", "reference graph name to compare labels " 
                     "with (default: same as input data graph)", true );
      app.addOption( autolabel, "-a", string( "automtic labeling attribute " 
                                              "(default: \"" ) + autolabel 
                     + "\")", true );
      app.addOption( manuallabel, "-m", string( "manual (reference) labeling " 
                                                "attribute (default: \"" ) 
                     + manuallabel + "\")", true );
      app.initialize();

      rc_ptr<FGraph>	fg( new FGraph ), fg2 = fg;

      if( labelsname.empty() )
        labelsname = si().labelsTranslPath();

      try
        {
          FoldReader	fr( graphname );

          fr >> *fg;
          cout << "Data graph read OK.\n";

          FoldLabelsTranslator	ft( labelsname );

          ft.translate( *fg );
          ft.translate( *fg, autolabel, autolabel );

          cout << "Graph translated.\n\n";

          if( !refgname.empty() )
            {
              fg2.reset( new FGraph );
              FoldReader	fr2( refgname );

              fr2 >> *fg2;
              cout << "Reference graph read OK.\n";

              ft.translate( *fg2 );
              ft.translate( *fg, manuallabel, manuallabel );
              cout << "Reference graph translated.\n\n";
            }
        }
      catch( parse_error & e )
        {
          cerr << e.what() << " : " << e.filename() << ", line " 
               << e.line() << endl;
          throw;
        }
      catch( exception & e )
        {
          cerr << e.what() << endl;
          throw;
        }

      FGraph::const_iterator	iv, fv=fg2->end();
      string			name, label;
      unsigned			n = 0, nbr = 0, naj = 0, nrj = 0, nch = 0, 
        nbon = 0;
      float			s, tots = 0, errs = 0, sbr = 0, saj = 0, 
        sch = 0, srj = 0, sbon = 0, sobr = 0;
      map<int, Vertex *>	corresp;
      int			index;
      Vertex			*v;
      float si;

      // build correspondance map fg2 -> fg
      if( fg.get() != fg2.get() )
        for( iv=fg2->begin(); iv!=fv; ++iv )
          if( (*iv)->getProperty( "index", index ) )
            corresp[ index ] = *iv;

      for( iv=fg->begin(), fv=fg->end(); iv!=fv; ++iv )
        {
          if( fg.get() == fg2.get() )
            v = *iv;
          else
            {
              if( !(*iv)->getProperty( "index", index ) )
                {
                  cerr << "warning: node without index - can't retreive " 
                       << "corresponding node in reference graph\n";
                  continue;
                }
              v = corresp[ index ];
              if( !v )
                {
                  cerr << "warning: node without a corresponding node in " 
                       << "reference graph\n";
                  continue;
                }
            }
          if( v->getProperty( manuallabel, name ) && 
              (*iv)->getProperty( autolabel, label) )
            {
              s = 0;
              if( (*iv)->getProperty( SIA_SIZE, s ) )
                tots += s;
              if( label != name 
                  && (label != SIV_VOID_LABEL || name != "brain" ) 
                  && (label != "brain" || name != SIV_VOID_LABEL ) )
                {
                  cout << name << " -> " << label << " (size : " << s << ")\n";
                  errs += s;
                  ++n;
                  if( label == SIV_VOID_LABEL || label == "brain" )
                    {
                      ++nrj;
                      srj += s;
                    }
                  else if( name == SIV_VOID_LABEL || name == "brain" )
                    {
                      ++naj;
                      saj += s;
                      sobr += s;
                    }
                  else
                    {
                      ++nch;
                      sch += s;
                    }
                }
              else if( label == SIV_VOID_LABEL || label == "brain" )
                {
                  sbr += s;
                  sobr += s;
                  ++nbr;
                }
              else
                {
                  ++nbon;
                  sbon += s;
                }
            }
        }

      cout << endl << n << " erreurs sur " << fg->order() << ", donc " 
           << fg->order() - n << " bons : " << (float)n/fg->order() * 100 
           << "% d'erreur\n";
      if( tots > 0 )
      {
        cout << "En masse : " << errs / tots * 100 << "% d'erreur\n";
        si = 200 * sbon / ( 2 * sbon + errs );
        cout << "SI: " << si << "% (" << 100 - si << " % err)" << endl;
      }
      cout << "Masse d'origine des non-brain : " << sobr << endl;

      cout << nbr << " étiquettes 'brain' bien placées ( " 
           << (float)nbr*100/fg->order() << " %, masse : " << sbr 
           << " , " << sbr*100/tots << " % )\n";

      cout << "Ajouts : " << naj << " (brain -> autre) :\n  " 
           << ((float)naj*100)/fg->order() 
           << " % du total, " << ((float)naj*100)/(fg->order()-nbr) 
           << " % des non-brain\n";
      cout << "masse : " << saj << " : " << saj*100/tots << " % du total, " 
           << saj*100/(tots-sbr) << " % des non-brain\n";

      cout << "Rejets : " << nrj << " (autre -> brain) :\n  " 
           << ((float)nrj*100)/fg->order() 
           << " % du total, " << ((float)nrj*100)/(fg->order()-nbr) 
           << " % des non-brain\n";
      cout << "masse : " << srj << " : " << srj*100/tots << " % du total, " 
           << srj*100/(tots-sbr) << " % des non-brain\n";

      cout << "Changements : " << nch << " (n'impliquant pas 'brain') :\n  " 
           << ((float)nch*100)/fg->order() << " % du total, " 
           << ((float)nch*100)/(fg->order()-nbr) << " % des non-brain\n";
      cout << "masse : " << sch << " : " << sch*100/tots << " % du total, " 
           << sch*100/(tots-sbr) << " % des non-brain\n";

      cout << "Bons : " << nbon << " (autre que 'brain', bien placés) :\n  " 
           << ((float)nbon*100)/fg->order() << " % du total, " 
           << ((float)nbon*100)/(fg->order()-nbr) << " % des non-brain\n";
      cout << "masse : " << sbon << " : " << sbon*100/tots << " % du total, " 
           << sbon*100/(tots-sbr) << " % des non-brain, " 
           << sbon*100/(tots-sobr) << " des non-brain d'origine\n";
    }
  catch( user_interruption & )
    {
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
      exit(1);
    }
}



