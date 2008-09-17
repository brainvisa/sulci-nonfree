/*
 *  Copyright (C) 1998-2004 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 */

#ifndef SI_GRAPH_CLIQUE_H
#define SI_GRAPH_CLIQUE_H

#include <set>
#include <cartobase/object/attributed.h>


namespace sigraph
{

  /**	Classe Clique. 
	Une clique contient la liste des Vertex qui la forment. Pour 
	passer rapidement d'un Vertex à la liste des cliques qui le 
	contiennent, on stocke dans les Vertex un attribut "cliques", 
	qui est un pointeur sur un set<Clique*>. Cette liste de cliques 
	est maintenue à jour par les cliques elles-mêmes (elles s'enregistrent 
	et se désenregistrent quand le Vertex est ajouté ou enlevé de la 
	clique, ou quand la clique est détruite), et effacée quand elle est 
	vide (l'attribut "cliques" également). 
	Clique dérive de AttributedObject pour permettre le 
	stockage d'attributs, temporaires ou définitifs.
  */
  class Clique : public carto::AttributedObject
  {
  public:
    virtual ~Clique();
    virtual void clear();

    /**	copies the clique and the elements it refers too

	For a VertexClique operating on graph vertices, the vertices are 
	copied (althrough they are in the general case only pointers to the 
	nodes inside the original graph).

	This behaviour is needed for cliques modifications that will also 
	affect the elements referred, such as noising data during learning.
	Copied cliques are requested to set their internal attribute 
	\c "is_copy" (of type \c bool) to true, so we know when we have a 
	copied clique which may have allocated data (and must free it upon 
	destruction)

	Inherited classes <it>MUST</it> define this important function

	\return copied clique
    */
    virtual Clique* deepCopy() const = 0;

  protected:
    Clique();
    ///	Ne copie que les attributs, pas les noeuds contenus !
    Clique( const Clique & cl );

  private:
  };

  //	Fonctions inline

  inline Clique::Clique() : carto::AttributedObject( "clique" )
  {
  }

}


namespace carto
{
  DECLARE_GENERIC_OBJECT_TYPE( std::set<sigraph::Clique *> * )
}

#endif


