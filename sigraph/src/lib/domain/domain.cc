#include <si/domain/domain.h>
#include <graph/graph/graph.h>

using namespace sigraph;
using namespace carto;

Domain::~Domain()
{
}

#include <cartobase/object/object_d.h>
INSTANTIATE_GENERIC_OBJECT_TYPE( Domain * )

