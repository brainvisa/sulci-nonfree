#include <si/graph/vertexclique.h>
#include <graph/graph/graph.h>
#include <si/graph/cliqueCache.h>
#include <cartobase/exception/assert.h>
#include <iostream>

using namespace sigraph;
using namespace std;
using namespace carto;


VertexClique::VertexClique( const set<Vertex*> & vert ) : Clique()
{
  _vertices = vert;

  set<Vertex*>::const_iterator	iv, fv=_vertices.end();
  set<Clique*>			*sc;

  for( iv=_vertices.begin(); iv!=fv; ++iv )
    {
      if( (*iv)->hasProperty( "cliques" ) )
	(*iv)->getProperty( "cliques", sc );
      else
	{
	  sc = new set<Clique*>;
	  (*iv)->setProperty( "cliques", sc );
	}

      sc->insert( this );
    }
}


VertexClique::VertexClique( const VertexClique & cl )
  : RCObject(), Clique( cl )
{
}


VertexClique::~VertexClique()
{
  VertexClique::clear();
}


void VertexClique::clear()
{
  set<Vertex*>::iterator	iv, fv=_vertices.end();
  set<Clique*>			*sc;

  for( iv=_vertices.begin(); iv!=fv; ++iv )
    if( (*iv)->hasProperty( "cliques" ) )
      {
	(*iv)->getProperty( "cliques", sc );
	sc->erase( this );
	if( sc->size() == 0 )
	  {
	    (*iv)->removeProperty( "cliques" );
	    delete sc;
	  }
      }

  Clique::clear();
}


void VertexClique::addVertex( Vertex* vert )
{
  _vertices.insert( vert );

  set<Clique*>	*sc;

  if( !vert->getProperty( "cliques", sc ) )
    {
      sc = new set<Clique*>;
      vert->setProperty( "cliques", sc );
    }

  sc->insert( this );
}


void VertexClique::removeVertex( Vertex* vert )
{
  _vertices.erase( vert );

  set<Clique*>	*sc;

  if( !vert->getProperty( "cliques", sc ) )
    return;

  sc->erase( this );
  if( sc->size() == 0 )
    {
      vert->removeProperty( "cliques" );
      delete sc;
    }
}


set <Vertex*> VertexClique::getVerticesWith( const string& s ) const
{
  set<Vertex*> vertices;

  for ( const_iterator v = begin(); v != end(); ++v )
    if( (*v)->hasProperty( s ) )
      vertices.insert( *v );

  return vertices;
}


void VertexClique::edgesBetweenLabels( const string & label1, 
				       const string & label2, 
				       set<Edge *> & ed ) const
{
  set<Vertex *>	vl1 = getVerticesWith( "label", label1 );
  set<Vertex *>::const_iterator	iv, fv=vl1.end();
  Vertex::const_iterator	ie, fe;
  Edge::const_iterator		iv2;
  Vertex			*v;
  string			lab;

  //	pour chaque noeud de label label1
  for( iv=vl1.begin(); iv!=fv; ++iv )
    //	pour chaque relation du noeud
    for( ie=(*iv)->begin(), fe=(*iv)->end(); ie!=fe; ++ie )
      {
	//	chercher l'autre bout de la relation
	iv2 = (*ie)->begin();
	if( *iv2 == *iv )
	  ++iv2;
	v = *iv2;
	//	autre bout au label label2 ?
	if( v->getProperty( "label", lab ) && lab == label2 )
	  ed.insert( *ie );
      }
}


void VertexClique::edgesBetween( const set<Vertex *> & s1, 
				 const set<Vertex *> & s2, 
				 set<Edge *> & ed ) const
{
  set<Vertex *>::const_iterator	iv, fv=s1.end(), fv2 = s2.end();
  Vertex::const_iterator	ie, fe;
  Edge::const_iterator		iv2;
  Vertex			*v;
  string			lab;

  //	pour chaque noeud de label label1
  for( iv=s1.begin(); iv!=fv; ++iv )
    //	pour chaque relation du noeud
    for( ie=(*iv)->begin(), fe=(*iv)->end(); ie!=fe; ++ie )
      {
	//	chercher l'autre bout de la relation
	iv2 = (*ie)->begin();
	if( *iv2 == *iv )
	  ++iv2;
	v = *iv2;
	//	autre bout dans le set s2 ?
	if( s2.find( v ) != fv2 )
	  ed.insert( *ie );
      }
}


