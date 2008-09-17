#include <si/fold/interFoldDescr4.h>
#include <si/fold/fattrib.h>
#include <graph/tree/tree.h>
#include <si/fold/foldvertex.h>
#include <si/fold/interfoldedge.h>

using namespace sigraph;
using namespace std;


/* taken from <ffmpeg/common.h>
   redefined just to avoid dirty warnings
 */
#define structoffset( T, F ) ((size_t)((char *)&((T *)8)->F)-8)


InterFoldDescr4::~InterFoldDescr4()
{
}


void InterFoldDescr4::buildTree( Tree & t )
{
  t.setSyntax( SIA_INTER_FOLD_DESCRIPTOR4 );
}


vector<string> InterFoldDescr4::descriptorsNames() const
{
  return InterFoldDescr2::descriptorsNames();
}


string InterFoldDescr4::name() const
{
  return SIA_INTER_FOLD_DESCRIPTOR4;
}


string InterFoldDescr4::foldSurfaceAttribute( bool normalized, const Clique*, 
                                              int, int ) const
{
  if( normalized )
    return SIA_REFSURFACE_AREA;
  return SIA_SURFACE_AREA;
}


string InterFoldDescr4::corticalLengthAttribute( bool normalized, 
                                                 const Clique*, 
                                                 int, int ) const
{
  if( normalized )
    return SIA_REFLENGTH;
  return SIA_LENGTH;
}


string InterFoldDescr4::corticalDistanceAttribute( bool normalized, 
                                                   const Clique*, int, 
                                                   int ) const
{
  if( normalized )
    return SIA_REFDIST;
  return SIA_DIST;
}


string InterFoldDescr4::corticalSS1NearestAttribute( bool normalized, 
                                                     const Clique*, int, 
                                                     int ) const
{
  if( normalized )
    return SIA_REFSS1NEAREST;
  return SIA_SS1NEAREST;
}


string InterFoldDescr4::corticalSS2NearestAttribute( bool normalized, 
                                                     const Clique*, int, 
                                                     int ) const
{
  if( normalized )
    return SIA_REFSS2NEAREST;
  return SIA_SS2NEAREST;
}


string InterFoldDescr4::hullJunctionDirectionAttribute( bool normalized, 
                                                    const Clique*, int, 
                                                    int ) const
{
  if( normalized )
    return SIA_REFDIRECTION;
  return SIA_DIRECTION;
}


string InterFoldDescr4::hullJunctionExtremity1Attribute( bool normalized, 
                                                     const Clique*, int, 
                                                     int ) const
{
  if( normalized )
    return SIA_REFEXTREMITY1;
  return SIA_EXTREMITY1;
}


string InterFoldDescr4::hullJunctionExtremity2Attribute( bool normalized, 
                                                     const Clique*, int, 
                                                     int ) const
{
  if( normalized )
    return SIA_REFEXTREMITY2;
  return SIA_EXTREMITY2;
}


string InterFoldDescr4::junctionLengthAttribute( bool normalized, 
                                                 const Clique*, 
                                                 int, int ) const
{
  if( normalized )
    return SIA_REFLENGTH;
  return SIA_LENGTH;
}


string InterFoldDescr4::junctionDepthAttribute( bool normalized, 
                                                const Clique*, int, int ) const
{
  if( normalized )
    return SIA_REFMAXDEPTH;
  return SIA_MAXDEPTH;
}


string InterFoldDescr4::pliDePassageDepthAttribute( bool normalized, 
                                                    const Clique*, int, 
                                                    int ) const
{
  if( normalized )
    return SIA_REFDEPTH;
  return SIA_DEPTH;
}


int InterFoldDescr4::foldSurfaceOffset( bool normalized, const Clique*, 
                                        int, int ) const
{
  if( normalized )
    return structoffset( FoldVertex, refsurface_area );
  return structoffset( FoldVertex, surface_area );
}


int InterFoldDescr4::foldSurfaceValidOffset( bool normalized, const Clique*, 
                                             int, int ) const
{
  if( normalized )
    return structoffset( FoldVertex, refsurface_area_valid );
  return structoffset( FoldVertex, surface_area_valid );
}


int InterFoldDescr4::corticalLengthOffset( bool normalized, const Clique*, 
                                           int, int ) const
{
  if( normalized )
    return structoffset( CorticalEdge, reflength );
  return structoffset( CorticalEdge, length );
}


int InterFoldDescr4::corticalLengthValidOffset( bool normalized, 
                                                const Clique*, int, int ) const
{
  if( normalized )
    return structoffset( CorticalEdge, reflength_valid );
  return structoffset( CorticalEdge, length_valid );
}


int InterFoldDescr4::corticalDistanceOffset( bool normalized, const Clique*, 
                                             int, int ) const
{
  if( normalized )
    return structoffset( CorticalEdge, refdist );
  return structoffset( CorticalEdge, dist );
}


