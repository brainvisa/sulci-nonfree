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

#ifndef SI_GRAPH_CGRAPH_H
#define SI_GRAPH_CGRAPH_H


#include <graph/graph/graph.h>
#include <si/graph/clique.h>


namespace sigraph
{
  class MGraph;

  /**	Graphes avec �tiquettes et gestion de cliques (classe abstraite).

	Ce type de graphe est utilis� pour les graphes exemples � �tiqueter 
	(utilis�s en particulier pour le recuit simul�, voir classe Anneal).

	La fonction \c randLabels est d�finie par d�faut pour assigner � 
	chaque noeud un label al�atoirement choisi parmi ceux autoris�s pour 
	lui (ceux de la liste \c "possible_labels" ). Elle peut �tre 
	red�finie au besoin pour une initialisation plus fine.

	Les �tiquettes des noeuds sont stock�s dans l'attribut \c "label".
  */
  class CGraph : public Graph
  {
  public:
    typedef std::set<carto::rc_ptr<Clique> > CliqueSet;

    CGraph( std::string s = "" );
    virtual ~CGraph();

    virtual void deleteCliques();
    const CliqueSet & cliques() const;
    CliqueSet & cliques();

    virtual void randLabels();
    ///	Met le label donn� sur tous les noeuds
    void setAllLabels( const std::string & label );
    ///	Elimine les labels impossibles au d�part
    void ensureAllLabelsPossible();

    virtual void clearAll();
    virtual void loadBuckets( const std::string & basename, 
                              bool rels = false );

  protected:
    virtual void parseDelete( carto::AttributedObject *ao );

    CliqueSet	_cliques;

  };


  //	Fonctions inline


  inline const CGraph::CliqueSet & CGraph::cliques() const
  {
    return _cliques;
  }


  inline CGraph::CliqueSet & CGraph::cliques()
  {
    return _cliques;
  }

}

#endif

