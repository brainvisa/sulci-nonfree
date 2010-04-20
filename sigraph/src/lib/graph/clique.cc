#include <si/graph/clique.h>
#include <graph/graph/graph.h>
#include <si/graph/cliqueCache.h>

using namespace sigraph;
using namespace carto;
using namespace std;


Clique::Clique( const Clique & cl )
  : RCObject(), Interface(), StringInterface(), ScalarInterface(),
    SizeInterface(),  IterableInterface(), ArrayInterface(),
    DynArrayInterface(),DictionaryInterface(), 
    IteratorInterface(), DictionaryIteratorInterface(), AttributedObject( cl )
{
}


Clique::~Clique()
{
  cout << "~Clique " << this << endl;
  clear();
  cout << "~Clique: cleared.\n";
}


void Clique::clear()
{
  CliqueCache	*cache;

  if( getProperty( "cache", cache ) )
    {
      delete cache;
      removeProperty( "cache" );
    }
  if( !hasProperty( "is_copy" ) && getProperty( "original_cache", cache ) )
    {
      delete cache;
      removeProperty( "original_cache" );
    }
}


#include <cartobase/object/object_d.h>
INSTANTIATE_GENERIC_OBJECT_TYPE( set<Clique *> * )

