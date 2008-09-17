
#ifndef SI_DOMAIN_ADAPDOMAIN_H
#define SI_DOMAIN_ADAPDOMAIN_H


#include <si/domain/domain.h>
class Vertex;
class Graph;


namespace sigraph
{

  /**	Domaine de validité adaptatif.

	Particularité: fonction learn( Vertex* )
  */
  class AdapDomain : public Domain
  {
  public:
    virtual ~AdapDomain();
    ///	Duplication (fonction abstraite)
    virtual Domain* clone() const = 0;

    /**@name	Apprentissage */
    //@{
    /**	Apprends le domaine d'un noeud exemple donné
	@param	v	noeud de graphe. Le noeud doit contenir un bucket 
	(attribut {\tt "bucket"}) pour pouvoir être appris
	@param	g	graphe contenant le noeud {\tt v}; des informations 
	globales du graphe peuvent être utilisées par le noeud
    */
    virtual void learn( const Vertex* v, const Graph* g = 0 );
    unsigned nData() const;
    void setNData( unsigned ndata );
    ///	Réinitialise l'apprentissage (remet les stats à zéro)
    virtual void reset();
    /**	Dit s'il faut encore un passage sur une base pour que l'apprentissage 
	soit complet.
	Certains modèles peuvent apprendre en plieurs phases: par exemple, 
	réglages de rotations / centrages, puis apprentissage des bornes dans 
	le nouveau repère (cf InertialDomainBox dans le package fold).
    */
    virtual bool needsMorePasses() const { return( false ); }
    ///	Effectue le changement de phase d'apprentissage
    virtual void nextPass() {}
    ///	Revient à la première passe d'apprentissage
    virtual void firstPass() {}
    //@}

    /**@name	IO */
    //@{
    ///	Conversion en arbre (pour IO)
    virtual void buildTree( Tree & tr ) const;
    //@}

  protected:
    AdapDomain();
    AdapDomain( const AdapDomain & dom );

    unsigned	_ndata;

  private:
  };

  //	Fonctions inline

  inline AdapDomain::AdapDomain() : Domain(), _ndata( 0 )
  {
  }


  inline AdapDomain::AdapDomain( const AdapDomain & dom ) 
    : Domain( dom ), _ndata( dom._ndata )
  {
  }


  inline AdapDomain::~AdapDomain()
  {
  }


  inline unsigned AdapDomain::nData() const
  {
    return( _ndata );
  }


  inline void AdapDomain::setNData( unsigned ndata )
  {
    _ndata = ndata;
  }


  inline void AdapDomain::reset()
  {
    _ndata = 0;
  }

}

#endif


