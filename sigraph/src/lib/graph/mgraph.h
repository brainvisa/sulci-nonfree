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

#ifndef SI_GRAPH_MGRAPH_H
#define SI_GRAPH_MGRAPH_H


#include <graph/graph/graph.h>


namespace sigraph
{
  class ModelFinder;

  /**	Graph mod�le (classe abstraite).


  Un graphe mod�le est un graphe dont les noeuds contiennent des mod�les 
  (Model). Pour �tre utilisable, une classe d�rivant de  
  MGraph doit comprendre les �l�ments et fonctions suivants:

  - S�lecteur de mod�le: la fonction MGraph::modelFinder renvoie 
    une r�f�rence sur un s�lecteur (d�riv� de ModelFinder). Le 
    moyen le plus simple est de le garder dans l'attribut du graphe \c 
    "model_finder", soit en le cr�ant dans le constructeur, soit 
    en chargeant le fichier graphe (auquel cas le \c Reader associ� 
    doit effectuer cette op�ration). Une fois stock� dans l'attribut \c 
    "model_finder", il est automatiquement d�truit par le 
    destructeur de MGraph.
  - Les noeuds ont l'attribut \c "model" qui pointe sur un mod�le 
    d'�valuation de clique. Les mod�les sont d�truits automatiquement.
  - Les noeuds du graphe mod�le ont g�n�ralement un attribut \c 
    "domain"} (pointeur sur un Domain) qui donne le domaine de 
    validit� du mod�le. Le domaine est souvent utilis� pour 
    initialiser les �tiquettes possibles pour les noeuds des graphes 
    � �tiqueter (CGraph). Les domaines sont optinnels, et sont 
    automatiquement d�truits par le destructeur de MGraph.
  */
  class MGraph : public Graph
  {
  public:
    /// used for model-data version compatibility check
    enum VersionStatus
      {
        Unversioned, 
        VersionOk, 
        Outdated
      };

    /// Model-data version check result
    struct VersionCheck
    {
      VersionCheck() : ok( false ), model( Unversioned ), data( Unversioned )
      {}
      bool		ok;
      VersionStatus	model;
      VersionStatus	data;
      /// error / warning message
      std::string	message;
    };

    virtual ~MGraph();

    ///	Renvoie un s�lecteur de mod�le adapt� au graphe mod�le
    virtual ModelFinder & modelFinder() = 0;
    virtual void clearAll();
    virtual std::string domainFile( const std::string & graphname, 
				    const carto::AttributedObject* ao );
    virtual std::string modelFile( const std::string & graphname, 
				   const carto::AttributedObject* ao, 
				   std::set<std::string> & otherFiles );
    ///	Ferme l'apprentissage de tous les mod�les
    virtual void closeLearning();

    ///	Initialise les stats
    virtual void initStats();
    ///	Initialise les partie adaptatives (r�seaux de neurones et autres)
    virtual void initAdap();
    ///	Initialise les domaines d'influence
    virtual void initDomain();
    ///	Elimine les noeuds et relations non-utilis�s (de domaine vide)
    virtual void removeUnusedModels( bool removeFiles = false, 
				     const std::string & prefix = "" );
    /**	Enl�ve les relations peu fr�quentes (max. {\tt num} occurences dans 
	l'attribut "{\tt occurence_count}") 
	@return	nombre de relations �limin�es
    */
    virtual unsigned removeRareEdges( float freqmin );
    ///	Enl�ve les relations avec le label 'brain'
    virtual void removeEdgesToVoid();
    ///	Fixe les poids des mod�les des noeuds
    virtual void setWeights( double factor = 1. );
    ///	Enl�ve les poids des mod�les des noeuds
    virtual void removeWeights();

    /** Checks compatibility between this model graph and the given 
        data graph. Basically, versions and compatibility versions of 
        both graphs are compared, but this function can be overriden to 
        check more precise things.

        4 properties govern the version checking:
        - in data graphs:
          - datagraph_VERSION is the current version of a data graph (folds, 
            ROI, clusters etc). By default it can be the current carto library 
            number
          - datagraph_compatibility_model_VERSION is the minimum model graph 
            version number that can deal with it
        - in model graphs:
          - model_VERSION: verison of the model graph
          - model_compatibility_data_VERSION: minimum data graph version 
            number that is accepted by this model

        Using these 4 attributes, I think we can determine a correct behaviour 
        even for models/data that are not known yet
    */
    virtual VersionCheck checkCompatibility( const Graph & data ) const;

  protected:
    MGraph( const std::string synt );

    virtual void parseDelete( carto::AttributedObject * ao );
  };

  //	Fonctions inline

  inline MGraph::MGraph( const std::string synt ) : Graph( synt )
  {
  }

}


namespace carto
{
  DECLARE_GENERIC_OBJECT_TYPE( sigraph::MGraph * )
}

#endif

