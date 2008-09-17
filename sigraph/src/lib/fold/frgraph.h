/*
 *  Copyright (C) 1998-2003 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 */

#ifndef SI_FOLD_FRGRAPH_H
#define SI_FOLD_FRGRAPH_H


#include <si/graph/mgraph.h>


namespace sigraph
{
  class Model;

  /**	Folds Random graph
   */
  class FRGraph : public MGraph
  {
  public:
    /**@name	Constructeur(s) - destructeur */
    //@{
    /**	Constructeur
	@param	synt		attribut syntaxique du graphe
	@param	clqDescr	type de descripteur de cliques associé
    */
    FRGraph( const std::string synt = "", 
	     const std::string clqDescr = "standard1" );
    ///
    virtual ~FRGraph();
    //@}

    /**@name	Fonctions propres */
    //@{
    ///	Renvoie un descripteur de clique adapté au modèle
    virtual ModelFinder & modelFinder();
    /**	Ajoute des "random edges" en liant des labels qui sont effectivement 
	liés dans le graphe exemple.
	Les relations ajoutées ont un attribut (non sauvegardé "{\tt 
	occurence_count}" qui donne le nombre de fois que cette relation a été 
	rencontrée dans les exemples. Cet attribut est utilisé par la fonction 
	{\tt removeRareEdges} pour éliminer celles qui sont inhabituelles.
    */
    virtual void addEdges( const Graph & gr, const Model* mod );
    ///	Crée un "random edge"
    virtual Edge* makeEdge( Vertex* v1, Vertex* v2, 
			    const std::string & label1, 
			    const std::string & label2, const Model* mod );

    ///	Crée des fichiers .tri avec les boîtes des domaines
    virtual void createTriangDomainFiles( const std::string & dir );

    ///	Crée la relation FoldFakeRel
    virtual void createFakeRel();
    //@}

  protected:

  };

}


#endif

