
#ifndef SI_FOLD_FOLDCOPIER_H
#define SI_FOLD_FOLDCOPIER_H


#include <si/learner/copyLearner.h>


namespace sigraph
{

  /**	Duplicateur de clique de sillons, qui prend en compte cet andouille 
	de noeud "brain_hull" qui fout le bazar.
  */
  class FoldCopier : public CopyLearner
  {
  public:
    FoldCopier();
    virtual ~FoldCopier();

    virtual void process(LearnConstParam *lp);
    virtual void process(LearnParam *lp);

  protected:
    FoldCopier( const std::string & syntax ) : CopyLearner( syntax ) {}

  private:
  };


  // inline


  inline void FoldCopier::process(LearnParam *lp)
  {
  	process((LearnConstParam *) lp);
  }

}

#endif


