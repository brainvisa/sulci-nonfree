/*
 *  Copyright (C) 1998-2004 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 */

#include <cstdlib>
#include <si/global/global.h>
#include <graph/graph/graph.h>
#include <graph/graph/greader.h>
#include <graph/graph/gwriter.h>
#include <cartobase/object/sreader.h>
#include <cartobase/plugin/plugin.h>
#include <aims/def/path.h>
#include <iostream>

using namespace sigraph;
using namespace aims;
using namespace carto;
using namespace std;

void usage( const char* name )
{
  cerr << "usage : \n" << name << " namedGraphFile unnamedGraphFile " 
       << "[newFile=unnamedGraphFile]\n";
  cerr << "\ncopies names / labels from an old or incomplete graph file\n";
  cerr << "to a new one without names / labels\n";
  cerr << endl;

  exit( 1 );
}


int main( int argc, char** argv )
{
  PluginLoader::load();

  if( argc != 3 && argc != 4 )
    usage( argv[0] );

  string	sname = Path::singleton().syntax() + "/graph.stx";

  try
    {
      
      SyntaxReader	sr( sname );
      SyntaxSet	syntax;

      sr >> syntax;

      GraphReader	grn( argv[1], syntax );
      Graph		gnamed;

      grn >> gnamed;

      GraphReader	gru( argv[2], syntax );
      Graph		gunnamed;

      gru >> gunnamed;

      char	*nn;

      if( argc == 4 )
	nn = argv[3];
      else
	nn = argv[2];

      if( gnamed.order() != gunnamed.order() )
	{
	  char	c;

	  cerr << "Size mismatch between graphs. Proceed anyway (y/n) ? : " 
	       << flush;
	  cin >> c;
	  if( c != 'y' && c != 'Y' )
	    {
	      cerr << "Abort.\n";
	      exit( 1 );
	    }
	}

      Graph::const_iterator	iv1, fv1 = gunnamed.end();
      set<Vertex *>		sv;
      int			id;
      bool			hasNotFound = false;
      Vertex		*v;
      string		name;

      for( iv1=gunnamed.begin(); iv1!=fv1; ++iv1 )
	{
	  assert( (*iv1)->getProperty( "index", id ) );
	  sv = gnamed.getVerticesWith( "index", id );
	  assert( sv.size() < 2 );
	  if( sv.empty() && !hasNotFound )
	    {
	      char	c;

	      cerr << "index " << id << endl;
	      cerr << "Some indexes are not found in the named graph " 
		   << "(perhaps new nodes ?).\n";
	      cerr << "proceed anyway with the old ones ? : " << flush;
	      cin >> c;
	      if( c != 'y' && c != 'Y' )
		{
		  cerr << "Abort.\n";
		  exit( 1 );
		}
	      hasNotFound = true;
	    }
	  if( sv.size() == 1 )
	    {
	      v = *sv.begin();
	      if( v->getProperty( "name", name ) )
		{
		  if( name == "unknown unknown" )
		    name = "unknown";
		  (*iv1)->setProperty( "name", name );
		}
	      if( v->getProperty( "label", name ) )
		(*iv1)->setProperty( "label", name );
	    }
	}

      GraphWriter	gw( nn, syntax );
      gw << gunnamed;
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
    }
}






