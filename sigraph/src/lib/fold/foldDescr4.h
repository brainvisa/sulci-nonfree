
#ifndef SI_FOLD_FOLDDESCR4_H
#define SI_FOLD_FOLDDESCR4_H


#include <si/fold/foldDescr2.h>


namespace sigraph
{

  /**	Fold descriptor, version 4 (11/2005).
  */
  class FoldDescr4 : public FoldDescr2
  {
  public:
    ///	Descriptors list
    enum Inputs
      {
	VEC_VALID,
	E1E2_VALID, 
	E1X, 
	E1Y, 
	E1Z, 
	E2X, 
	E2Y, 
	E2Z, 
	GX, 
	GY, 
	GZ, 

	NVALID, 
	NX, 
	NY, 
	NZ, 
	DIRX, 
	DIRY, 
	DIRZ, 

	SIZE, 
	DEPTH, 
	MINDEPTH, 
        MEANDEPTH, 

	NCC, 
	NCC_NOT_CORTICAL, 
	NCORT, 
	DISTMAX_CC, 
	NPLIS, 
	SIZE_HULLJUNC, 
	
	GM_THICKNESS,
	FOLD_OPENING,

	END
      };

    FoldDescr4();
    FoldDescr4( const FoldDescr4 & f );
    virtual ~FoldDescr4();
    virtual CliqueDescr* clone() const;

    virtual void buildTree( Tree & t );
    virtual std::vector<std::string> descriptorsNames() const;
    virtual std::string name() const;

    // attributes switches
    virtual std::string surfaceAttribute( bool normalized, const Clique*, 
                                          int major, int minor ) const;
    virtual std::string gravityCenterAttribute( bool normalized, const Clique*,
                                                int major, int minor ) const;
    virtual std::string normalAttribute( bool normalized, const Clique*, 
                                         int major, int minor ) const;
    virtual std::string minDepthAttribute( bool normalized, const Clique*, 
                                           int major, int minor ) const;
    virtual std::string maxDepthAttribute( bool normalized, const Clique*, 
                                           int major, int minor ) const;
    virtual std::string meanDepthAttribute( bool normalized, const Clique*, 
                                            int major, int minor ) const;
    virtual std::string junctionLengthAttribute( bool normalized, 
                                                 const Clique*, int major, 
                                                 int minor ) const;
    virtual std::string junctionExtremity1Attribute( bool normalized, 
                                                     const Clique*, int major, 
                                                     int minor ) const;
    virtual std::string junctionExtremity2Attribute( bool normalized, 
                                                     const Clique*, int major, 
                                                     int minor ) const;
    virtual std::string junctionDirectionAttribute( bool normalized, 
                                                    const Clique*, int major, 
                                                    int minor ) const;
    virtual std::string corticalDistanceAttribute( bool normalized, 
                                                   const Clique*, int major, 
                                                   int minor ) const;


  protected:
    virtual bool makeVectorElements( const Clique* cl, 
                                     std::vector<double> & vec, 
                                     carto::GenericObject* model = 0 );
    virtual int surfaceOffset( bool normalized, const Clique*, int major, 
                               int minor ) const;
    virtual int surfaceValidOffset( bool normalized, const Clique*, int major, 
                                    int minor ) const;
    virtual int gravityCenterOffset( bool normalized, const Clique*, 
                                     int major, int minor ) const;
    virtual int gravityCenterValidOffset( bool normalized, const Clique*, 
                                          int major, int minor ) const;
    virtual int normalOffset( bool normalized, const Clique*, int major, 
                              int minor ) const;
    virtual int normalValidOffset( bool normalized, const Clique*, int major, 
                                   int minor ) const;
    virtual int minDepthOffset( bool normalized, const Clique*, int major, 
                                int minor ) const;
    virtual int minDepthValidOffset( bool normalized, const Clique*, 
                                     int major, int minor ) const;
    virtual int maxDepthOffset( bool normalized, const Clique*, int major, 
                                int minor ) const;
    virtual int maxDepthValidOffset( bool normalized, const Clique*, 
                                     int major, int minor ) const;
    virtual int meanDepthOffset( bool normalized, const Clique*, int major, 
                                 int minor ) const;
    virtual int meanDepthValidOffset( bool normalized, const Clique*, 
                                      int major, int minor ) const;
    virtual int hullJunctionLengthOffset( bool normalized, const Clique*, 
                                          int major, int minor ) const;
    virtual int hullJunctionLengthValidOffset( bool normalized, const Clique*, 
                                               int major, int minor ) const;
    virtual int hullJunctionExtremity1Offset( bool normalized, const Clique*, 
                                              int major, int minor ) const;
    virtual int 
    hullJunctionExtremity1ValidOffset( bool normalized, const Clique*, 
                                       int major, int minor ) const;
    virtual int hullJunctionExtremity2Offset( bool normalized, const Clique*, 
                                              int major, int minor ) const;
    virtual int 
    hullJunctionExtremity2ValidOffset( bool normalized, const Clique*, 
                                       int major, int minor ) const;
    virtual int hullJunctionDirectionOffset( bool normalized, const Clique*, 
                                             int major, int minor ) const;
    virtual int 
    hullJunctionDirectionValidOffset( bool normalized, const Clique*, 
                                      int major, int minor ) const;
    virtual int corticalDistanceOffset( bool normalized, const Clique*, 
                                        int major, int minor ) const;
    virtual int corticalDistanceValidOffset( bool normalized, const Clique*, 
                                             int major, int minor ) const;
  };


  //	inline


  inline CliqueDescr* FoldDescr4::clone() const
  {
    return( new FoldDescr4( *this ) );
  }

}

#endif

