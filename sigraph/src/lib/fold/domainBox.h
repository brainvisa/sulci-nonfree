
#ifndef SI_FOLD_DOMAINBOX_H
#define SI_FOLD_DOMAINBOX_H

#include <si/domain/adapDomain.h>
#include <vector>
#include <string>


namespace sigraph
{

  /**	Domaine de validité par boîte englobante
   */
  class DomainBox : public AdapDomain
  {
  public:
    DomainBox();
    DomainBox( const DomainBox & dom );
    virtual ~DomainBox();
    ///	Duplication
    virtual Domain* clone() const;

    /**@name	Utilisation */
    //@{
    ///	Renvoie si le point peut se trouver aux coordonnées (x, y, z)
    bool canBeFound( double x, double y, double z );
    /**	Renvoie si le noeud de graphe peut appartenir au modèle concerné
	(donc avoir le label considéré) 
	@param	v	noeud à tester. Il doit posséder l'attribut 
	\c "gravity_center" pour être utilisable
	@param	g	graphe contenant le noeud
    */
    virtual bool canBeFound( const Vertex* v, const Graph* g = 0 );
    //@}

    /**@name	Apprentissage */
    //@{
    /**	Apprends le domaine d'un morceau de sillon donné
	@param	v	sillon ou morceau de sillon sous sa forme de noeud de 
	graphe. Le noeud doit contenir un bucket (attribut 
	\c "bucket" ) pour pouvoir être appris
	@param	g	graphe contenant le noeud \c v; des informations 
	globales du graphe peuvent être utilisées par le noeud
    */
    virtual void learn( const Vertex* v, const Graph* g = 0 );
    virtual void reset();
    void setDims( double xmin, double ymin, double zmin, 
		  double xmax, double ymax, double zmax );
    double xmin() const;
    double ymin() const;
    double zmin() const;
    double xmax() const;
    double ymax() const;
    double zmax() const;
    const std::string & gravityCenterAttribute() const { return _gcattrib; }
    void setGravityCenterAttribute( const std::string & att );
    /**	Transforme le vecteur {\tt (v1, v2, v3)} dans les coordonnées 
	de Talairach (transformation dans le graphe {\tt g})
    */
    virtual void talairach( float & v1, float & v2, float & v3, 
			    const Graph* g ) const;
    /**	Donne les coordonnées des coins dans Talairach (pas forcément les 
	minmax internes, cf InertialDomainBox), remplit le vecteur avec les 
	8 points*/
    virtual void cubeTalairach( std::vector<std::vector<double> > 
				& pts ) const;
    //@}

    /**@name	IO */
    //@{
    ///	Conversion en arbre (pour IO)
    virtual void buildTree( Tree & tr ) const;
    static void buildDomBox( Tree* parent, Tree* tr );
    //@}

  protected:
    double	_xmin;
    double	_ymin;
    double	_zmin;
    double	_xmax;
    double	_ymax;
    double	_zmax;
    std::string	_gcattrib;

  private:

  };

  //	Fonctions inline

  inline Domain* DomainBox::clone() const
  {
    return( new DomainBox( *this ) );
  }


  inline DomainBox::~DomainBox()
  {
  }


  inline void DomainBox::setDims( double xmin, double ymin, 
				  double zmin, double xmax, double ymax, 
				  double zmax )
  {
    _xmin = ( xmin < xmax ) ? xmin : xmax;
    _xmax = ( xmin < xmax ) ? xmax : xmin;
    _ymin = ( ymin < ymax ) ? ymin : ymax;
    _ymax = ( ymin < ymax ) ? ymax : ymin;
    _zmin = ( zmin < zmax ) ? zmin : zmax;
    _zmax = ( zmin < zmax ) ? zmax : zmin;
  }


  inline double DomainBox::xmin() const
  {
    return( _xmin );
  }


  inline double DomainBox::xmax() const
  {
    return( _xmax );
  }


  inline double DomainBox::ymin() const
  {
    return( _ymin );
  }


  inline double DomainBox::ymax() const
  {
    return( _ymax );
  }


  inline double DomainBox::zmin() const
  {
    return( _zmin );
  }


  inline double DomainBox::zmax() const
  {
    return( _zmax );
  }


  inline bool DomainBox::canBeFound( double x, double y, double z )
  {
    if( nData() )
      return( (x >= _xmin) && (x <= _xmax) && (y >= _ymin) && (y <= _ymax) 
	      && (z >= _zmin) && (z <= _zmax) );
    else return( true );	// si pas appris, la reponse est oui par défaut
  }

}

#endif

