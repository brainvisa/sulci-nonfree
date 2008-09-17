/*
 *  Copyright (C) 1998-2004 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 */

#include <si/graph/exoticGraphReader.h>
#include <graph/graph/graph.h>
#include <cartobase/exception/ioexcept.h>
#include <cartobase/object/sreader.h>
#include <iostream>

using namespace carto;
using namespace sigraph;
using namespace std;


ExoticGraphReader::ExoticGraphReader( const string & filename, 
				      const SyntaxSet & attr ) 
  : GraphReader( filename, attr )
{
}


ExoticGraphReader::ExoticGraphReader( const SyntaxSet & attr ) 
  : GraphReader( attr )
{
}


ExoticGraphReader::~ExoticGraphReader()
{
}


SyntaxSet ExoticGraphReader::initSyntax( const string & filename )
{
  SyntaxSet	as;
  bool		err = false;

  try
    {
      SyntaxReader	ar( filename );
      ar >> as;
    }
  catch( parse_error & e )
    {
      cerr << e.what() << " : " << e.filename() << ", line " << e.line() 
	   << endl;
      err = true;
    }
  catch( exception & e )
    {
      cerr << e.what() << endl;
      err = true;
    }

  if( err )
    {
      throw io_error("Graph Reader syntax not sucessfully loaded.", filename);
    }

  return as;
}


void ExoticGraphReader::read( Graph & sg )
{
  Graph::iterator	in, fn=sg.end();

  GraphReader::read( sg );
  sg.setProperty( "aims_reader_filename", name() );
  sg.setProperty( "aims_reader_loaded_objects", int(0) );

  //cout << "Verification des attributs du graphe...\n";

  //	attributs globaux graphe
  parse( sg, &sg );

  //	noeuds
  for( in=sg.begin(); in!=fn; ++in )
    parse( sg, *in );

  //	relations
  set<Edge*> edges = sg.edges();
  set<Edge*>::iterator	ir, fr=edges.end();

  for( ir=edges.begin(); ir!=fr; ++ir )
    parse( sg, *ir );
}
