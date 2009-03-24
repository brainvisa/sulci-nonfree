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

#include <cstdlib>
#include <neur/mlp/pat.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <float.h>
#include <stdlib.h>

using namespace std;

void usage( const string & name )
{
  cout << "usage: " << name << " [-r] input [output] [norm_patters]\n";
  cout << "default output: input.pat\n";
  cout << "imports an ASCII text file and converts it to a SNNS pattern file " 
       << "(.pat)\n";
  cout << "-r            :  rescale data\n";
  cout << "norm_patterns :  use normalization figures from given file\n";
  exit( 1 );
}


void badfile( const string & file )
{
  cerr << file << " : unexpected EOF.\n";
  exit( 1 );
}


int main( int argc, char** argv )
{
  bool	rescale = false;
  int	opt;

  while( (opt = getopt( argc, argv, "rh" ) ) != EOF )
    switch( opt )
      {
      case 'r':
        rescale = true;
        break;
      case 'h':
      default:
	usage( argv[0] );
	break;
      }

  int	nopt = argc - optind;
  if( nopt < 1 || nopt > 3 )
    usage( argv[0] );
  string	in = argv[optind++], out, normpat;
  if( nopt >= 2 )
    out = argv[optind++];
  else
    out = in;
  if( out.rfind( ".pat" ) == string::npos )
    out += ".pat";
  if( nopt >= 3 )
    {
      normpat = argv[optind++];
      cout << "normalization samples: " << normpat << endl;
    }

  ifstream	f( in.c_str() );
  if( !f )
    {
      cerr << "can't read " << in << endl;
      exit( 1 );
    }

  
  string		s;
  {
    vector<char>	toto( 10000 );
    f.getline( &toto[0], toto.size() );
    s = &toto[0];
  }
  //cout << s << endl;
  int			ni = 0, no = 1, i, u;
  vector<double>	values, mins, maxs;
  double		output, omin, omax;
  bool			end = false;

  stringstream		ss( s );
  ss >> output;
  cout << "out: " << output << endl;
  while( !ss.eof() )
    {
      ss >> omin;
      cout << "read: " << omin << endl;
      //if( !ss.eof() )
	{
	  values.push_back( omin );
	  mins.push_back( omin );
	  maxs.push_back( omin );
	  ++ni;
	}
    }

  cout << "inputs : " << ni << endl;

  omin = output;
  omax = output;

  pat<double>		p( "pattern", 0, ni, no );
  p.add( &values[0], &output );

  while( !f.eof() )
    {
      f >> output;
      if( output < omin )
	omin = output;
      else if( output > omax )
	omax = output;
      for( i=0; i<ni; ++i )
	{
	  if( f.eof() )
	    {
	      end = true;
	      if( i != 0 )
		badfile( in );
	      else
		break;
	    }
	  f >> values[i];
	  if( values[i] < mins[i] )
	    mins[i] = values[i];
	  else if( values[i] > maxs[i] )
	    maxs[i] = values[i];
	}
      if( !end )
	p.add( &values[0], &output );
    }
  f.close();

  cout << "patterns: " << p.np() << endl;

  //	rescale on other patterns set
  if( !normpat.empty() )
    {
      ifstream	g( normpat.c_str() );
      unsigned	nn = 0;
      if( !g )
	{
	  cerr << "can't read " << in << endl;
	  exit( 1 );
	}

      omin = FLT_MAX;
      omax = -FLT_MAX;
      for( i=0; i<ni; ++i )
	{
	  mins[i] = FLT_MAX;
	  maxs[i] = -FLT_MAX;
	}

      while( !g.eof() )
	{
	  g >> output;
	  if( output < omin )
	    omin = output;
	  else if( output > omax )
	    omax = output;
	  for( i=0; i<ni; ++i )
	    {
	      if( g.eof() )
		{
		  end = true;
		  if( i != 0 )
		    badfile( normpat );
		  else
		    break;
		}
	      g >> values[i];
	      if( values[i] < mins[i] )
		mins[i] = values[i];
	      else if( values[i] > maxs[i] )
		maxs[i] = values[i];
	    }
	  ++nn;
	}
      cout << "renorm samples: " << nn << endl;
    }

  // rescale
  if( rescale )
    {
      vector<double>	scale( ni );
      for( i=0; i<ni; ++i )
        if( mins[i] != maxs[i] )
          scale[i] = 2. / ( maxs[i] - mins[i] );
        else
          scale[i] = 1;
    
      for( u=0; u<p.np(); ++u )
        {
          for( i=0; i<ni; ++i )
            p.vi()[u][i] =  ( p.vi()[u][i] - mins[i] ) * scale[i] - 1;
          if( omin != omax )
            p.vo()[u][0] = ( p.vo()[u][0] - omin ) / ( omax - omin );
        }
    }

  p.save( out.c_str() );
}

