
#include <si/graph/sgiterator.h>
#include <neur/rand/rand.h>
#include <si/graph/vertexclique.h>

using namespace sigraph;
using namespace std;


SGProvider::~SGProvider()
{
}


//


VertexProvider::~VertexProvider()
{
  cleanup();
}


void VertexProvider::cleanup()
{
  datatype::iterator	is, fs = _data.end();

  for( is=_data.begin(); is!=fs; ++is )
    delete *is;
}


void VertexProvider::init()
{
  CGraph::const_iterator	iv, fv=_graph.end();
  set<Vertex *>			*sv;
  map<long, Vertex *> tractable_vert;

  cleanup();

  // sort vertices first in a reproducible order
  int index;
  for( iv=_graph.begin(); iv!=fv; ++iv )
  {
    if( !(*iv)->getProperty( "index", index )
        && !(*iv)->getProperty( "skeleton_label", index ) )
      index = reinterpret_cast<long>( *iv ); // NON-TRACTABLE.
    tractable_vert[ index ] = *iv;
  }

  map<long, Vertex *>::iterator iov, eov = tractable_vert.end();
  for( iov=tractable_vert.begin(); iov!=eov; ++iov )
    {
      sv = new set<Vertex *>;
      sv->insert( iov->second );
      _data.push_back( sv );
    }
}


void VertexProvider::refresh()
{
  if( _data.size() != _graph.order() )
    init();

  // sort
  _data.sort( RandComp() );
}


//


bool RandComp::operator () ( const set<Vertex *> *s1, const set<Vertex *> *s2 )
{
  map< const set<Vertex *> *, float >::iterator	i1, i2;
  float						v1, v2;

  i1 = dat.find( s1 );
  i2 = dat.find( s2 );

  if( i1 == dat.end() )
    {
      v1 = ran1();
      dat[ s1 ] = v1;
    }
  else
    v1 = (*i1).second;

  if( i2 == dat.end() )
    {
      v2 = ran1();
      dat[ s2 ] = v2;
    }
  else
    v2 = (*i2).second;

  return( v1 < v2 );
}


void RandComp::reset()
{
  dat.erase( dat.begin(), dat.end() );
}



//


VertexCliqueProvider::~VertexCliqueProvider()
{
  /*	Rien à détruire: la liste _data ne contient pas des groupes de 
	vertex alloués, mais reprend des pointeurs sur les cliques */
}


void VertexCliqueProvider::init()
{
  _data.erase( _data.begin(), _data.end() );

  const CGraph::CliqueSet 		& sc = _graph.cliques();
  CGraph::CliqueSet::const_iterator	ic, fc=sc.end();
  map<long, Clique *>                   tractable_cliques;

  // sort cliques first in a reproducible order
  int index;
  for( ic=sc.begin(); ic!=fc; ++ic )
  {
    if( !(*ic)->getProperty( "index", index ) )
      index = reinterpret_cast<long>( ic->get() ); // NON-TRACTABLE.
    tractable_cliques[ index ] = ic->get();
  }

  map<long, Clique *>::iterator ioc, eoc = tractable_cliques.end();
  for( ioc=tractable_cliques.begin(); ioc!=eoc; ++ioc )
    _data.push_back( &((VertexClique *) ioc->second)->vertices() );
}


void VertexCliqueProvider::refresh()
{
  if( _data.size() != _graph.cliques().size() )
    init();

  _data.sort( RandComp() );
}



