
#ifndef SI_FOLD_FOLDDESCR2_H
#define SI_FOLD_FOLDDESCR2_H


#include <si/descr/adapDescr.h>
#include <si/fold/frgraph.h>


namespace sigraph
{

  /**	Descripteur de sillon version 2. \\ \\
	Description des vecteurs d'entrée des modèles sillon: \\ \\
	Voir l'enum {\Ref Inputs} pour la liste
	\\ \\
	Après stats, les entrées 5,6,7 sont recalées avec l'orientation 
	moyenne des stats. \\ \\
	Cliques de type "arc aléatoire" :
	\\ (rien pour l'instant)
  */
  class FoldDescr2 : public AdapDescr
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

	NCC, 
	NCC_NOT_CORTICAL, 
	NCORT, 
	DISTMAX_CC, 		// dist. max entre CC (sans corticales)
	NPLIS, 
	SIZE_HULLJUNC, 

	END
      };

    enum NormalizedMode
      {
        NormalizedNone = 0, 
        Normalized, 
        NormalizedBoth 
      };

    FoldDescr2();
    FoldDescr2( const FoldDescr2 & f );
    virtual ~FoldDescr2();
    virtual CliqueDescr* clone() const;

    /**	Remplit le vecteur d'entrées. 
	@param	cl	clique à décrire
	@param	vec	(RETOUR) vecteur qui décrit la clique
	@return		dit si ça s'est bien passé
    */
    virtual bool makeVector( const Clique* cl, std::vector<double> & vec, 
			     carto::GenericObject* model = 0 );
    /**	Effectue des pré-processings des stats d'orientation. 
	Fonction appelée par {\tt potential()} et {\tt learn()} */
    virtual void preProcess( std::vector<double> & vec, 
			     carto::GenericObject* model = 0 );
    virtual bool hasChanged( const Clique* cl, 
			     const std::map<Vertex*, std::string> & changes, 
			     const carto::GenericObject* model = 0 ) const;
    virtual void buildTree( Tree & t );
    virtual std::vector<std::string> descriptorsNames() const;
    virtual std::string name() const;

    virtual bool makeLearnVector( const Clique* cl, std::vector<double> & vec, 
				  carto::GenericObject* model = 0,
				  double outp = 0 );
    virtual void handleStats( const Clique* cl, std::vector<double> & vec, 
			      carto::GenericObject* model = 0,
			      double outp = 0 );
    virtual void reset();
    void setNormalizedMode( NormalizedMode x ) { _normalized = x; }
    NormalizedMode normalizedMode() const { return _normalized; }

    void setNStats( unsigned n ) { _nnorm = n; }
    void setNormal( double nx, double ny, double nz )
    { _nx = nx; _ny = ny; _nz = nz; }
    void setE12Stats( unsigned n ) { _nE1E2 = n; }
    void setE1E2( double x, double y, double z )
    { _e12x = x; _e12y = y; _e12z = z; }
    void setDirStats( unsigned n ) { _nDirHJ = n; }
    void setDirection( double x, double y, double z )
    { _dHJx = x; _dHJy = y; _dHJz = z; }

    /// data graph version test
    static void checkDataGraphVersion( const Clique*, int & major, 
                                       int & minor );
    /** \return true if the data graph version is greater or equal to the 
        given numbers.
    */
    static bool dataVersionGE( int major, int minor, 
                               int datamajor, int dataminor );

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
    virtual std::string hullJunctionLengthAttribute( bool normalized, 
                                                     const Clique*, int major, 
                                                     int minor ) const;
    virtual std::string 
    hullJunctionExtremity1Attribute( bool normalized, const Clique*, 
                                     int major, int minor ) const;
    virtual std::string 
    hullJunctionExtremity2Attribute( bool normalized, const Clique*, 
                                     int major, int minor ) const;
    virtual std::string 
    hullJunctionDirectionAttribute( bool normalized, const Clique*, int major, 
                                    int minor ) const;
    virtual std::string corticalDistanceAttribute( bool normalized, 
                                                   const Clique*, int major, 
                                                   int minor ) const;

  protected:
    virtual bool makeVectorElements( const Clique* cl, 
                                     std::vector<double> & vec, 
                                     carto::GenericObject* model = 0 );
    // builtin attributes offsets
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

  private:
    ///	Stats des normales
    unsigned	_nnorm;
    double	_nx;
    double	_ny;
    double	_nz;
    ///	Nb stats E1E2
    unsigned	_nE1E2;
    double	_e12x;
    double	_e12y;
    double	_e12z;
    ///	Nb stats Direction hull_junction
    unsigned	_nDirHJ;
    double	_dHJx;
    double	_dHJy;
    double	_dHJz;

    NormalizedMode	_normalized;
  };


  //	inline


  inline CliqueDescr* FoldDescr2::clone() const
  {
    return( new FoldDescr2( *this ) );
  }

}

#endif

