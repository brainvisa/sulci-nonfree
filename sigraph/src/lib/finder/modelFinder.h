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

#ifndef SI_FINDER_MODELFINDER_H
#define SI_FINDER_MODELFINDER_H


#include <si/graph/clique.h>
#include <si/graph/mgraph.h>
#include <si/model/model.h>


namespace aims
{
  class SelectionSet;
}


namespace sigraph
{
  class CGraph;

  /**	Chercheur de mod�le (classe abstraite).


	Interface entre MGraph / CGraph et Model.

	Cet �l�ment a pour fonction de trouver l'�l�ment Model du 
	graphe mod�le en fonction de la clique. Cela consiste en:

	- choisir un mod�le dans le graphe mod�le en fonction de la 
	  clique: fonction ModelFinder::selectModel, � d�finir.
	- calculer le potentiel de la clique en utilisant le mod�le: 
	  fonction ModelFinder::potential. Normalement on n'a pas besoin de 
	  red�finir cette fonction, qui utilise makeVector(). C'est en 
	  principe cette fonction qui sera utilis�e dans l'interface avec 
	  les graphes.
	- une variante de la fonction ModelFinder::potential prend en 
	  param�tre suppl�mentaire une liste (de type \c map, en fait) de 
	  noeuds avec chacun une �tiquette associ�e. Cette liste contient les 
	  noeuds dont l'�tiquette a chang� depuis la derni�re �valuation 
	  du potentiel de la clique, avec l'�tiquette d'origine de chaque 
	  noeud. Cette fonction est utilis�e en particulier au cours du 
	  recuit simul�, et est destin�e � optimiser le calcul du 
	  potentiel dans le cas particulier d'�valuations successives avec 
	  des �tiquettes diff�rentes, soit en l'effectuant par diff�rences 
	  si cela est possible, soit en ne le recalculant pas lorsque ce 
	  n'est pas n�cessaire (si les changements n'influent pas sur le 
	  potentiel). Dans ce dernier cas, il suffit de renvoyer le 
	  potentiel pr�c�dent, qui est normalement stock� dans l'attribut 
	  \c "potential". Par d�faut, cette fonction ne tient pas 
	  compte des changements qui lui sont pr�cis�s et appelle le 
	  calcul complet du potentiel.
  */
  class ModelFinder
  {
  public:
    virtual ~ModelFinder();

    const MGraph & mGraph() const { return( _mgraph ); }
    ///	Choisit le noeud mod�le dans le MGraph en fonction de la clique
    virtual carto::AttributedObject* selectModel( const Clique* cl ) = 0;
    /**	Donne le potentiel d'une clique
	@param	cl	clique � d�crire
	@return		potentiel de la clique
    */
    virtual double potential( const Clique *cl );
    /**	Donne le potentiel d'une clique, apr�s une transformation de labels de 
	noeuds, en ne le recalculant que si les labels ayant chang� changent 
	effectivement le potentiel
	@param	cl	clique � d�crire
	@param	changes	liste des noeuds dont le label a chang�, avec pour 
	chacun le label d'origine (avant changement), le 
	changement doit d�j� �tre effectu�
	@return	potentiel
    */
    virtual double potential( const Clique* cl, 
			      std::map<Vertex*, std::string> & changes );
    /**	Fait un update() sur le mod�le sous-jacent
	@return	diff�rence de potentiel */
    virtual double update( const Clique* cl );
    /**	Fait un update() sur le mod�le sous-jacent
	@return	diff�rence de potentiel */
    virtual double update( const Clique* cl, 
			   std::map<Vertex*, std::string> & changes );
    ///	peut apprendre ?
    virtual bool isAdaptive() const;
    ///	nettoie les variables & caches �ventuels
    virtual void clear() {}
    /**	Calcule le potentiel et garde le(s) vecteur(s) de description de la 
	clique. Les vecteurs sont mis en attribut dans la clique, sous les 
	attributs "pot_vector" et "pot_vector_norm" (avant et apr�s 
	normalisation)
    */
    virtual double printDescription( Clique *cl, bool naming = false );

    /** Initialize cliques of data graph using the current model.
        The default implementation only makes cliques of nodes with 
        the same label, regardless of the model (only useful for 
        morphometry-like applications).
        - If a selection is provided, it is used to translate labels.
        - The \c translateLabels option is only used if no selection is 
        provided. In this case it decides whether to make a selection 
        "identity" (with no labels translation) or to use the model and 
        a labels hierarchy (\see LabelsTranslator) to select/group labels. 
        To make it usable, the model has to provide elements with labels so 
        the default implementation doesn't take care of it, it will only be 
        useful for specialized ModelFinders (\see FoldFinder)
    */
    virtual void initCliques( CGraph & data, 
                              bool verbose = true, 
                              bool withCache = false, 
                              bool translateLabels = true, 
                              bool checkLabels = true, 
                              const aims::SelectionSet *sel = 0 );

  protected:
    MGraph	&_mgraph;

    ModelFinder( MGraph & rg );

  private:
  };



  //	inline


  inline bool ModelFinder::isAdaptive() const
  {
    return( false );
  }

}


namespace carto
{
  DECLARE_GENERIC_OBJECT_TYPE( sigraph::ModelFinder * )
}

#endif

