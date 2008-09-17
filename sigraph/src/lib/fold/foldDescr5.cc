// 21/07/2006

#include <si/fold/foldDescr5.h>
#include <si/model/adaptive.h>
#include <si/graph/vertexclique.h>
#include <si/fold/foldCache.h>
#include <si/fold/fattrib.h>
#include <si/fold/foldvertex.h>
#include <si/fold/interfoldedge.h>
#include <aims/moment/momInvariant.h>
#include <graph/tree/tree.h>
#include <iostream>

using namespace sigraph;
using namespace aims;
using namespace carto;
using namespace std;


FoldDescr5::FoldDescr5() : FoldDescr4()
{
}

FoldDescr5::FoldDescr5( const FoldDescr5 & f )  : FoldDescr4( f )
{
}

FoldDescr5::~FoldDescr5()
{
}

bool FoldDescr5::makeVectorElements( const Clique* cl, vector<double> & vec, 
                                     GenericObject* ao )
{
  vec.reserve( END );
  FoldDescr4::makeVectorElements( cl, vec, ao );

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
      return false;
    }

  const VertexClique		*vcl = static_cast<const VertexClique *>( cl );
  string			label, labelV;
  Moment<int16_t>		mom, m2;
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

  cl->getProperty( SIA_LABEL, label );
  cl->getProperty( SIA_GRAPH, g );
  g->getProperty( SIA_VOXEL_SIZE, vs );
  vsz = vs[0] * vs[1] * vs[2];
  mom.clear();
  mom.setcx( vs[0] );
  mom.setcy( vs[1] );
  mom.setcz( vs[2] );
  mom.setct( vsz );
  m2.setcx( vs[0] );
  m2.setcy( vs[1] );
  m2.setcz( vs[2] );
  m2.setct( vsz );

  for( iv=vcl->begin(); iv!=ev; ++iv )
    {
#ifdef SI_USE_BUILTIN_FOLD
      v = static_cast<FoldVertex *>( *iv );
      labelV = v->label;
#else
      v = *iv;
      v->getProperty( SIA_LABEL, labelV );
#endif
      if( label == labelV )
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
                  m2.m0() = vm[0];
                  m2.m1()[0] = vm[1];
                  m2.m1()[1] = vm[2];
                  m2.m1()[2] = vm[3];
                  m2.m2()[0] = vm[4];
                  m2.m2()[1] = vm[5];
                  m2.m2()[2] = vm[6];
                  m2.m2()[3] = vm[7];
                  m2.m2()[4] = vm[8];
                  m2.m2()[5] = vm[9];
                  m2.m3()[0] = vm[10];
                  m2.m3()[1] = vm[11];
                  m2.m3()[2] = vm[12];
                  m2.m3()[3] = vm[13];
                  m2.m3()[4] = vm[14];
                  m2.m3()[5] = vm[15];
                  m2.m3()[6] = vm[16];
                  m2.m3()[7] = vm[17];
                  m2.m3()[8] = vm[18];
                  m2.m3()[9] = vm[19];
                  m2.sum() = vm[0] / vsz;
                  m2.gravity()[0] = m2.m1()[0] / m2.m0();
                  m2.gravity()[0] = m2.m1()[1] / m2.m0();
                  m2.gravity()[0] = m2.m1()[2] / m2.m0();
                  mom += m2;
                }
            }
        }
    }

  /*
  cout << "moments for " << label << ": " << mom.m0() << "; " << mom.m1()[0] 
       << ", " << mom.m1()[1] << ", " << mom.m1()[2] << "; " << mom.m2()[0] 
       << ", " << mom.m2()[1] << ", " << mom.m2()[2] << ", " << mom.m2()[3] 
       << ", " << mom.m2()[4] << ", " << mom.m2()[5] << "; " << mom.m3()[0] 
       << ", " << mom.m3()[1] << ", " << mom.m3()[2] << ", " << mom.m3()[3] 
       << ", " << mom.m3()[4] << ", " << mom.m3()[5] << ", " << mom.m3()[6] 
       << ", " << mom.m3()[7] << ", " << mom.m3()[8] << ", " << mom.m3()[9] 
       << endl;
  */

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

  return true;
}


void FoldDescr5::buildTree( Tree & t )
{
  FoldDescr4::buildTree( t );
  t.setSyntax( SIA_FOLD_DESCRIPTOR5 );
}


vector<string> FoldDescr5::descriptorsNames() const
{
  static vector<string>	names;
  if( names.empty() )
    {
      names.reserve( END );
      names = FoldDescr4::descriptorsNames();
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
    }
  return names;
}


string FoldDescr5::name() const
{
  return SIA_FOLD_DESCRIPTOR5;
}