unsigned VertexClique::connectivity( const string & label, 
				     set<CComponent *> *sc, 
				     const string & syntType ) const
{
  set<Vertex *>		vx = getVerticesWith( "label", label );
  return( connectivity( vx, sc, syntType ) );
}


unsigned VertexClique::connectivity( const set<Vertex *> & vx, 
				     set<CComponent *> *sc, 
				     const string & syntType )
{
  set<string>			stypes;

  if( !syntType.empty() )
    stypes.insert( syntType );
  return( connectivity( vx, sc, stypes ) );
}


unsigned VertexClique::connectivity( const set<Vertex *> & vx, 
				     set<CComponent *> *sc, 
				     const set<string> & syntTypes )
{
  set<Vertex *>			done;	// ceux qui ont d�j� trait�s
  set<Vertex *>::const_iterator	iv, fv=vx.end(), notdone=done.end();
  Vertex			*v;
  string			label2;
  CComponent			*cc = 0;
  unsigned			num = 0;

  //	
  for( iv=vx.begin(); iv!=fv; ++iv )
    if( done.find( *iv ) == notdone )
      {
	v = *iv;
	if( sc )
	  {
	    cc = new CComponent;
	    sc->insert( cc );
	  }
	++num;

	connPropagate( v, vx, done, cc, syntTypes );
      }

  return( num );
}


void VertexClique::connPropagate( Vertex* v, const set<Vertex *> & vx, 
				  set<Vertex *> & done, CComponent *cc, 
				  const set<string> & syntTypes )
{
  set<Vertex *>			frt;	// front de propagation
  set<Vertex *>::const_iterator	notdone = done.end(), useless = vx.end(), ift;
  Vertex::const_iterator	ie, fe;
  Edge::const_iterator		iv2;
  Vertex			*v2;
  bool				hasNoType = syntTypes.empty();
  set<string>::const_iterator	send = syntTypes.end();

  frt.insert( v );

  while( frt.size() > 0 )
    {
      ift = frt.begin();
      v2 = *ift;
      done.insert( v2 );
      if( cc )
	cc->insert( v2 );

      for( ie=v2->begin(), fe=v2->end(); ie!=fe; ++ie )
	if( hasNoType || syntTypes.find( (*ie)->getSyntax() ) != send )
	  {
	    iv2 = (*ie)->begin();
	    if( *iv2 == v2 )
	      ++iv2;	// autre bout de la relation
	    if( vx.find( *iv2 ) != useless && done.find( *iv2 ) == notdone )
	      frt.insert( *iv2 );
	  }
      frt.erase( ift );
    }
}


Clique* VertexClique::deepCopy() const
{
  VertexClique	*copy = new VertexClique( *this );
  CliqueCache	*cch = 0;
  Graph		*bg;
  Graph		cg( "temporary_copy_graph" );
  Graph::const_iterator	ig, fg;

  ASSERT( getProperty( "graph", bg ) );
  getProperty( "original_cache", cch );

  bg->extract( cg, begin(), end() );
  copy->setProperty( "graph", &cg );
  //	marquer la clique comme copie
  //	( pour d�sactiver le cache �ventuel du ModelFinder )
  copy->setProperty( "is_copy", true );
  if( cch )
    {
      copy->setProperty( "cache", cch->clone() );
      cout << "copie du cache\n";
    }
  //else cout << "pas de cache\n";

  for( ig=cg.begin(), fg=cg.end(); ig!=fg; ++ig )
    {
      if( (*ig)->hasProperty( "cliques" ) )
	(*ig)->removeProperty( "cliques" );
      copy->addVertex( *ig );
    }

  return( copy );
}

#include <cartobase/object/object_d.h>
INSTANTIATE_GENERIC_OBJECT_TYPE( set<VertexClique *> * )
INSTANTIATE_GENERIC_OBJECT_TYPE( VertexClique * )

