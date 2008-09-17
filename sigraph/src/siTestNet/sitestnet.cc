
#include <aims/getopt/getopt2.h>
#include <si/subadaptive/subAdMlp.h>
#include <si/model/mReader.h>
#include <si/fold/fdParser.h>
#include <si/model/adaptiveTree.h>
#include <si/model/adaptiveLeaf.h>
#include <si/model/topAdaptive.h>
#include <si/subadaptive/subAdaptive.h>
#include <cartobase/stream/fileutil.h>

using namespace sigraph;
using namespace aims;
using namespace carto;
using namespace std;


SubAdaptive* subAd( Model* model )
{
  Model		*mod = model;
  AdaptiveLeaf	*al;
  AdaptiveTree	*at;
  TopAdaptive	*ta;
  SubAdaptive	*sa = 0;

  while( mod && mod->isAdaptive() )
    {
      al = dynamic_cast<AdaptiveLeaf *>( mod );
      if( al )
	{
	  sa = &al->workEl();
	  mod = 0;
	}
      else
	{
	  at = dynamic_cast<AdaptiveTree *>( mod );
	  if( at )
	    mod = *at->begin();
	  else
	    {
	      ta = dynamic_cast<TopAdaptive *>( mod );
	      if( ta )
		mod = ta->model();
	      else
		mod = 0;
	    }
	}
    }
  return sa;
}


int main( int argc, const char** argv )
{
  try
    {
      AimsApplication	app( argc, argv, "Use a neural network to write its " 
                             "outputs in a file" );

      string	netfile, outfile, datafile;
      unsigned	n = 0;
      app.addOption( netfile, "-i", "input network file (MLP .net, " 
                     "or sigraph generic model .mod)" );
      app.alias( "--input", "-i" );
      app.addOption( datafile, "-d", "input data file (ASCII, one line per " 
                     "vector)" );
      app.alias( "--data", "-d" );
      app.addOption( outfile, "-o", "output result file" );
      app.alias( "--outut", "-o" );
      app.addOption( n, "-n", "number of inputs on each vector " 
                     "(default: automatic)", true );

      app.initialize();

      cout << "reading network\n";

      SubAdaptive	*sa = 0;
      Model		*model = 0;
      string		ext = FileUtil::extension( netfile );
      if( ext == "net" )
        sa = new SubAdMlp( netfile, netfile, netfile );
      else
        {
          MReader	ar( netfile );
          FDParser	fp;
          ar.addFactories( fp.factories() );
          model = ar.readModel();
          if( model )
            {
              sa = subAd( model );
              if( !sa )
                cerr << "No SubAdaptive\n";
              else
                cout << "inputs number : " << sa->inputs().size() << endl;
              if( n == 0 )
                n = sa->inputs().size();
            }
          else
            throw runtime_error( "Model reading failed" );
        }

      cout << "reading data...\n";

      vector<vector<double> >	data;
      ifstream	df( datafile.c_str() );
      unsigned	i, l = 0;
      double	x;
      string	line;
      int	c = ' ';

      while( !df.eof() )
        {
          ++l;
          line.clear();
          while( !df.eof() && ( c = df.get() ) != '\n' && c != '\0' )
            line += (char) c;
          //cout << "line: " << l << ": " << line << endl;
          //cout << "char: " << c << ", eof: " << df.eof() << endl;
          if( df.eof() )
            break;
          istringstream	ss( line.c_str() );
          i = 0;
          while( !ss.eof() && (n == 0 || i < n ) )
            {
              ++i;
              ss >> x;
              // cout << i << ": " << x << endl;
              if( data.size() < l )
                data.push_back( vector<double>() );
              data[l-1].push_back( x );
            }
          if( n == 0 )
            n = i;
          else if( i != n )
            {
              ostringstream	ss;
              ss << "mismatch in vector dimensions, line " << l << ": " 
                 << i << " elements found, " << n << " expected";
              throw runtime_error( ss.str() );
            }
        }
      l = data.size();
      cout << "data: " << l << " vectors of size " << n << endl;

      //
      for( i=0; sa->inputs().size()<n; ++i )
        sa->addInput( i );
      sa->resetStats();

      cout << "processing...\n";

      ofstream	out( outfile.c_str() );
      for( i=0; i<l; ++i )
        out << sa->prop( data[i] ) << endl;

      if( model )
        delete model;
      else
        delete sa;

      cout << "done\n";
    }
  catch( user_interruption & )
    {
    }
  catch( exception & e )
    {
      cerr << argv[0] << ": " << e.what() << endl;
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

