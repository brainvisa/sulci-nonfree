
#include <si/global/global.h>
#include <si/model/mReader.h>
#include <si/model/mWriter.h>
#include <graph/tree/tree.h>
#include <iostream>

using namespace sigraph;
using namespace carto;
using namespace std;


MWriter::MWriter( const string & filename, bool makedir )
  : ExoticTreeWriter( filename, MReader::syntax(), makedir )
{
}


MWriter::~MWriter()
{
}


void MWriter::write( const Model & m )
{
  Tree    tr;

  buildTree( tr, m );

  ExoticTreeWriter::visitTree( &tr );
}


void MWriter::buildTree( Tree & tr, const Model & m )
{
  m.buildTree( tr );
}


void MWriter::parse( const AttributedObject *ao )
{
  ParserSet::const_iterator	i, e = parsers().end();
  for( i=parsers().begin(); i!=e; ++i )
    if( (*i)( *this, ao ) )
      return;
}


MWriter::ParserSet & MWriter::parsers()
{
  static ParserSet	ps;
  return ps;
}


void MWriter::registerParser( Parser p )
{
  parsers().insert( p );
}


void MWriter::unregisterParser( Parser p )
{
  parsers().erase( p );
}