int InterFoldDescr4::corticalDistanceValidOffset( bool normalized, 
                                                  const Clique*, 
                                                  int, int ) const
{
  if( normalized )
    return structoffset( CorticalEdge, refdist_valid );
  return structoffset( CorticalEdge, dist_valid );
}


int InterFoldDescr4::corticalSS1NearestOffset( bool normalized, const Clique*, 
                                               int, int ) const
{
  if( normalized )
    return structoffset( CorticalEdge, refSS1nearest );
  return structoffset( CorticalEdge, SS1nearest );
}


int InterFoldDescr4::corticalSS1NearestValidOffset( bool normalized, 
                                                    const Clique*, 
                                                    int, int ) const
{
  if( normalized )
    return structoffset( CorticalEdge, refSS1nearest_valid );
  return structoffset( CorticalEdge, SS1nearest_valid );
}


int InterFoldDescr4::corticalSS2NearestOffset( bool normalized, const Clique*, 
                                               int, int ) const
{
  if( normalized )
    return structoffset( CorticalEdge, refSS2nearest );
  return structoffset( CorticalEdge, SS2nearest );
}


int InterFoldDescr4::corticalSS2NearestValidOffset( bool normalized, 
                                                    const Clique*, 
                                                    int, int ) const
{
  if( normalized )
    return structoffset( CorticalEdge, refSS2nearest_valid );
  return structoffset( CorticalEdge, SS2nearest_valid );
}


int InterFoldDescr4::junctionLengthOffset( bool normalized, const Clique*, 
                                           int, int ) const
{
  if( normalized )
    return structoffset( JunctionEdge, reflength );
  return structoffset( JunctionEdge, length );
}


int InterFoldDescr4::junctionLengthValidOffset( bool normalized, 
                                                const Clique*, int, int ) const
{
  if( normalized )
    return structoffset( JunctionEdge, reflength_valid );
  return structoffset( JunctionEdge, length_valid );
}


int InterFoldDescr4::junctionDepthOffset( bool normalized, const Clique*, 
                                          int, int ) const
{
  if( normalized )
    return structoffset( JunctionEdge, refmaxdepth );
  return structoffset( JunctionEdge, maxdepth );
}


int InterFoldDescr4::junctionDepthValidOffset( bool normalized, const Clique*, 
                                               int, int ) const
{
  if( normalized )
    return structoffset( JunctionEdge, refmaxdepth_valid );
  return structoffset( JunctionEdge, maxdepth_valid );
}


int InterFoldDescr4::hullJunctionDirectionOffset( bool normalized, 
                                                  const Clique*, int, 
                                                  int ) const
{
  if( normalized )
    return structoffset( HullJunctionEdge, refdirection );
  return structoffset( HullJunctionEdge, direction );
}


int InterFoldDescr4::hullJunctionDirectionValidOffset( bool normalized, 
                                                       const Clique*, int, 
                                                       int ) const
{
  if( normalized )
    return structoffset( HullJunctionEdge, refdirection_valid );
  return structoffset( HullJunctionEdge, direction_valid );
}


int InterFoldDescr4::hullJunctionExtremity1Offset( bool, // normalized, 
                                                   const Clique*, int, 
                                                   int ) const
{
  // if( normalized )
  return structoffset( HullJunctionEdge, refextremity1 );
  // return structoffset( HullJunctionEdge, extremity1 );
}


int InterFoldDescr4::hullJunctionExtremity1ValidOffset( bool, // normalized, 
                                                        const Clique*, int, 
                                                        int ) const
{
  // if( normalized )
  return structoffset( HullJunctionEdge, refextremity1_valid );
  // return structoffset( HullJunctionEdge, extremity1_valid );
}


int InterFoldDescr4::hullJunctionExtremity2Offset( bool ,//normalized, 
                                                   const Clique*, int, 
                                                   int ) const
{
  // if( normalized )
  return structoffset( HullJunctionEdge, refextremity2 );
  // return structoffset( HullJunctionEdge, extremity2 );
}


int InterFoldDescr4::hullJunctionExtremity2ValidOffset( bool, // normalized, 
                                                        const Clique*, int, 
                                                        int ) const
{
  // if( normalized )
  return structoffset( HullJunctionEdge, refextremity2_valid );
  // return structoffset( HullJunctionEdge, extremity2_valid );
}


int InterFoldDescr4::pliDePassageDepthOffset( bool normalized, const Clique*, 
                                              int, int ) const
{
  if( normalized )
    return structoffset( PliDePassageEdge, refdepth );
  return structoffset( PliDePassageEdge, depth );
}


int InterFoldDescr4::pliDePassageDepthValidOffset( bool normalized, 
                                                   const Clique*, 
                                                   int, int ) const
{
  if( normalized )
    return structoffset( PliDePassageEdge, refdepth_valid );
  return structoffset( PliDePassageEdge, depth_valid );
}


