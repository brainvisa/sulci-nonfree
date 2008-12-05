// 30/11/2005

#include <si/fold/foldDescr4.h>
#include <si/model/adaptive.h>
#include <si/graph/vertexclique.h>
#include <si/fold/fattrib.h>
#include <si/fold/foldvertex.h>
#include <si/fold/interfoldedge.h>
#include <graph/tree/tree.h>
#include <iostream>

using namespace sigraph;
using namespace carto;
using namespace std;

/* taken from <ffmpeg/common.h>
   redefined just to avoid dirty warnings
 */
#define structoffset( T, F ) ((size_t)((char *)&((T *)8)->F)-8)


FoldDescr4::FoldDescr4() : FoldDescr2()
{
}

FoldDescr4::FoldDescr4( const FoldDescr4 & f )  : FoldDescr2( f )
{
}

FoldDescr4::~FoldDescr4()
{
}

bool FoldDescr4::makeVectorElements( const Clique* cl, vector<double> & vec, 
                                     GenericObject* ao )
{
  vec.reserve( END );
  bool vec_valid = FoldDescr2::makeVectorElements( cl, vec, ao );

  if( vec_valid )
  {
    // ajout
    const VertexClique	*vcl = (const VertexClique *) cl;
    string			label, labelV;
    VertexClique::iterator	iv, fv=vcl->end();
    //THICKNESS
    int                         mid_inter_voxels_clique = 0;
    int                         mid_inter_voxels_vertex;
    float                       thickness_vertex;
    float                       thickness = 0;
    
//     FOLD_OPENING
    float                       surface_area = vec[SIZE];
    float                       LCR_volume_clique = 0;
    float                       LCR_volume_vertex;
    float                       fold_opening;
     
    
    cl->getProperty( SIA_LABEL, label );
    for( iv=vcl->begin(); iv!=fv; ++iv )
    {
      Vertex  *v = *iv;
      v->getProperty( SIA_LABEL, labelV );
      if( label == labelV )
      {
        if( v->getProperty("LCR_volume",LCR_volume_vertex) )
          LCR_volume_clique += LCR_volume_vertex;
        // else cout << "no LCR_volume in node\n";
        if( v->getProperty("mid_interface_voxels",mid_inter_voxels_vertex) )
        {
          mid_inter_voxels_clique += mid_inter_voxels_vertex;
          if( v->getProperty("thickness_mean",thickness_vertex) )
            thickness += thickness_vertex * mid_inter_voxels_vertex;
          // else cout << "no thickness_mean in node\n";
        }
        // else cout << "no mid_interface_voxels in node\n";
      }
    }
    if(surface_area != 0)
    {
      fold_opening = LCR_volume_clique/surface_area;
    }

    else
    {
      fold_opening = 10000;
    }
    if(mid_inter_voxels_clique!=0)
      thickness = thickness/mid_inter_voxels_clique;
    else
      thickness = 0;

    vec.push_back(thickness);
    vec.push_back(fold_opening);

  }
  else
  {
    // invalid vector
    vec.push_back( 0. );
    vec.push_back( 0. );
  }

  return vec_valid;
}


void FoldDescr4::buildTree( Tree & t )
{
  FoldDescr2::buildTree( t );
  t.setSyntax( SIA_FOLD_DESCRIPTOR4 );
}


vector<string> FoldDescr4::descriptorsNames() const
{
  static vector<string>	names;
  if( names.empty() )
    {
      names.reserve( END );
      names = FoldDescr2::descriptorsNames();
      names.push_back( names[ FoldDescr2::END-1 ] );
      for( unsigned i=FoldDescr2::END-1; i>MEANDEPTH; --i )
        names[ i ] = names[ i-1 ];
      names[ MEANDEPTH ] = "geodesicDepthMean";
      names[ SIZE ] = "surface";
      names[ SIZE_HULLJUNC ] = "hullJunctionsLength";
      names.push_back("GM_thickness");
      names.push_back("fold_opening");
    }
  return names;
}


