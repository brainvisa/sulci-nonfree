
#ifndef SI_LEARNER_TRAINERITERATOR_H
#define SI_LEARNER_TRAINERITERATOR_H

#include <set>
#include <map>
#include <list>
#include <cartobase/object/object.h>

namespace sigraph
{
  class Trainer;
  class CGraph;
  class Model;
  class Clique;
  class Adaptive;

  class TrainerIterator
  {
  public:
    typedef std::map<Model *, std::list<Clique *> > CliquesModelMap;

    TrainerIterator( Trainer &, const std::set<CGraph *> *lrnBase,
                     const std::set<CGraph *> *tstBase = 0, int cycles = 1, 
                     int cycles_tst = 1 );
    TrainerIterator( const TrainerIterator & );
    ~TrainerIterator();

    TrainerIterator & operator = ( const TrainerIterator & );

    /// trains the Trainer for 1 cycle, so the iterator increments
    void train(carto::Object &o);
    void next();
    bool isValid() const;

    Model* model();
    /// same as model() but dynamic_casted to Adaptive *
    Adaptive* adaptive();
    CliquesModelMap* learnBase();
    CliquesModelMap* testBase();
    int cycles() const;
    int testCycles() const;

  private:
    friend class Trainer;
    struct Private;
    Private	*d;
  };

}

#endif


