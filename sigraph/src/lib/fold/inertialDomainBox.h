
#ifndef SI_FOLD_INERTIALDOMAINBOX_H
#define SI_FOLD_INERTIALDOMAINBOX_H


#include <si/fold/domainBox.h>
#include <cartodata/volume/volume.h>


namespace sigraph
{

  class InertialDomainBox : public DomainBox
  {
  public:
    InertialDomainBox();
    InertialDomainBox( const InertialDomainBox & dom );
    virtual ~InertialDomainBox();
    virtual Domain* clone() const;
    ///	Renvoie si le point peut se trouver aux coordonnées (x, y, z)
    bool canBeFound( double x, double y, double z );
    /**	Renvoie si le noeud de graphe peut appartenir au modèle concerné
	(donc avoir le label considéré) 
	@param	v	noeud à tester. Il doit posséder l'attribut 
	{\tt "gravity_center"} pour être utilisable
	@param	g	graphe contenant le noeud
    */
    virtual bool canBeFound( const Vertex* v, const Graph* g = 0 );
    virtual void learn( const Vertex* v, const Graph* g = 0 );
    ///	apprends 2e phase, bornes des buckets dans le repère d'inertie
    virtual void learnBucket( const Vertex* v, const Graph* g );
    virtual void reset();
    virtual bool needsMorePasses() const { return( !_transfUpToDate ); }
    virtual void nextPass() { if( !_transfUpToDate ) diagonalize(); }
    virtual void firstPass();
    virtual void buildTree( Tree & tr ) const;
    static void buildInertialDomBox( Tree* parent, Tree* tr );
    virtual void cubeTalairach( std::vector<std::vector<double> > 
				& pts ) const;

    ///	Diagonalise la matrice d'inertie
    virtual void diagonalize();
    ///	Change de repère vers celui de la matrice d'inertie
    virtual void changeRef( double & x, double & y, double & z );
    float tolerenceMargin() const { return( _tolMargin ); }
    void setTolerenceMargin( float tm ) { _tolMargin = tm; }

  protected:
    ///	Apprends un voxel (2e passe)
    void learnVoxel( const std::vector<float> & rot, 
		     const std::vector<float> & scale, 
		     const std::vector<float> & transl, 
		     const std::vector<float> & vsz, 
		     const AimsVector<short, 3> & pt );
    ///	Apprends un voxel (2e passe) pour un point dans Talairach
    void learnTalVoxel( double x, double y, double z );

    ///	Matrice d'inertie
    carto::VolumeRef<float>	_inertia;
    ///	Centre de gravité
    Point3df	_gravity;
    ///	Matrice de rotation
    carto::VolumeRef<float>	_rotation;
    ///	Valeurs propres de la matrice d'inertie
    Point3df	_eigenValues;
    unsigned	_npoints;
    ///	Flag
    bool		_transfUpToDate;
    ///	Marge de tolérence supplémentaire
    float		_tolMargin;
  };


  //	inline

  inline InertialDomainBox::InertialDomainBox( const InertialDomainBox & dom )
    : DomainBox( dom ), _inertia( dom._inertia ), _gravity( dom._gravity ), 
      _rotation( dom._rotation ), _eigenValues( dom._eigenValues ), 
      _npoints( dom._npoints), _transfUpToDate( dom._transfUpToDate ), 
      _tolMargin( dom._tolMargin )
  {
  }


  inline Domain* InertialDomainBox::clone() const
  {
    return( new InertialDomainBox( *this ) );
  }


  inline InertialDomainBox::~InertialDomainBox()
  {
  }

}

#endif


