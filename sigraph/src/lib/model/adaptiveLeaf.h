

#ifndef SI_MODEL_ADAPTIVELEAF_H
#define SI_MODEL_ADAPTIVELEAF_H


#include <si/model/adaptive.h>
#include <si/model/learnParam.h>
#include <si/graph/clique.h>
#include <si/subadaptive/incrementalSubAdaptive.h>
#include <si/subadaptive/subAdaptive.h>
#include <si/learnable/vectorLearnable.h>
#include <si/descr/adapDescr.h>
#include <si/dimreductor/dimreductor.h>
#include <si/optimizer/optimizer.h>
#include <si/graph/attrib.h>
#include <si/graph/mgraph.h>


namespace sigraph
{
  class Learner;

  /**	Elément adaptatif terminal.
   */
  class AdaptiveLeaf : public Adaptive
  {
  public:
    ///	Etat d'apprentissage
    enum State
      {
	LEARNING, 
	STOPPABLE, 
	STOPPED
      };

    AdaptiveLeaf( const CliqueDescr * cd = 0, const SubAdaptive *work = 0, 
		  const SubAdaptive *eval = 0 );
    AdaptiveLeaf( const AdaptiveLeaf & ad );
    virtual ~AdaptiveLeaf();
    virtual Model* clone() const;

    /**@name	Opérateurs */
    //@{
    AdaptiveLeaf & operator = ( const AdaptiveLeaf & ad );
    //@}

    /**@name	Fonctions de base */
    //@{
    /// Récupère une liste de cliques
    virtual void getVectors(AdapDescr &ad, Learner &learner,
				const std::list<Clique *> &cliques,
				int cycle, int &cur_cycle);
    virtual void generateDataBase(Learner &learner, const std::string &prefix,
		const std::list<Clique *> *lrnCliques,
		const std::list<Clique *> *tstCliques,
		int cycles, int cycles_tst);

    virtual std::string	getDataBaseName(const std::string &prefix);

    ///	Propagation (réponse de l'Adaptive)
    virtual double prop( const Clique * );
    /* virtual double prop( const Clique* cl,
			 const std::map<Vertex*, std::string> & changes ); */
    virtual bool doesOutputChange( const Clique* cl, 
                                   const std::map<Vertex*,
                                   std::string> & changes ) const;
    ///	Evaluation (confiance dans la réponse au point donné)
    virtual double eval( const Clique * );
    ///
    virtual void init();
    ///	Ouvre le(s) fichier(s) de sauvegardes des données reçues
    virtual bool openFile( const std::string & basename = "" );
    ///	Ferme le(s) fichier(s) de sauvegardes des données reçues
    virtual void closeFile();
    ///	Donne la liste des fichiers sous le modèle
    virtual void subFiles( const std::string & prefix, 
			   std::set<std::string> & listNames ) const;
    ///	Taux d'erreur d'apprentissage global
    virtual double errorRate() const { return( _work->errorRate() ); }
    ///	Taux d'erreur de généralisation
    virtual double genErrorRate() const 
    { return( _work->genMeanErrorRate() ); }
    virtual double relianceWeight() const
    { return _work->relianceWeight(); }
    ///	Calcule le potentiel et garde le(s) vecteur(s) de description
    virtual double printDescription( Clique* cl, bool withnames );
    virtual bool checkLearnFinished();
    /**	fait ce qu'il faut pour "fermer" l'apprentissage (remet le dernier 
	mémo...) */
    virtual void forceLearnFinished();
    //@}

    /**@name	Statistiques (pour normaliser les entrées) */
    //@{
    ///	Vide toutes les stats
    virtual void resetStats();
    ///	Ajout à la base de statistiques
    virtual void trainStats(Learner &learner,
			const std::list<Clique *> &cliques);
    /// center and reduce vector according to _mean and _std
    virtual void centerAndReduce(std::vector<double> &vec);
    //@}

    /**@name	Accès aux données */
    //@{
    /// Renvoit le descripteur adaptatif du modèle s'il existe ou NULL
    AdapDescr *getAdapDescr(void);
    ///	Accés à l'élément efficace (qui donne la sortie utile)
    SubAdaptive & workEl() const { return( *_work ); }
    /// Return Dimension Reductor element definition
    const DimReductor* dimreductor() const { return _dimreductor; }
    /// Return Optimizer definition (parmeters, ranges, scale)
    const Optimizer & optimizer() const { return *_optimizer; }
    /// Set work element (no copy), old one is deleted if needed.
    inline void setWorkEl(SubAdaptive *work)
    {
	if (_work) delete _work;
        _work = work;
    }

