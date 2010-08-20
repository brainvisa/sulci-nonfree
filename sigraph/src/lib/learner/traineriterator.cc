
#include <si/learner/trainer.h>
#include <si/model/adaptive.h>
#include <cartobase/smart/rcptr.h>

using namespace sigraph;
using namespace carto;
using namespace std;


struct TrainerIterator::Private
{
  Private( Trainer & t, const std::set<CGraph *> *lrnBase,
           const std::set<CGraph *> *tstBase, int cycles, int cycles_tst );
  Private( const Private & );
  ~Private();

  Private & operator = ( const Private & );

  Trainer			*trainer;
  rc_ptr<CliquesModelMap>	cmlrn;
  rc_ptr<CliquesModelMap>	cmtst;
  rc_ptr<set<Model *> >	models;
  int				cycles;
  int				cycles_tst;
  set<Model *>::iterator	miter;
};


TrainerIterator::Private::Private( Trainer & t, 
                                   const std::set<CGraph *> *lrnBase,
                                   const std::set<CGraph *> *tstBase, 
                                   int c, int ct )
  : trainer( &t ), cmtst( 0 ), cycles( c ), cycles_tst( tstBase ? ct : 0 )
{
  cmlrn.reset( t.dataBaseToCliquesModelMap( *lrnBase ) );
  if (tstBase && !tstBase->empty())
    cmtst.reset( t.dataBaseToCliquesModelMap( *tstBase ) );
  models.reset( t.modelsFromCliquesModelMap( cmlrn.get(), cmtst.get() ) );
  miter = models->begin();
}


TrainerIterator::Private::Private( const TrainerIterator::Private & p )
  : trainer( p.trainer ), cmlrn( p.cmlrn), cmtst( p.cmtst ), 
    models( p.models ), cycles( p.cycles ), cycles_tst( p.cycles_tst ), 
    miter( p.miter )
{
}


TrainerIterator::Private::~Private()
{
}


TrainerIterator::Private & 
TrainerIterator::Private::operator = ( const TrainerIterator::Private & p )
{
  if( this != &p )
    {
      trainer = p.trainer;
      cmlrn = p.cmlrn;
      cmtst = p.cmtst;
      models = p.models;
      cycles = p.cycles;
      cycles_tst = p.cycles_tst;
      miter = p.miter;
    }
  return *this;
}


TrainerIterator::TrainerIterator( Trainer & t, 
                                  const std::set<CGraph *> *lrnBase,
                                  const std::set<CGraph *> *tstBase,
                                  int cycles, int cycles_tst  )
  : d( new Private( t, lrnBase, tstBase, cycles, cycles_tst ) )
{
}


TrainerIterator::TrainerIterator( const TrainerIterator & it )
  : d( new Private( *it.d ) )
{
}


TrainerIterator::~TrainerIterator()
{
  delete d;
}


TrainerIterator & TrainerIterator::operator = ( const TrainerIterator & x )
{
  if( this != &x )
    {
      *d = *x.d;
    }
  return *this;
}


void TrainerIterator::train(carto::Object &o)
{
  if( isValid() )
      d->trainer->trainOne( *this, o);
}

void TrainerIterator::next()
{
      ++d->miter;
}


bool TrainerIterator::isValid() const
{
  return d->trainer && d->cmlrn && d->models && d->miter != d->models->end();
}


Model* TrainerIterator::model()
{
  return *d->miter;
}


Adaptive* TrainerIterator::adaptive()
{
  return dynamic_cast<Adaptive *>( *d->miter );
}


TrainerIterator::CliquesModelMap* TrainerIterator::learnBase()
{
  return d->cmlrn.get();
}


TrainerIterator::CliquesModelMap* TrainerIterator::testBase()
{
  return d->cmtst.get();
}


int TrainerIterator::cycles() const
{
  return d->cycles;
}


int TrainerIterator::testCycles() const
{
  return d->cycles_tst;
}


int TrainerIterator::count() const
{
  return d->models->size();
}


