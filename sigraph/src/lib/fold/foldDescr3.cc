// 03/03/03

#include <si/fold/foldDescr3.h>
#include <si/model/adaptive.h>
#include <si/graph/vertexclique.h>
#include <si/fold/fattrib.h>
#include <graph/tree/tree.h>
#include <iostream>

using namespace sigraph;
using namespace carto;
using namespace std;


FoldDescr3::FoldDescr3() : FoldDescr2()
{
}


FoldDescr3::FoldDescr3( const FoldDescr3 & f )
  : FoldDescr2( f )
{
}


FoldDescr3::~FoldDescr3()
{
}


bool FoldDescr3::makeVectorElements( const Clique* cl, vector<double> & vec, 
                                     GenericObject* ao )
{
  vec.reserve( END );
  bool vec_valid = FoldDescr2::makeVectorElements( cl, vec, ao );

  if( vec.size() == SURFACE )
    {
      const VertexClique		*vcl = (const VertexClique *) cl;
      VertexClique::const_iterator	iv, fv=vcl->end();
      Vertex				*v;
      string				label, labelV;
      float				s, surf = 0;

      cl->getProperty( SIA_LABEL, label );
      for( iv=vcl->begin(); iv!=fv; ++iv )
        {
          v = *iv;
          v->getProperty( SIA_LABEL, labelV );
          if( label == labelV && v->getProperty( SIA_SURFACE_AREA, s ) )
            surf += s;
        }
      vec.push_back( surf );
    }

  return vec_valid;
}


void FoldDescr3::buildTree( Tree & t )
{
  FoldDescr2::buildTree( t );
  t.setSyntax( SIA_FOLD_DESCRIPTOR3 );
}


vector<string> FoldDescr3::descriptorsNames() const
{
  static vector<string>	names;
  if( names.empty() )
    {
      names.reserve( 28 );

      names = FoldDescr2::descriptorsNames();

      names.push_back( "surface" );
    }
  return names;
}


string FoldDescr3::name() const
{
  return SIA_FOLD_DESCRIPTOR3;
}


