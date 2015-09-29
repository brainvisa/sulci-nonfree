#include <cstdlib>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>

using namespace std;

void usage( char* name )
{
  cerr << "usage :\n";
  cerr << name << " [-p prefix] n directory savefile\n";
  cerr << name << " [-p prefix] -c directory savefile n1 n2...\n\n";
  cerr << "Divise une liste de modèles (fichiers .mod dans le directory\n";
  cerr << " donné) en n expressions régulières. Les expressions crées sont\n";
  cerr << " sauvées dans les fichiers savefile.0, savefile.1 ...\n";
  cerr << "savefile.(n-1)\n\n";
  cerr << "    -p   enlève le préfixe donné aux noms de fichiers avant \n";
  cerr << "         de construire les expressions régulières\n";
  cerr << "    -c   mode cpu, donne une liste d'indices correspondant chacun\n"
       << "         à la puissance CPU de la machine concernée\n";
  exit( 1 );
}


int main( int argc, char** argv )
{
  string	prefix;

  int	c;
  bool	errorflg = false;
  bool	cpumode = false;

  while( ( c = getopt( argc, argv, "pch" ) ) != EOF )
    switch( c )
      {
      case 'c':
	cpumode = true;
	break;
      case 'p':
	prefix = argv[optind];
	cout << "prefix : " << prefix << endl;
	++optind;
	break;
      case 'h':
      case '?':
	errorflg = true;
      }
  if( errorflg ) usage( argv[0] );

  if( ( !cpumode && argc-optind != 3 ) || (cpumode && argc-optind < 3) )
    usage( argv[0] );

  unsigned	n = 0, nexpert = 0, i;
  if( !cpumode )
    {
      sscanf( argv[ optind ], "%d", &n );
      ++optind;
    }

  string		dir = argv[ optind++ ];
  string		save = argv[ optind++ ];
  map<string, unsigned>	str;
  string		s;
  vector<float>		cpus;
  float			num, sum = 0;

  //cout << "dir: " << dir << "\nsave : " << save << endl;

  if( cpumode )
    {
      //cout << "cpumode, " << argc - optind << " cpus" << endl;
      while( argc > optind )
	{
	  num = 0;
	  sscanf( argv[optind++], "%f", &num );
	  if( num > 0 )
	    {
	      cpus.push_back( num );
	      sum += num;
	    }
	  else cerr << "warning bad CPU power specified (" << argv[optind-1] 
		    << ")" << endl;
	}
      n = cpus.size();
    }
  else
    {
      for( i=0; i<n; ++i )
	cpus.push_back( 1. );
      sum = n;
    }

  cout << "machines : " << n << ", sum power : " << sum << endl;
  cout << "repartition:" << endl;
  for( i=0; i<n; ++i )
    {
      cpus[i] /= sum;
      cout << cpus[i] * 100 << "% ";
    }
  cout << endl;

  //	lecture directory
  struct dirent	*dent;
  DIR		*d = opendir( dir.c_str() );

  if( !d )
    {
      cerr << "cannot open directory " << dir << endl;
      exit( EXIT_FAILURE );
    }

  while( ( dent = readdir( d ) ) != 0 )
    {
      s = dent->d_name;
      if( ( prefix.empty() || s.find( prefix ) == 0 ) 
	  && s.rfind( ".mod" ) == s.size() - 4 )
	{
	  if( !prefix.empty() )
	    s.erase( 0, prefix.size() );
	  s.erase( s.size() - 4, 4 );
	  string::size_type f = s.find( "right-" ); // cherche un séparateur de relation
	  if( f != string::npos )
	    f += 5;	// se placer sur le '-'
	  else
	    {
	      f = s.find( "left-" );
	      if( f != string::npos )
		f += 4;	// se placer sur le '-'
	    }
	  if( f != string::npos )
	    s.erase( f, s.size() - f );
	  if( s != "unknown" )
	    {
	      ++str[ s ];
	      // cout << s << endl;
	      ++nexpert;
	    }
	}
    }

  cout << "tout lu : " << str.size() << " , nb experts : " << nexpert << endl;

  map<string, unsigned>::const_iterator	is, fs = str.end();
  vector<string>	rexp(n);
  unsigned	wt;

  is=str.begin();
  for( unsigned j=0; j<n; ++j )
    {
      for( wt=0; wt < cpus[j] * nexpert && is != fs; ++is )
	{
	  wt += (*is).second;
	  if( !rexp[j].empty() )
	    rexp[j] += "|";
	  rexp[j] += "^" + (*is).first + "$";
	}
      if( is != fs )
	cout << "coupe avant : " << (*is).first << endl;
      cout << "paquet " << j << " : n : " << wt << endl;

      char	fname[ save.size() + 5 ];
      sprintf( fname, "%s.%d", save.c_str(), j );
      ofstream	of( fname );
      if( !of )
	{
	  cerr << "cannot open " << fname << " for writing" << endl;
	  exit( EXIT_FAILURE );
	}
      if( n == 1 )	// simplification
	rexp[j] = "^.*$";
      of << rexp[j] << endl;
      of.close();
    }
  cout << "OK" << endl;
}
