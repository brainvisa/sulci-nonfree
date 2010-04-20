
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

  cleanup();

  for( iv=_graph.begin(); iv!=fv; ++iv )
    {
      sv = new set<Vertex *>;
      sv->insert( *iv );
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

  for( ic=sc.begin(); ic!=fc; ++ic )
    _data.push_back( &((VertexClique *) ic->get())->vertices() );
}


void VertexCliqueProvider::refresh()
{
  if( _data.size() != _graph.cliques().size() )
    init();

  _data.sort( RandComp() );
}



