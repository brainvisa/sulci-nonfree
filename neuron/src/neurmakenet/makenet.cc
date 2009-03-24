/*
 *  Copyright (C) 2005 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 */

#include <cstdlib>
#include <neur/mlp/mlp.h>
#include <iostream>
#include <vector>

using namespace std;

void usage( char* name )
{
  cout << "usage: " << name << " output.net n0 n1 [n2...]" << endl;
  cout << "generates a MLP .net file" << endl;
  cout << "n0, n1, ...: numbers of cells on each layer - 2 layers minimum "
       << "are required (inputs and outputs)\n";
  exit( 0 );
}

int main( int argc, char** argv )
{
  if( argc < 4 || string( argv[1] ) == "-h" || string( argv[1] ) == "--help" )
    usage( argv[0] );

  string	fname( argv[1] );
  int		i, n = argc - 2, x;
  vector<int>	layers( n );
  cout << "layers: " << n << endl;
  for( i=0; i<n; ++i )
    {
      if( sscanf( argv[i+2], "%d", &x ) == 0 )
        usage( argv[0] );
      layers[i] = x;
      cout << x << " ";
    }
  cout << endl;
  mlp<double, double>	net( fname.c_str(), n, &layers[0] );
  net.save( fname.c_str() );

  return 0;
}


