
#ifndef SI_DOMAIN_DOMAIN_H
#define SI_DOMAIN_DOMAIN_H

#include <cartobase/object/object.h>

class Vertex;
class Graph;
class Tree;


namespace sigraph
{

  /**	Domaine de validité
   */
  class Domain
  {
  public:
    virtual ~Domain();
    ///	Duplication (fonction abstraite)
    virtual Domain* clone() const = 0;

    ///	Renvoie si le sillon peut se trouver aux coordonnées (x, y, z)
    virtual bool canBeFound( double x, double y, double z ) = 0;
    /**	Renvoie si le noeud de graphe de sillons peut appartenir au sillon 
	concerné(donc avoir le label considéré) 
	@param	v	noeud à tester. Il doit posséder l'attribut 
	{\tt "gravity_center"} pour 
	être utilisable
	@param	g	graphe auquel appartient le noeud (peut contenir des 
	informations globales utiles au noeud)
    */
    virtual bool canBeFound( const Vertex* v, const Graph* g = 0 ) = 0;
    ///	Conversion en arbre (pour IO)
    virtual void buildTree( Tree & tr ) const = 0;

  protected:
    Domain();
    Domain( const Domain & dom );

  private:
  };

  //	Fonctions inline

  inline Domain::Domain()
  {
  }


  inline Domain::Domain( const Domain & )
  {
  }

}


namespace carto
{
  DECLARE_GENERIC_OBJECT_TYPE( sigraph::Domain * )
}

#endif

