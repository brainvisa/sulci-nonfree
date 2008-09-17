
#ifndef SI_FOLD_FOLDDESCR_H
#define SI_FOLD_FOLDDESCR_H


#include <si/descr/adapDescr.h>
#include <si/fold/frgraph.h>


namespace sigraph
{

  /**	Descripteur de sillon. \\ \\
	Description des vecteurs d'entrée des modèles sillon: \\ \\
	Voir l'enum {\Ref Inputs} pour la liste
	\\ \\
	Après stats, les entrées 5,6,7 sont recalées avec l'orientation 
	moyenne des stats. \\ \\
	Cliques de type "arc aléatoire" :
	\\ (rien pour l'instant)
  */
  class FoldDescr : public AdapDescr
  {
  public:
    /**@name	Types, enums */
    //@{
    ///	Liste des entrées
    enum Inputs
      {
	///
	VEC_VALID, 
	///
	GX, 
	///
	GY, 
	///
	GZ, 
	///
	SIZE, 
	///
	DEPTH, 
	///
	NORM_VALID, 
	///
	NX, 
	///
	NY, 
	///
	NZ, 
	///
	NBIG, 
	///
	NSMALL, 
	///
	CORT_SURF, 
	///
	HJUNC_SURF, 
	///
	CORT_CC, 
	///
	JUNC_CC, 
	///	Marque la fin
	END
      };
    //@}

    /**@name	Constructeur(s) - destructeur */
    //@{
    ///
    FoldDescr();
    ///
    FoldDescr( const FoldDescr & f );
    ///
    virtual ~FoldDescr();
    ///
    virtual CliqueDescr* clone() const;
    //@}

    /**@name	Hérité de CliqueDescr */
    //@{
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
    ///
    virtual bool hasChanged( const Clique* cl, 
			     const std::map<Vertex*, std::string> & changes, 
			     const carto::GenericObject* model = 0 ) const;
    ///	IO
    virtual void buildTree( Tree & t );
    //@}

    /**@name	Hérité d'AdapDescr */
    //@{
    ///
    virtual bool makeLearnVector( const Clique* cl, std::vector<double> & vec, 
				  carto::GenericObject* model = 0,
				  double outp = 0 );
    ///
    virtual void handleStats( const Clique* cl, std::vector<double> & vec, 
			      carto::GenericObject* model = 0,
			      double outp = 0 );
    ///
    virtual void reset();
    //@}

    /**@name	Fonctions propres */
    //@{
    ///
    void setNStats( unsigned n ) { _nnorm = n; }
    ///
    void setNormal( double nx, double ny, double nz )
    { _nx = nx; _ny = ny; _nz = nz; }
    ///
    double limitSize() const { return( _limitSize ); }
    ///
    void setLimitSize( double lim ) { _limitSize = lim; }
    //@}

  protected:
    virtual bool makeVectorElements( const Clique* cl, 
                                     std::vector<double> & vec, 
                                     carto::GenericObject* model = 0 );
    /**@name	Fonctions protégées */
    //@{
    //@}

  private:
    ///	Stats des normales
    unsigned	_nnorm;
    ///
    double	_nx;
    ///
    double	_ny;
    ///
    double	_nz;
    ///	limite gros - petit bout de sillon
    double	_limitSize;
  };


  //	inline


  inline CliqueDescr* FoldDescr::clone() const
  {
    return( new FoldDescr( *this ) );
  }

}

#endif

