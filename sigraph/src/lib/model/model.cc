
#include <si/model/model.h>

using namespace sigraph;
using namespace carto;
using namespace std;


Model::~Model()
{
}


double Model::prop( const Clique* cl, const map<Vertex*, string> & )
{
  
  return( prop( cl ) );	// par défaut, on recalcule toujours
}


double Model::printDescription( Clique* cl, bool )
{
  return( prop( cl ) );
}


bool Model::doesOutputChange( const Clique*,
                              const map<Vertex*, string> & ) const
{
  return true;
}


#include <cartobase/object/object_d.h>
INSTANTIATE_GENERIC_OBJECT_TYPE( Model * )