string FoldDescr4::name() const
{
  return SIA_FOLD_DESCRIPTOR4;
}


string FoldDescr4::surfaceAttribute( bool normalized, const Clique*, 
                                     int, int ) const
{
  if( normalized )
    return SIA_REFSURFACE_AREA;
  return SIA_SURFACE_AREA;
}


string FoldDescr4::gravityCenterAttribute( bool normalized, const Clique*, 
                                           int, int ) const
{
  if( normalized )
    return SIA_REFGRAVITY_CENTER;
  return SIA_GRAVITY_CENTER;
}


string FoldDescr4::normalAttribute( bool normalized, const Clique*, int, 
                                    int ) const
{
  if( normalized )
    return SIA_REFNORMAL;
  return SIA_NORMAL;
}


string FoldDescr4::minDepthAttribute( bool normalized, const Clique*, 
                                      int, int ) const
{
  if( normalized )
    return SIA_REFMINDEPTH;
  return SIA_MINDEPTH;
}


string FoldDescr4::maxDepthAttribute( bool normalized, const Clique*, 
                                      int, int ) const
{
  if( normalized )
    return SIA_REFMAXDEPTH;
  return SIA_MAXDEPTH;
}


string FoldDescr4::meanDepthAttribute( bool normalized, const Clique*, 
                                       int, int ) const
{
  if( normalized )
    return SIA_REFMEANDEPTH;
  return SIA_MEANDEPTH;
}


string FoldDescr4::junctionLengthAttribute( bool normalized, const Clique*, 
                                            int, int ) const
{
  if( normalized )
    return SIA_REFLENGTH;
  return SIA_LENGTH;
}


string FoldDescr4::junctionExtremity1Attribute( bool normalized, 
                                                const Clique*, int, int ) const
{
  if( normalized )
    return SIA_REFEXTREMITY1;
  return SIA_EXTREMITY1;
}


string FoldDescr4::junctionExtremity2Attribute( bool normalized, const Clique*,
                                                int, int ) const
{
  if( normalized )
    return SIA_REFEXTREMITY2;
  return SIA_EXTREMITY2;
}


string FoldDescr4::junctionDirectionAttribute( bool normalized, const Clique*, 
                                               int, int ) const
{
  if( normalized )
    return SIA_REFDIRECTION;
  return SIA_DIRECTION;
}


string FoldDescr4::corticalDistanceAttribute( bool normalized, const Clique*, 
                                              int, int ) const
{
  if( normalized )
    return SIA_REFDIST;
  return SIA_DIST;
}


int FoldDescr4::surfaceOffset( bool normalized, const Clique*, int, int ) const
{
  if( normalized )
    return structoffset( FoldVertex, refsurface_area );
  return structoffset( FoldVertex, surface_area );
}


int FoldDescr4::surfaceValidOffset( bool normalized, const Clique*, int, 
                                    int ) const
{
  if( normalized )
    return structoffset( FoldVertex, refsurface_area_valid );
  return structoffset( FoldVertex, surface_area_valid );
}


int FoldDescr4::gravityCenterOffset( bool normalized, const Clique*, int, 
                                     int ) const
{
  if( normalized )
    return structoffset( FoldVertex, refgravity_center );
  return structoffset( FoldVertex, gravity_center );
}


int FoldDescr4::gravityCenterValidOffset( bool normalized, const Clique*, int, 
                                          int ) const
{
  if( normalized )
    return structoffset( FoldVertex, refgravity_center_valid );
  return structoffset( FoldVertex, gravity_center_valid );
}


int FoldDescr4::normalOffset( bool normalized, const Clique*, int, int ) const
{
  if( normalized )
    return structoffset( FoldVertex, refnormal );
  return structoffset( FoldVertex, normal );
}


int FoldDescr4::normalValidOffset( bool normalized, const Clique*, int, 
                                   int ) const
{
  if( normalized )
    return structoffset( FoldVertex, refnormal_valid );
  return structoffset( FoldVertex, normal_valid );
}