    /// Get database mean vector
    const std::vector<float> &getMean(void) const { return _mean; };
    /// Get database std vector
    const std::vector<float> &getStd(void) const { return _std; };
    void setDimReductor( DimReductor* dimreductor );
    void setOptimizer( Optimizer* opt);
    SubAdaptive *workMemoEl() const { return _workMemo; }
    ///	Accès à l'élément d'évaluation
    SubAdaptive & evalEl() { return( *_eval ); }
    ///	Accès à l'élément d'évaluation, version const
    const SubAdaptive & evalEl() const { return( *_eval ); }
    ///	Remplacement de l'élément efficace
    void setWork( const SubAdaptive & w );
    ///	Remplacament de l'élément d'évaluation
    void setEval( const SubAdaptive & e );
    ///	Remplacement de l'élément efficace mémorisé
    void setWorkMemo( const SubAdaptive & w );
    ///	Remplacament de l'élément d'évaluation mémorisé
    void setEvalMemo( const SubAdaptive & e );
    const CliqueDescr & cliqueDescr() const { return( *_cliqueDescr ); }
    CliqueDescr & cliqueDescr() { return( *_cliqueDescr ); }
    void setCliqueDescr( CliqueDescr* cd );
    /// Set database mean vector
    void setMean(const std::vector<float> &mean) { _mean = mean; };
    /// Set database standard deviation vector
    void setStd(const std::vector<float> &std) { _std = std; };
    //@}

    ///	Conversion en arbre (pour IO)
    void buildTree( Tree & tr ) const;
    virtual void setBaseName( const std::string & basename );

    ///	Mémorisation des modèles
    virtual void memorize();
    ///	Retour aux modèles mémorisés
    virtual void revert();
    State learnState() const { return( _lrnState ); }
    void setLearnState( State s );
    unsigned nDataMemo() const { return( _ndataMemo ); }
    void setNDataMemo( unsigned n ) { _ndataMemo = n; }
    /** Update internal values (including work element, learning state) from
        an other adaptiveleaf. */
    void update(sigraph::AdaptiveLeaf &ad);

  protected:
    ///	Elément efficace
    SubAdaptive		*_work;
    ///	Elément d'évaluation
    SubAdaptive		*_eval;
    ///	Descripteur de clique
    CliqueDescr		*_cliqueDescr;
    ///	Etat de l'apprentissage
    State		_lrnState;
    ///	Memorisation des SubAdap
    SubAdaptive		*_workMemo;
    SubAdaptive		*_evalMemo;
    ///	Mémorisation du nombre de données apprises
    unsigned		_ndataMemo;
    /// dimension reduction element
    DimReductor		*_dimreductor;
    /// optimizer element
    Optimizer		*_optimizer;
    /// mean of database vectors
    std::vector<float>	_mean;
    /// standard deviation of database vectors
    std::vector<float>	_std;

  private:

  };


  //	Fonctions

  inline AdaptiveLeaf::AdaptiveLeaf( const AdaptiveLeaf & ad ) 
    : Adaptive( ad ), _work( ad._work->clone() ), 
      _eval( ad._eval ? ad._eval->clone() : 0 ), 
      _cliqueDescr( ad._cliqueDescr ? ad._cliqueDescr->clone() : 0 ), 
      _lrnState( ad._lrnState ), 
      _workMemo( ad._workMemo ? ad._workMemo->clone() : 0 ), 
      _evalMemo( ad._evalMemo ? ad._evalMemo->clone() : 0 ), 
      _ndataMemo( ad._ndataMemo ),
      _dimreductor(ad._dimreductor ? ad._dimreductor->clone() : 0),
      _optimizer(ad._optimizer ? ad._optimizer->clone() : 0)
  {
  }


  inline Model* AdaptiveLeaf::clone() const
  {
    return( new AdaptiveLeaf( *this ) );
  }


