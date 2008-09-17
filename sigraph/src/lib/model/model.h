
#ifndef SI_MODEL_MODEL_H
#define SI_MODEL_MODEL_H


#include <vector>
#include <string>
#include <set>
#include <map>

#include <si/model/topModel.h>
class Vertex;


namespace sigraph
{
  class Clique;

  /**	Mod�le pour l'�valuation de cliques de graphes (classe abstraite).


	Le mod�le sert � �valuer une description ou une configuration. Sa 
	fonction est de donner un potentiel � un vecteur d'entr�e.
  */
  class Model
  {
  public:
    virtual ~Model();
    ///	Duplication (fonction abstraite)
    virtual Model* clone() const = 0;

    Model & operator = ( const Model & m );

    /**@name	Fonctions de base */
    //@{
    /**	Propagation (r�ponse du mod�le)
	@param	cl	Clique dont on veut conna�tre le potentiel
	@return	potentiel de la clique
    */
    virtual double prop( const Clique* cl ) = 0;
    /**	Donne le potentiel d'une clique, apr�s une transformation de labels de 
	noeuds, en ne le recalculant que si les labels ayant chang� changent 
	effectivement le potentiel
	@param	cl	clique � d�crire
	@param	changes	liste des noeuds dont le label a chang�, avec pour 
	chacun le label d'origine (avant changement), le 
	changement doit d�j� �tre effectu�
	@return	potentiel
    */
    virtual double prop( const Clique* cl, 
			 const std::map<Vertex*, std::string> & changes );
    /** tells whether a label change inpacts the model output or not.
        The default implementation always returns true */
    virtual bool doesOutputChange( const Clique* cl, 
                                   const std::map<Vertex*,
                                   std::string> & changes ) const;
    /**	Met � jour les �tats internes du mod�le (au besoin)
	Cette fonction doit �tre appel�e apr�s chaque changement de label 
	@return	le nouveau potentiel de la clique */
    virtual double update( const Clique* cl ) { return( prop( cl ) ); }
    /**	Change les �tats internes pour les noeuds de la liste changes, chacun 
	avec son ancien nom (pour les changements par diff�rence).
        Par d�faut (si elle n'est pas surcharg�e), il rappelle simplement 
        update(const Clique*).
	@return	la diff�rence de potentiel entra�n�e par le changement */
    virtual double update( const Clique* cl, 
			   const std::map<Vertex*, std::string> & changes );
    /**	Le mod�le est-il adaptatif ?. \\
	Par d�faut, il ne l'est pas. Un mod�le adaptatif d�rive de la classe 
	Adaptive, dont la fonction isAdaptive retourne
	true. \\
	Si la fonction retourne true, on peut faire un cast vers
	une classe Adaptive.
    */
    virtual bool isAdaptive() const;
    ///	Ouvre le(s) fichier(s) de sauvegardes des donn�es re�ues
    virtual bool openFile( const std::string & basename = "" );
    ///	Ferme le(s) fichier(s) de sauvegardes des donn�es re�ues
    virtual void closeFile();
    ///	Donne la liste des fichiers sous le mod�le
    virtual void subFiles( const std::string & prefix, 
			   std::set<std::string> & listNames ) const;
    ///	Calcule le potentiel et garde le(s) vecteur(s) de description
    virtual double printDescription( Clique* cl, bool withnames = false );
    //@}

    /**@name	Gestion de l'arborescence (parents) */
    //@{
    ///	Donne le parent direct (0 si pas de parent)
    virtual Model* parent();
    ///	Fixe le parent
    virtual void setParent( Model* m );
    ///	Donne la base de l'arborescence (TopModel, s'il y en a)
    virtual TopModel* topModel();
    virtual const TopModel* topModel() const;
    ///	Donne l'�l�ment parent du graphe mod�le
    virtual carto::AttributedObject* graphObject();
    virtual const carto::AttributedObject* graphObject() const;
    //@}

    /**@name	IO */
    //@{
    ///	(devrait �tre externe � la classe...)
    virtual void buildTree( Tree & tr ) const = 0;
    virtual void setBaseName( const std::string & basename );
    //@}

  protected:
    Model( Model* parent = 0 );
    Model( const Model & m );

  private:
    ///	parent
    Model	*_parent;
  };


  //	Fonctions inline

  inline Model::Model( Model* parent ) : _parent( parent )
  {
  }


  inline Model::Model( const Model & ) : _parent( 0 )
  {
  }


  inline Model & Model::operator = ( const Model & )
  {
    return( *this );
  }


  inline bool Model::isAdaptive() const
  {
    return( false );
  }


  inline bool Model::openFile( const std::string & )
  {
    return( false );
  }


  inline void Model::closeFile()
  {
  }


  inline void Model::subFiles( const std::string &,
                               std::set<std::string> & ) const
  {
  }


  inline Model* Model::parent()
  {
    return( _parent );
  }


  inline void Model::setParent( Model* m )
  {
    _parent = m;
  }


  inline TopModel* Model::topModel()
  {
    return( _parent ? _parent->topModel() : 0 );
  }


  inline const TopModel* Model::topModel() const
  {
    return( _parent ? _parent->topModel() : 0 );
  }


  inline carto::AttributedObject* Model::graphObject()
  {
    TopModel* tm = topModel();
    return tm ? tm->parentAO() : 0;
  }


  inline const carto::AttributedObject* Model::graphObject() const
  {
    const TopModel* tm = topModel();
    return tm ? tm->parentAO() : 0;
  }


  inline void Model::setBaseName( const std::string & )
  {
  }


  inline double Model::update( const Clique* cl, 
			       const std::map<Vertex*, std::string> & )
  {
    return update( cl );
  }

}


namespace carto
{
  DECLARE_GENERIC_OBJECT_TYPE( sigraph::Model * )
}

#endif

