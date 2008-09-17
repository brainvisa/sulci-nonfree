/*
 *  Copyright (C) 2002-2005 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 */

#include <neur/mlp/mlp.h>
#include <neur/mlp/pat.h>
#include <iostream>
#include <string>
#include <unistd.h>

using namespace std;

void usage( const string & name )
{
  cout << "usage: " << name << " [-b] [-r] [-e eta] mlp_network.net " 
       << "patterns.pat ncycles" << endl;
  cout << "(re-writes the input network)\n";
  cout << "-b: boolean mode: also print errors in binary classifier mode\n";
  cout << "-r: reset network before learning (otherwise continues previous " 
       << "learning\n";
  cout << "-e eta: learn factor [default: 0.1]\n";
  exit( 1 );
}

int main( int argc, char** argv )
{
  bool	boolean = false, reset = false;
  int	opt;
  float eta = 0.1;

  while( (opt = getopt( argc, argv, "bre:" ) ) != EOF )
    switch( opt )
      {
      case 'b':
	boolean = true;
	break;
      case 'r':
	reset = true;
	break;
      case 'e':
	if( sscanf( optarg, "%f", &eta ) == 0 )
          usage( argv[0] );
	break;
      default:
	usage( argv[0] );
	break;
      }

  if( argc - optind != 3 )
    usage( argv[0] );

  string	netfile = argv[ optind++ ];
  string	patfile = argv[ optind++ ];
  unsigned	cycles = atoi( argv[ optind++ ] );

  cout << "network: " << netfile << endl;

  mlp<double,double>	netw;
  if( netw.load( netfile.c_str() ) )
    {
      cerr << "could not load network " << netfile << endl;
      return 1;
    }

  cout << "blabla\n";
  cout << "patterns: " << patfile << endl;
  cout << "cycles  :" << cycles << endl;
  pat<double>		patt( patfile.c_str() );
  if( patt.np() == 0 )
    {
      cerr << "could not load patterns " << patfile << endl;
      return 1;
    }
  cout << "pattern file loaded.\n";

  double	serr = 0, err0 = 0, err1 = 0, o, e, d;
  unsigned	nerr0 = 0, nerr1 = 0, nerrh = 0, n0 = 0, n1 = 0;
  int		i;

  if( reset )
    netw.rand_w( -1, 1 );

  if( netw.backprop( patt, cycles, eta, 1 ) )
    {
      cerr << "Perceptron learning failed" << endl;
      return 1;
    }

  if( netw.save( netfile.c_str() ) )
    {
      cerr << "could not save network " << netfile << endl;
      return 1;
    }

  for( i=0; i<patt.np(); ++i )
    {
      netw.prop( patt, i );
      o = netw.u()[ netw.fo() ].o();
      e = patt.vo()[i][0];
      d = fabs( e - o );
      serr += d;
      if( d >= 0.5 )
	++nerrh;
      if( e < 0.5 )
	{
	  ++n0;
	  err0 += d;
	  if( o >= 0.5 )
	    ++nerr0;
	}
      else
	{
	  ++n1;
	  err1 += d;
	  if( o < 0.5 )
	    ++nerr1;
	}
    }
  cout << "errors :" << endl;
  cout << "total                            : " << serr << " ( " 
       << serr / (n0+n1) << " /sample )" << endl;
  cout << "num of samples with err >= 0.5   : " << nerrh << " / " 
       << n0+n1 << " ( " << ((float)nerrh)*100/(n0+n1) << " % )" << endl;
  if( boolean )
    {
      cout << "sum error on class 0             : " << err0 << " ( " 
	   << err0 / n0 << " /sample )" << endl;
      cout << "sum error on class 1             : " << err1 << " ( " 
	   << err1 / n1 << " /sample )" << endl;
      cout << "misclassified samples of class 0 : " << nerr0 << " / " 
	   << n0 << " ( " << ((float)nerr0)*100/n0 << " % )" << endl;
      cout << "misclassified samples of class 1 : " << nerr1 << " / " 
	   << n1 << " ( " << ((float)nerr1)*100/n1 << " % )" << endl;
    }
  return 0;
}


