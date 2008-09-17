
#ifndef SI_FOLD_INTERFOLDDESCR2_H
#define SI_FOLD_INTERFOLDDESCR2_H


#include <si/descr/adapDescr.h>
#include <set>


namespace sigraph
{

  class InterFoldDescr2 : public AdapDescr
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

    enum NormalizedMode
      {
        NormalizedNone = 0, 
        Normalized, 
        NormalizedBoth 
      };

    InterFoldDescr2();
    InterFoldDescr2( const InterFoldDescr2 & ifd );
    virtual ~InterFoldDescr2();
    virtual CliqueDescr* clone() const;

    /**	Remplit le vecteur d'entrées. 
     */
    virtual bool makeVector( const Clique* cl, std::vector<double> & vec, 
			     carto::GenericObject* model = 0 );
    virtual bool hasChanged( const Clique* cl, 
			     const std::map<Vertex*, std::string> & changes, 
			     const carto::GenericObject* model = 0 ) const;
    virtual void buildTree( Tree & t );
    virtual std::vector<std::string> descriptorsNames() const;
    virtual std::string name() const;

    virtual bool makeLearnVector( const Clique* cl, std::vector<double> & vec, 
				  carto::GenericObject* model = 0 );

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
    hullJunctionDirectionAttribute( bool normalized, const Clique*, 
                                    int major, int minor ) const;
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
    virtual bool makeVectorElements( const Clique* cl, 
                                     std::vector<double> & vec, 
                                     carto::GenericObject* model = 0 );
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

    void scanFold( const std::set<Vertex *> & sv, float & size, 
		   std::vector<float> & extr1, std::vector<float> & extr2, 
		   double & dmax, bool & hashj1, const Clique*, int vmaj, 
                   int vmin );

  private:
    NormalizedMode	_normalized;
  };

  //	inline

  inline InterFoldDescr2::InterFoldDescr2() 
    : AdapDescr(), _normalized( Normalized )
  {
  }


  inline InterFoldDescr2::InterFoldDescr2( const InterFoldDescr2 & ifd )
    : AdapDescr( ifd ), _normalized( ifd._normalized )
  {
  }


  inline CliqueDescr* InterFoldDescr2::clone() const
  {
    return( new InterFoldDescr2( *this ) );
  }

}

#endif

