#include <cstdlib>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

int main( int argc, char** argv )
{
  if( argc < 3 )
    {
      cerr << "usage : " << argv[0] << " PIDfile command [params...]\n\n";
      cerr << "execs command 'command' with given parameters and writes the ";
      cerr << "process ID to file PIDfile\n";
      exit( 1 );
    }

  ofstream	f( argv[1] );
  if( !f )
    {
      cerr << "cannot open file " << argv[1] << endl;
      exit( 1 );
    }
  f << getpid() << endl << flush;
  f.close();
  cout << "pid : " << getpid() << endl;

  cout << "exec " << argv[2] << endl;

  execvp( argv[2], argv+2 );

  cerr << "exec foiré\n";
  // effacer le fichier
  remove( argv[1] );
  return( 1 );
}


