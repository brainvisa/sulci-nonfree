/*
 *  Copyright (C) 1998-2004 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 */

#include <si/graph/exoticTreeReader.h>
#include <graph/tree/tree.h>
#include <cartobase/exception/ioexcept.h>
#include <cartobase/object/sreader.h>
#include <iostream>

using namespace carto;
using namespace sigraph;
using namespace std;


ExoticTreeReader::ExoticTreeReader( const string & filename, 
				    const SyntaxSet & attr,
				    const AttributedReader::HelperSet& helpers )
  : TreeReader( filename, attr, helpers )
{
}


ExoticTreeReader::ExoticTreeReader( const SyntaxSet & attr,
				    const AttributedReader::HelperSet& helpers )
  : TreeReader( attr, helpers )
{
}


ExoticTreeReader::ExoticTreeReader( const TreeFactory & factory, 
				    const string & filename, 
				    const SyntaxSet & attr,
				    const AttributedReader::HelperSet& helpers )
  : TreeReader( factory, filename, attr, helpers )
{
}


ExoticTreeReader::ExoticTreeReader( const TreeFactory & factory, 
				    const SyntaxSet & attr,
				    const AttributedReader::HelperSet& helpers )
  : TreeReader( factory, attr, helpers )
{
}


ExoticTreeReader::~ExoticTreeReader()
{
}


SyntaxSet ExoticTreeReader::initSyntax( const string & filename )
{
  SyntaxSet 		as;
  bool		err = false;

  //cout << "initSyntax, file : " << filename << "\n";
  try
    {
      SyntaxReader	ar( filename );
      ar.read( as );
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


void ExoticTreeReader::readTree( Tree* tr )
{
  //cout << "ExoticTreeReader::readTree\n";
  TreeReader::read( *tr );

  //cout << "Verification des attributs de l'arbre...\n";

  //	attributs arbre
  parseTree( tr );
}


Tree* ExoticTreeReader::read()
{
  Tree	*tree = TreeReader::read();
  parseTree( tree );

  return tree;
}


void ExoticTreeReader::parseTree( Tree* tr )
{
  if( !tr )
    cout << "ExoticTreeReader::parseTree " << tr << ", filename: " 
         << name() << endl;
  //	attributs globaux
  parse( tr );

  Tree::const_iterator	in, fn=tr->end();

  //	éléments
  for( in=tr->begin(); in!=fn; ++in )
    {
      parseTree( (Tree *) *in );
    }
}
