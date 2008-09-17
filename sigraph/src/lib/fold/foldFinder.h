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

#ifndef SI_FOLD_FOLDFINDER_H
#define SI_FOLD_FOLDFINDER_H


#include <si/finder/adapFinder.h>
#include <si/fold/frgraph.h>


namespace sigraph
{

  /**	Sélecteur de sillon/edge modèle.
   */
  class FoldFinder : public AdapFinder
  {
  public:
    FoldFinder( MGraph & rg );
    virtual ~FoldFinder();

    ///	Choisit le noeud modèle dans le FRGraph en fonction de la clique
    virtual carto::AttributedObject* selectModel( const Clique* cl );
    virtual void clear();

    virtual void initCliques( CGraph & data, bool verbose = true, 
                              bool withCache = false, 
                              bool translateLabels = true, 
                              bool checkLabels = true, 
                              const aims::SelectionSet *sel = 0 );

  protected:

  private:
    ///	Cache de conversion Clique -> Model
    std::map<const Clique*, carto::AttributedObject*>	_cache;
  };

}

#endif


