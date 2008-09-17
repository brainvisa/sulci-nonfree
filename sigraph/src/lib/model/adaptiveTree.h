

#ifndef SI_MODEL_ADAPTIVETREE_H
#define SI_MODEL_ADAPTIVETREE_H


#include <si/model/adaptive.h>
#include <si/mixer/mixer.h>
#include <set>


namespace sigraph
{

  /**	El�ment adaptatif multiple.
   */
  class AdaptiveTree : public Adaptive
  {
  public:
    typedef std::set<Adaptive *>		datatype;
    typedef datatype::iterator	iterator;
    typedef datatype::const_iterator	const_iterator;

    AdaptiveTree( const Mixer & mix );
    AdaptiveTree( const std::string mix_method = "" );
    AdaptiveTree( const AdaptiveTree & ad );
    virtual ~AdaptiveTree();
    virtual Model* clone() const;

    AdaptiveTree & operator = ( const AdaptiveTree & ad );

    /// Apprentissage et test
    virtual void generateDataBase(Learner &learner, const std::string &prefix,
		const std::list<Clique *> *lrnCliques,
		const std::list<Clique *> *tstCliques,
		int cycles, int cycles_tst);
    ///	Propagation (r�ponse de l'Adaptive)
    virtual double prop( const Clique * );
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
    virtual bool doesOutputChange( const Clique* cl, 
                                   const std::map<Vertex*,
                                   std::string> & changes ) const;
    ///	Evaluation (confiance dans la r�ponse au point donn�)
    virtual double eval( const Clique * );
    virtual void init();
    ///	Ouvre le(s) fichier(s) de sauvegardes des donn�es re�ues
    virtual bool openFile( const std::string & basename = "" );
    ///	Ferme le(s) fichier(s) de sauvegardes des donn�es re�ues
    virtual void closeFile();
    ///	Donne la liste des fichiers sous le mod�le
    virtual void subFiles( const std::string & prefix, 
			   std::set<std::string> & listNames ) const;
    ///	Taux d'erreur d'apprentissage global
    virtual double errorRate() const;
    ///	Taux d'erreur de g�n�ralisation
    virtual double genErrorRate() const;
    virtual double relianceWeight() const;
    ///	Calcule le potentiel et garde le(s) vecteur(s) de description
    virtual double printDescription( Clique* cl, bool = false );
    virtual bool checkLearnFinished();
    /**	fait ce qu'il faut pour "fermer" l'apprentissage (remet le dernier 
	m�mo...) */
    virtual void forceLearnFinished();

    /**@name	Statistiques (pour normaliser les entr�es) */
    //@{
    ///	Vide toutes les stats
    virtual void resetStats();
    ///	Ajout � la base de statistiques
    virtual void trainStats(Learner &learner,
			const std::list<Clique *> &cliques);
    //@}

    /**@name	Acc�s aux donn�es */
    //@{
    const datatype & children() const { return( _children ); }
    std::pair<iterator, bool> insert( Adaptive *child );
    void erase( Adaptive *child );
    void erase( const iterator & i1 );
    void erase( const iterator & i1, const iterator & i2 );
    void insert( const datatype & children );
    const_iterator begin() const { return( _children.begin() ); }
    const_iterator end() const { return( _children.end() ); }
    iterator begin() { return( _children.begin() ); }
    iterator end() { return( _children.end() ); }
    const Mixer & mixer() const { return ( *_mixer ); }
    void setMixer( Mixer* mix );
    //@}

    ///	Conversion en arbre (pour IO)
    void buildTree( Tree & tr ) const;
    virtual void setBaseName( const std::string & basename );

  protected:
    datatype	_children;
    Mixer		*_mixer;

  private:
  };


  //	Fonctions

  inline AdaptiveTree::AdaptiveTree( const Mixer & mix )
    : Adaptive(), _mixer( mix.clone() )
  {
  }


  inline AdaptiveTree::AdaptiveTree( const AdaptiveTree & ad )
    : Adaptive( ad ), _mixer( ad._mixer->clone() )
  {
    const_iterator	is, fs=ad.end();
    for( is=ad.begin(); is!=fs; ++is )
      insert( (Adaptive* ) (*is)->clone() );
  }


  inline Model* AdaptiveTree::clone() const
  {
    return( new AdaptiveTree( *this ) );
  }


  inline AdaptiveTree & AdaptiveTree::operator = ( const AdaptiveTree & ad )
  {
    if( this != & ad )
      {
	(*(Adaptive *) this) = (const Adaptive &) ad;
	erase( begin(), end() );
	const_iterator	is, fs=ad.end();
	for( is=ad.begin(); is!=fs; ++is )
	  insert( (Adaptive *) (*is)->clone() );
	if( ad._mixer ) _mixer = ad._mixer->clone();
	else _mixer = 0;
      }
    return( *this );
  }


  inline std::pair<AdaptiveTree::iterator, bool> 
  AdaptiveTree::insert( Adaptive *child )
  {
    child->setParent( this );
    return( _children.insert( child ) );
  }


  inline void AdaptiveTree::insert( const AdaptiveTree::datatype & children )
  {
    const_iterator	ic, fc=children.end();
    for( ic=children.begin(); ic!=fc; ++ic )
      {
	_children.insert( *ic );
	(*ic)->setParent( this );
      }
  }


  inline void AdaptiveTree::erase( Adaptive *child )
  {
    _children.erase( child );
    child->setParent( 0 );
  }


  inline void AdaptiveTree::erase( const AdaptiveTree::iterator & ic )
  {
    _children.erase( ic );
    (*ic)->setParent( 0 );
  }


  inline void AdaptiveTree::erase( const AdaptiveTree::iterator & ic, 
				   const AdaptiveTree::iterator & fc )
  {
    AdaptiveTree::iterator	i;

    for( i=ic; i!=fc; ++i )
      (*i)->setParent( 0 );
    _children.erase( ic, fc );
  }


  inline void AdaptiveTree::setMixer( Mixer* mix )
  {
    delete _mixer;
    _mixer = mix;
  }

}


#endif