int FoldDescr4::minDepthOffset( bool normalized, const Clique*, int, 
                                int ) const
{
  if( normalized )
    return structoffset( FoldVertex, refmindepth );
  return structoffset( FoldVertex, mindepth );
}


int FoldDescr4::minDepthValidOffset( bool normalized, const Clique*, int, 
                                     int ) const
{
  if( normalized )
    return structoffset( FoldVertex, refmindepth_valid );
  return structoffset( FoldVertex, mindepth_valid );
}


int FoldDescr4::maxDepthOffset( bool normalized, const Clique*, int, 
                                int ) const
{
  if( normalized )
    return structoffset( FoldVertex, refmaxdepth );
  return structoffset( FoldVertex, maxdepth );
}


int FoldDescr4::maxDepthValidOffset( bool normalized, const Clique*, int, 
                                     int ) const
{
  if( normalized )
    return structoffset( FoldVertex, refmaxdepth_valid );
  return structoffset( FoldVertex, maxdepth_valid );
}


int FoldDescr4::meanDepthOffset( bool normalized, const Clique*, int, 
                                 int ) const
{
  if( normalized )
    return structoffset( FoldVertex, refmeandepth );
  return structoffset( FoldVertex, meandepth );
}


int FoldDescr4::meanDepthValidOffset( bool normalized, const Clique*, 
                                      int, int ) const
{
  if( normalized )
    return structoffset( FoldVertex, refmeandepth_valid );
  return structoffset( FoldVertex, meandepth_valid );
}


int FoldDescr4::hullJunctionLengthOffset( bool normalized, const Clique*, 
                                          int, int ) const
{
  if( normalized )
    return structoffset( HullJunctionEdge, reflength );
  return structoffset( HullJunctionEdge, length );
}


int FoldDescr4::hullJunctionLengthValidOffset( bool normalized, const Clique*, 
                                               int, int ) const
{
  if( normalized )
    return structoffset( HullJunctionEdge, reflength_valid );
  return structoffset( HullJunctionEdge, length_valid );
}


int FoldDescr4::hullJunctionExtremity1Offset( bool, const Clique*, int, 
                                              int ) const
{
  return structoffset( HullJunctionEdge, refextremity1 );
}


int FoldDescr4::hullJunctionExtremity1ValidOffset( bool, const Clique*, int, 
                                                   int ) const
{
  return structoffset( HullJunctionEdge, refextremity1_valid );
}


int FoldDescr4::hullJunctionExtremity2Offset( bool, const Clique*, int, 
                                              int ) const
{
  return structoffset( HullJunctionEdge, refextremity2 );
}


int FoldDescr4::hullJunctionExtremity2ValidOffset( bool, const Clique*, int, 
                                                   int ) const
{
  return structoffset( HullJunctionEdge, refextremity2_valid );
}


int FoldDescr4::hullJunctionDirectionOffset( bool normalized, const Clique*, 
                                             int, int ) const
{
  if( normalized )
    return structoffset( HullJunctionEdge, refdirection );
  return structoffset( HullJunctionEdge, direction );
}


int FoldDescr4::hullJunctionDirectionValidOffset( bool normalized, 
                                                  const Clique*, int, 
                                                  int ) const
{
  if( normalized )
    return structoffset( HullJunctionEdge, refdirection_valid );
  return structoffset( HullJunctionEdge, direction_valid );
}


int FoldDescr4::corticalDistanceOffset( bool normalized, const Clique*, int, 
                                        int ) const
{
  if( normalized )
    return structoffset( CorticalEdge, refdist );
  return structoffset( CorticalEdge, dist );
}


int FoldDescr4::corticalDistanceValidOffset( bool normalized, const Clique*, 
                                             int, int ) const
{
  if( normalized )
    return structoffset( CorticalEdge, refdist_valid );
  return structoffset( CorticalEdge, dist_valid );
}


