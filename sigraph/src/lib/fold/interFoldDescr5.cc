#include <si/fold/interFoldDescr5.h>
#include <si/fold/fattrib.h>
#include <graph/tree/tree.h>
#include <graph/graph/graph.h>
#include <si/fold/foldvertex.h>
#include <si/fold/interfoldedge.h>
#include <si/fold/interFoldCache.h>
#include <si/graph/vertexclique.h>
#include <aims/moment/momInvariant.h>

using namespace sigraph;
using namespace carto;
using namespace std;


InterFoldDescr5::~InterFoldDescr5()
{
}


void InterFoldDescr5::buildTree( Tree & t )
{
  t.setSyntax( SIA_INTER_FOLD_DESCRIPTOR5 );
  if( outputInertia() )
    t.setProperty( SIA_OUTPUT_INERTIA, int(1) );
}


vector<string> InterFoldDescr5::descriptorsNames() const
{
  static vector<string>	names;
  if( names.empty() )
    {
      names.reserve( END );
      names = InterFoldDescr4::descriptorsNames();
      names.push_back( "moment_inv0" );
      names.push_back( "moment_inv1" );
      names.push_back( "moment_inv2" );
      names.push_back( "moment_inv3" );
      names.push_back( "moment_inv4" );
      names.push_back( "moment_inv5" );
      names.push_back( "moment_inv6" );
      names.push_back( "moment_inv7" );
      names.push_back( "moment_inv8" );
      names.push_back( "moment_inv9" );
      names.push_back( "moment_inv10" );
      names.push_back( "moment_inv11" );
      if( outputInertia() )
      {
        names.push_back( "inertia_0" );
        names.push_back( "inertia_1" );
        names.push_back( "inertia_2" );
        names.push_back( "inertia_3" );
        names.push_back( "inertia_4" );
        names.push_back( "inertia_5" );
      }
    }
  return names;
}


string InterFoldDescr5::name() const
{
  return SIA_INTER_FOLD_DESCRIPTOR5;
}


bool InterFoldDescr5::makeVectorElements( const Clique* cl, 
                                          vector<double> & vec, 
                                          GenericObject* ao )
{
  if( outputInertia() )
    vec.reserve( END );
  else
    vec.reserve( INERTIA_0 );
  InterFoldDescr4::makeVectorElements( cl, vec, ao );

  if( vec[0] == 0 ) // not valid
    {
      vec.push_back( 0 );
      vec.push_back( 0 );
      vec.push_back( 0 );
      vec.push_back( 0 );
      vec.push_back( 0 );
      vec.push_back( 0 );
      vec.push_back( 0 );
      vec.push_back( 0 );
      vec.push_back( 0 );
      vec.push_back( 0 );
      vec.push_back( 0 );
      vec.push_back( 0 );
      if( outputInertia() )
      {
        vec.push_back( 0 );
        vec.push_back( 0 );
        vec.push_back( 0 );
        vec.push_back( 0 );
        vec.push_back( 0 );
        vec.push_back( 0 );
      }

      return false;
    }

  const VertexClique		*vcl = static_cast<const VertexClique *>( cl );
  string			label1, label2, labelV;
  Moment<int16_t>		mom, m;
  VertexClique::const_iterator	iv, ev = vcl->end();
  Object			o;
  Graph				*g;
  vector<float>			vs;
  double			vsz;
#ifdef SI_USE_BUILTIN_FOLD
  FoldVertex			*v;
#else
  Vertex			*v;
#endif

  cl->getProperty( SIA_LABEL1, label1 );
  cl->getProperty( SIA_LABEL2, label2 );
  cl->getProperty( SIA_GRAPH, g );
  g->getProperty( SIA_VOXEL_SIZE, vs );
  vsz = vs[0] * vs[1] * vs[2];
  mom.clear();
  mom.setcx( vs[0] );
  mom.setcy( vs[1] );
  mom.setcz( vs[2] );
  mom.setct( vsz );
  m.setcx( vs[0] );
  m.setcy( vs[1] );
  m.setcz( vs[2] );
  m.setct( vsz );

  for( iv=vcl->begin(); iv!=ev; ++iv )
    {
#ifdef SI_USE_BUILTIN_FOLD
      v = static_cast<FoldVertex *>( *iv );
      labelV = v->label;
#else
      v = *iv;
      v->getProperty( SIA_LABEL, labelV );
#endif
      if( label1 == labelV || label2 == labelV )
        {
          try
            {
              o = v->getProperty( SIA_MOMENTS );
            }
          catch( ... )
            {
              continue;
            }
          if( !o.isNull() )
            {
              const vector<float>	& vm = o->value<vector<float> >();
              // check vm size? should be 20.
              if( vm[0] != 0 )
                {
                  m.m0() = vm[0];
                  m.m1()[0] = vm[1];
                  m.m1()[1] = vm[2];
                  m.m1()[2] = vm[3];
                  m.m2()[0] = vm[4];
                  m.m2()[1] = vm[5];
                  m.m2()[2] = vm[6];
                  m.m2()[3] = vm[7];
                  m.m2()[4] = vm[8];
                  m.m2()[5] = vm[9];
                  m.m3()[0] = vm[10];
                  m.m3()[1] = vm[11];
                  m.m3()[2] = vm[12];
                  m.m3()[3] = vm[13];
                  m.m3()[4] = vm[14];
                  m.m3()[5] = vm[15];
                  m.m3()[6] = vm[16];
                  m.m3()[7] = vm[17];
                  m.m3()[8] = vm[18];
                  m.m3()[9] = vm[19];
                  m.sum() = vm[0] / vsz;
                  m.gravity()[0] = m.m1()[0] / m.m0();
                  m.gravity()[0] = m.m1()[1] / m.m0();
                  m.gravity()[0] = m.m1()[2] / m.m0();
                  mom += m;
                }
            }
        }
    }

  MomentInvariant<int16_t>	momi( &mom );
  momi.doit( &mom );
  const double	*vmomi = momi.invariant();

  vec.push_back( vmomi[0] );
  vec.push_back( vmomi[1] );
  vec.push_back( vmomi[2] );
  vec.push_back( vmomi[3] );
  vec.push_back( vmomi[4] );
  vec.push_back( vmomi[5] );
  vec.push_back( vmomi[6] );
  vec.push_back( vmomi[7] );
  vec.push_back( vmomi[8] );
  vec.push_back( vmomi[9] );
  vec.push_back( vmomi[10] );
  vec.push_back( vmomi[11] );

  if( outputInertia() )
  {
    vec.push_back( mom.m2()[0] );
    vec.push_back( mom.m2()[1] );
    vec.push_back( mom.m2()[2] );
    vec.push_back( mom.m2()[3] );
    vec.push_back( mom.m2()[4] );
    vec.push_back( mom.m2()[5] );
  }

  return true;
}


