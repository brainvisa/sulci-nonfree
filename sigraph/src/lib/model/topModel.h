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

#ifndef SI_MODEL_TOPMODEL_H
#define SI_MODEL_TOPMODEL_H


#include <cartobase/object/attributed.h>
#include <set>
#include <string>

class Tree;


namespace sigraph
{
  class MGraph;

  class TopModel
  {
  public:
    TopModel( carto::AttributedObject* parent = 0 );
    TopModel( const TopModel & tm );
    virtual ~TopModel();
    TopModel & operator = ( const TopModel & tm );

    /**@name	Accès aux listes de labels significatifs */
    //@{
    std::set<std::string> & significantLabels()
    { return( _significantLabels ); }
    const std::set<std::string> & significantLabels() const
    { return( _significantLabels ); }
    const std::string & voidLabel() const
    { return( _voidLabel ); }
    void setVoidLabel( const std::string & s )
    { _voidLabel = s; }
    virtual carto::AttributedObject* parentAO() { return( _parentAO ); }
    virtual const carto::AttributedObject* parentAO() const
    { return( _parentAO ); }
    void setParentAO( carto::AttributedObject* newParent )
    { _parentAO = newParent; }
    void setMGraph( MGraph* mg ) { _mgraph = mg; }
    const MGraph* mGraph() const { return _mgraph; }
    MGraph* mGraph() { return _mgraph; }
    double weight() const { return( _weight ); }
    void setWeight( double w ) { _weight = w; }
    /** confidence factor generally used to weight the model output 
        (different from weight() which is another additional factor) */
    virtual double confidenceFactor() const { return 1.; }
    //@}

    /**@name	IO */
    //@{
    virtual void buildTree( Tree & tr ) const;
    //@}

  protected:
    /**	Label par défaut, qu'on attribue à tous les noeuds de label non 
	reconnus */
    std::string		_voidLabel;
    ///	Liste des labels qui ont un sens pour le modèle
    std::set<std::string>	_significantLabels;
    carto::AttributedObject*	_parentAO;
    ///	Poids de l'élément dans le recuit
    double		_weight;
    MGraph               *_mgraph;
    //@}

  private:
  };


  //	inline 

  inline TopModel::TopModel( carto::AttributedObject* parent ) 
    : _parentAO( parent ), _weight( 1. ), _mgraph( 0 )
  {
  }

}

#endif


