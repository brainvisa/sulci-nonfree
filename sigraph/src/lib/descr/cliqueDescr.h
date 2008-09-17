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

#ifndef SI_DESCR_CLIQUEDESCR_H
#define SI_DESCR_CLIQUEDESCR_H


#include <cartobase/object/object.h>
#include <vector>

class Tree;
class Vertex;


namespace sigraph
{
  class Clique;

  /**	Descripteur de clique (classe abstraite).


	Interface entre Clique et Model.

	Cet élément a pour fonction de décrire des cliques sous forme de 
	vecteur, par la fonction CliqueDescr::makeVector.
  */
  class CliqueDescr
  {
  public:
    virtual ~CliqueDescr();
    virtual CliqueDescr* clone() const = 0;

    /**	Fabrique le vecteur d'entrées. 
	@param	cl	clique à décrire
	@param	vec	(RETOUR) vecteur description.
	@param	model	élément de modèle parent (objet du graphe MGraph)
	@return		true si OK, false si qqchose s'est mal
	passé
    */
    virtual bool makeVector( const Clique* cl, std::vector<double> & vec, 
			     carto::GenericObject* model = 0 );
    /**	Effectue des pré-processings (si nécessaire). 
	Fonction appelée par makeVector() */
    virtual void preProcess( std::vector<double> & vec, 
			     carto::GenericObject* model = 0 );
    /**	Dit si le potentiel d'une clique, après une transformation de labels 
	de noeuds, a pu changer, ou si les changements n'ont pas d'influence 
	sur le potentiel.
	@param	cl	clique à décrire
	@param	changes	liste des noeuds dont le label a changé, avec pour 
	chacun le label d'origine (avant changement), le 
	changement doit déjà être effectué
	@param	model	élément de modèle parent (objet du graphe MGraph)
	@return	true si le potentiel a besoin d'être recalculé
    */
    virtual bool hasChanged( const Clique* cl, 
			     const std::map<Vertex*, std::string> & changes, 
			     const carto::GenericObject* model = 0 ) const;

    virtual void buildTree( Tree & ) {}
    virtual std::vector<std::string> descriptorsNames() const;
    virtual std::string name() const;

  protected:
    CliqueDescr();
    virtual bool makeVectorElements( const Clique* cl, 
                                     std::vector<double> & vec, 
                                     carto::GenericObject* model = 0 ) = 0;

  private:
  };

  //	inline

  inline void CliqueDescr::preProcess( std::vector<double> &, 
				       carto::GenericObject* )
  {
  }


  inline bool CliqueDescr::hasChanged( const Clique*, 
				       const std::map<Vertex*, std::string> &, 
				       const carto::GenericObject* ) const
  {
    return( true );	// par défaut, toujours
  }

}


namespace carto
{
  DECLARE_GENERIC_OBJECT_TYPE( sigraph::CliqueDescr * )
}

#endif

