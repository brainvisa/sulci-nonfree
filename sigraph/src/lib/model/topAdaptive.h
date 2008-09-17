
#ifndef SI_MODEL_TOPADAPTIVE_H
#define SI_MODEL_TOPADAPTIVE_H


#include <si/model/adaptive.h>
#include <si/model/topModel.h>
#include <iostream>


namespace sigraph
{
  class Learner;

  class TopAdaptive : public Adaptive, public TopModel
  {
  public:
    TopAdaptive( Model* submod = 0 );
    TopAdaptive( const TopAdaptive & a );
    virtual ~TopAdaptive();
    virtual Model* clone() const;

    TopAdaptive & operator = ( const TopAdaptive & m );

    /**@name	Accès au modèle sous-jacent */
    //@{
    virtual Model* model() { return( _model ); }
    virtual void setModel( Model* subm ) { _model = subm; }

    ///	Propagation (réponse du modèle)
    virtual double prop( const Clique* );
    virtual double prop( const Clique* cl, 
			 const std::map<Vertex*, std::string> & changes );
    virtual bool doesOutputChange( const Clique* cl, 
                                   const std::map<Vertex*,
                                   std::string> & changes ) const;
    ///	Ouvre le(s) fichier(s) de sauvegardes des données reçues
    virtual bool openFile( const std::string & basename = "" );
    ///	Ferme le(s) fichier(s) de sauvegardes des données reçues
    virtual void closeFile();
    ///	Donne la liste des fichiers sous le modèle
    virtual void subFiles( const std::string & prefix, 
			   std::set<std::string> & listNames ) const;
    ///	Calcule le potentiel et garde le(s) vecteur(s) de description
    virtual double printDescription( Clique* cl, bool = false );
    ///	Donne la base de l'arborescence (TopModel, s'il y en a)
    virtual TopModel* topModel();
    virtual const TopModel* topModel() const;
    //@}

    /**@name	Accès à l'Adaptive */
    //@{
    /// Apprentissage et test
    virtual void generateDataBase(Learner &learner, const std::string &prefix,
		const std::list<Clique *> *lrnCliques,
		const std::list<Clique *> *tstCliques,
		int cycles, int cycles_tst);
    virtual double eval( const Clique *);
    //virtual void getVectors(LearnConstParam *lp);
    virtual double errorRate() const;
    virtual double genErrorRate() const;
    virtual double relianceWeight() const;
    virtual void init();
    virtual bool checkLearnFinished();
    /**	fait ce qu'il faut pour "fermer" l'apprentissage (remet le dernier 
	mémo...) */
    virtual void forceLearnFinished();

    virtual void resetStats();
    virtual void trainStats(Learner &learner,
			const std::list<Clique *> &cliques);
    //@}

    /**@name	IO */
    //@{
    ///	(devrait être externe à la classe...)
    virtual void buildTree( Tree & tr ) const;
    virtual void setBaseName( const std::string & basename );
    //@}

  protected:
    Model		*_model;

  private:
  };

  //	Fonctions inline

  inline Model* TopAdaptive::clone() const
  {
    return( new TopAdaptive( *this ) );
  }


  inline double TopAdaptive::relianceWeight() const
  {
    if( _model->isAdaptive() )
      return static_cast<const Adaptive *>(_model)->relianceWeight();
    return 1.;
  }


  inline double TopAdaptive::prop( const Clique* cl )
  {
    return( _model->prop( cl ) );
  }


  inline double TopAdaptive::printDescription( Clique* cl, bool naming )
  {
    return( _model->printDescription( cl, naming ) );
  }


  inline bool TopAdaptive::openFile( const std::string & basename )
  {
    return( _model->openFile( basename ) );
  }


  inline void TopAdaptive::closeFile()
  {
    _model->closeFile();
  }


  inline void TopAdaptive::subFiles( const std::string & prefix, 
				     std::set<std::string> & listNames ) const
  {
    _model->subFiles( prefix, listNames );
  }


inline void	TopAdaptive::generateDataBase(Learner &learner,
			const std::string &prefix,
			const std::list<Clique *> *lrnCliques,
			const std::list<Clique *> *tstCliques,
			int cycles, int cycles_tst)
{
	if (_model->isAdaptive())
		((Adaptive *) _model)->generateDataBase(learner, prefix,
			lrnCliques, tstCliques, cycles, cycles_tst);
}

  inline double TopAdaptive::eval( const Clique * cl )
  {
    if( _model->isAdaptive() )
      return( ((Adaptive *)_model)->eval( cl ) );
    else return( 0. );
  }


  inline double TopAdaptive::errorRate() const
  {
    if( _model->isAdaptive() )
      return( ((Adaptive *)_model)->errorRate() );
    else return( 0. );
  }


  inline double TopAdaptive::genErrorRate() const
  {
    if( _model->isAdaptive() )
      return( ((Adaptive *)_model)->genErrorRate() );
    else return( 0. );
  }


  inline void TopAdaptive::init()
  {
    if( _model->isAdaptive() )
      ((Adaptive *)_model)->init();
    _ndata = 0;
  }


  inline void TopAdaptive::resetStats()
  {
    if( _model->isAdaptive() )
      ((Adaptive *)_model)->resetStats();
  }


  inline void TopAdaptive::trainStats(Learner &learner,
			const std::list<Clique *> &cliques)
  {
    if( _model->isAdaptive() )
      ((Adaptive *)_model)->trainStats(learner, cliques);
  }


  inline TopModel* TopAdaptive::topModel()
  {
    return( this );
  }


  inline const TopModel* TopAdaptive::topModel() const
  {
    return( this );
  }

  inline void TopAdaptive::setBaseName( const std::string & basename )
  {
    _model->setBaseName( basename );
  }


  inline bool TopAdaptive::checkLearnFinished()
  {
    if( !_learnfinished )
      {
	if( _model->isAdaptive() )
	  _learnfinished = ((Adaptive *)_model)->checkLearnFinished();
	else
	  _learnfinished = true;
      }
    return( _learnfinished );
  }


  inline void TopAdaptive::forceLearnFinished()
  {
    if( !_learnfinished && _model->isAdaptive() )
      ((Adaptive *)_model)->forceLearnFinished();
    _learnfinished = true;
  }

}

#endif