  inline AdaptiveLeaf & AdaptiveLeaf::operator = ( const AdaptiveLeaf & ad )
  {
    if( this != & ad )
      {
	delete _work;
	delete _eval;
	delete _cliqueDescr;
	_work = ad._work->clone();
	_eval = ( ad._eval ? ad._eval->clone() : 0 );
	_cliqueDescr = ad._cliqueDescr->clone();
	_lrnState = ad._lrnState;
	_workMemo = ( ad._workMemo ? ad._workMemo->clone() : 0 );
	_evalMemo = ( ad._evalMemo ? ad._evalMemo->clone() : 0 );
	_dimreductor = (ad._dimreductor ? ad._dimreductor->clone() : 0);
	_optimizer = (ad._optimizer ? ad._optimizer->clone() : 0);
	_ndataMemo = ad._ndataMemo;
      }
    return( *this );
  }


  inline double AdaptiveLeaf::prop( const Clique* cl )
  {
    std::vector<double>		vec;
    carto::AttributedObject	*ao = graphObject();
    double			w, p = 0;
    Graph			*g = NULL;
    int				ng;
    bool			true_vector;
    TopModel			*tm = topModel();

    if( tm )
      w = tm->weight();
    else w = 1.;

    true_vector = _cliqueDescr->makeVector( cl, vec, ao );
    if (!true_vector)
    {
        g = tm->mGraph();
	assert(g->getProperty( SIA_NBASEGRAPHS, ng));
	assert(ng);
	ao->getProperty(SIA_NOINSTANCE_COUNT, p);

	double        f = 1. - double(p)/ng;
	double        thld = 0.85;

	return (0.5 / ( 1 + exp( -(f - thld) * 40 ) )) * w *
					_work->relianceWeight();
    }
    _cliqueDescr->preProcess( vec, ao );
    std::vector<double> *redvec;
    std::vector<double> rvec;
    if( dimreductor() )
    {
      //FIXME : pour que le code sans dimreduction marche toujours
      // l'application des stats n'est active que dans ce mode
      centerAndReduce(vec);
      rvec.reserve( dimreductor()->reducedDim() );
      rvec.insert( rvec.end(), dimreductor()->reducedDim(), 0 );
      dimreductor()->transform( vec, rvec );
      redvec = &rvec;
    }
    else
      redvec = &vec;
    p = _work->prop( *redvec );
    return p * w * _work->relianceWeight();
  }

  inline double AdaptiveLeaf::eval( const Clique* cl )
  {
    std::vector<double>		vec;
    carto::AttributedObject	*ao = graphObject();

    if( _eval )
      {
        _cliqueDescr->makeVector( cl, vec, ao );
        _cliqueDescr->preProcess( vec, ao );
        std::vector<double> *redvec;
        std::vector<double> rvec;
        if( dimreductor() )
        {
          //FIXME : pour que le code sans dimreduction marche toujours
	  // l'application des stats n'est active que dans ce mode
          centerAndReduce(vec);
          rvec.reserve( dimreductor()->reducedDim() );
          rvec.insert( rvec.end(), dimreductor()->reducedDim(), 0 );
          dimreductor()->transform( vec, rvec );
          redvec = &rvec;
        }
        else
          redvec = &vec;
        double x = _eval->prop( *redvec );
        return x;
      }
    else
      return errorRate();
  }


  inline void AdaptiveLeaf::setWork( const SubAdaptive & w )
  {
    delete _work;
    _work = w.clone();
  }


  inline void AdaptiveLeaf::setEval( const SubAdaptive & e )
  {
    delete _eval;
    _eval = e.clone();
  }


  inline void AdaptiveLeaf::setWorkMemo( const SubAdaptive & w )
  {
    delete _workMemo;
    _workMemo = w.clone();
  }


  inline void AdaptiveLeaf::setEvalMemo( const SubAdaptive & e )
  {
    delete _evalMemo;
    _evalMemo = e.clone();
  }


  inline void AdaptiveLeaf::setCliqueDescr( CliqueDescr* cd )
  {
    delete _cliqueDescr;
    _cliqueDescr = cd;
  }

  inline void AdaptiveLeaf::setDimReductor(DimReductor *dimreductor)
  {
    delete _dimreductor;
    _dimreductor = dimreductor;
  }

  inline void AdaptiveLeaf::setOptimizer(Optimizer *optimizer)
  {
    delete _optimizer;
    _optimizer = optimizer;
  }

}


#endif


