
#ifndef SI_LEARNER_TRAINER_H
#define SI_LEARNER_TRAINER_H

#include <si/graph/cgraph.h>
#include <si/graph/mgraph.h>
#include <si/learner/learner.h>
#include <si/learner/traineriterator.h>
#include <si/learnable/learnable.h>


namespace sigraph
{

  /**	Apprentissage des éléments adaptatifs des graphes modèles
   */
  class Trainer
  {
  public:
    enum TrainerMode
      {
        GenerateOnly,
        GenerateAndTrain,
        ReadAndTrain,
        TrainDomain,
        TrainStats,
      };

  protected:
    typedef TrainerIterator::CliquesModelMap CliquesModelMap;

  public:
    typedef TrainerIterator iterator;

    Trainer(MGraph &mg, Learner *learner = NULL);
    virtual ~Trainer();

    virtual
    CliquesModelMap *dataBaseToCliquesModelMap(const std::set<CGraph *> &lrn);
    virtual inline void setMode(TrainerMode mode) { _mode = mode; }
    inline TrainerMode mode() const { return _mode; }
    virtual void init(TrainerMode mode, unsigned pass = 0);
    virtual void resetDomains();
    iterator trainIterator( const std::set<CGraph *> *lrnBase,
                            const std::set<CGraph *> *tstBase = 0, 
                            int c = 1, int ct = 1 );
    /// performs training of one Model. 
    virtual void trainOne( iterator & i, carto::Object &o);
    virtual void train( const std::set<CGraph *> *lrnBase,
			const std::set<CGraph *> *tstBase = NULL,
			int cycles = 1, int cycles_tst = 1 );
    void trainDomain( iterator & i );
    void trainStats( iterator & i );
    void generateDataBase(iterator & i, const std::string &prefix);
    const MGraph &getGraphModel() const { return _mgraph; };

  protected:
    friend class sigraph::TrainerIterator;
    friend struct sigraph::TrainerIterator::Private;

    virtual std::set<Model *> 
    *modelsFromCliquesModelMap(const CliquesModelMap *cllrn, 
                                const CliquesModelMap *cltst);

    MGraph	&_mgraph;
    Learner	*_learner;
    unsigned	_pass;
    bool	_learnfinished;
    TrainerMode	_mode;
  };

}

#endif

