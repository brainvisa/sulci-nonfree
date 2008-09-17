#ifndef SI_FOLD_INTERFOLDDESCR4_H
#define SI_FOLD_INTERFOLDDESCR4_H


#include <si/fold/interFoldDescr2.h>


namespace sigraph
{

  class InterFoldDescr4 : public InterFoldDescr2
  {
  public:
    ///	Liste des entrées
    enum Inputs
      {
	VEC_VALID, 

	SIZE_S1, 
	NCC_S1, 
	NCC_S1_REL, 

	SIZE_S2, 
	NCC_S2, 
	NCC_S2_REL, 

	SIZE_CORT, 
	DIST_MIN, 
	DIRX, 
	DIRY, 
	DIRZ, 
	DIST_EXTR_MIN1, 
	DIST_EXTR_MIN2, 
	GRELX, 
	GRELY, 
	GRELZ, 
	PSCAL_CORT, 

	NJONC, 
	MINDEPTH_JONC, 
	SIZE_JONC, 

	NPP, 
	MAXDEPTH_PP, 

	END
      };

    InterFoldDescr4();
    InterFoldDescr4( const InterFoldDescr4 & ifd );
    virtual ~InterFoldDescr4();
    virtual CliqueDescr* clone() const;

    virtual void buildTree( Tree & t );
    virtual std::vector<std::string> descriptorsNames() const;
    virtual std::string name() const;

    // attributes switches
    virtual std::string foldSurfaceAttribute( bool normalized, const Clique*, 
                                              int major, int minor ) const;
    virtual std::string corticalLengthAttribute( bool normalized, 
                                                 const Clique*, 
                                                 int major, int minor ) const;
    virtual std::string corticalDistanceAttribute( bool normalized, 
                                                   const Clique*, int major, 
                                                   int minor ) const;
    virtual std::string corticalSS1NearestAttribute( bool normalized, 
                                                     const Clique*, int major, 
                                                     int minor ) const;
    virtual std::string corticalSS2NearestAttribute( bool normalized, 
                                                     const Clique*, int major, 
                                                     int minor ) const;
    virtual std::string 
    hullJunctionDirectionAttribute( bool normalized, const Clique*, int major, 
                                    int minor ) const;
    virtual std::string 
    hullJunctionExtremity1Attribute( bool normalized, const Clique*, 
                                     int major, int minor ) const;
    virtual std::string 
    hullJunctionExtremity2Attribute( bool normalized, const Clique*, 
                                     int major, int minor ) const;
    virtual std::string junctionLengthAttribute( bool normalized, 
                                                 const Clique*, 
                                                 int major, int minor ) const;
    virtual std::string junctionDepthAttribute( bool normalized, 
                                                const Clique*, 
                                                int major, int minor ) const;
    virtual std::string pliDePassageDepthAttribute( bool normalized, 
                                                    const Clique*, int major, 
                                                    int minor ) const;

  protected:
    // builtin attributes offsets
    virtual int foldSurfaceOffset( bool normalized, const Clique*, 
                                   int major, int minor ) const;
    virtual int foldSurfaceValidOffset( bool normalized, const Clique*, 
                                        int major, int minor ) const;
    virtual int corticalLengthOffset( bool normalized, const Clique*, 
                                      int major, int minor ) const;
    virtual int corticalLengthValidOffset( bool normalized, const Clique*, 
                                           int major, int minor ) const;
    virtual int corticalDistanceOffset( bool normalized, const Clique*, 
                                        int major, int minor ) const;
    virtual int corticalDistanceValidOffset( bool normalized, const Clique*, 
                                             int major, int minor ) const;
    virtual int corticalSS1NearestOffset( bool normalized, const Clique*, 
                                          int major, int minor ) const;
    virtual int corticalSS1NearestValidOffset( bool normalized, const Clique*, 
                                               int major, int minor ) const;
    virtual int corticalSS2NearestOffset( bool normalized, const Clique*, 
                                          int major, int minor ) const;
    virtual int corticalSS2NearestValidOffset( bool normalized, const Clique*, 
                                               int major, int minor ) const;
    virtual int junctionLengthOffset( bool normalized, const Clique*, 
                                      int major, int minor ) const;
    virtual int junctionLengthValidOffset( bool normalized, const Clique*, 
                                           int major, int minor ) const;
    virtual int junctionDepthOffset( bool normalized, const Clique*, 
                                     int major, int minor ) const;
    virtual int junctionDepthValidOffset( bool normalized, const Clique*, 
                                          int major, int minor ) const;
    virtual int hullJunctionDirectionOffset( bool normalized, const Clique*, 
                                             int major, int minor ) const;
    virtual int hullJunctionDirectionValidOffset( bool normalized, 
                                                  const Clique*, 
                                                  int major, int minor ) const;
    virtual int hullJunctionExtremity1Offset( bool normalized, const Clique*, 
                                              int major, int minor ) const;
    virtual int hullJunctionExtremity1ValidOffset( bool normalized, 
                                                   const Clique*, int major, 
                                                   int minor ) const;
    virtual int hullJunctionExtremity2Offset( bool normalized, const Clique*, 
                                              int major, int minor ) const;
    virtual int hullJunctionExtremity2ValidOffset( bool normalized, 
                                                   const Clique*, int major, 
                                                   int minor ) const;
    virtual int pliDePassageDepthOffset( bool normalized, const Clique*, 
                                         int major, int minor ) const;
    virtual int pliDePassageDepthValidOffset( bool normalized, const Clique*, 
                                              int major, int minor ) const;
  };

  //	inline

  inline InterFoldDescr4::InterFoldDescr4() 
    : InterFoldDescr2()
  {
  }


  inline InterFoldDescr4::InterFoldDescr4( const InterFoldDescr4 & ifd )
    : InterFoldDescr2( ifd )
  {
  }


  inline CliqueDescr* InterFoldDescr4::clone() const
  {
    return( new InterFoldDescr4( *this ) );
  }

}

#endif

