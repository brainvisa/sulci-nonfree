/*
 *  Copyright (C) 1998-2004 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 *  Slices of a mask to buckets
 *
 */

#include <assert.h>
#include <si/learner/badLearner.h>

using namespace sigraph;


BadLearner::BadLearner() : ConstLearner( true, "bad_learner" )
{
}


BadLearner::~BadLearner()
{
}


void BadLearner::process(LearnConstParam *lp)
{
  const_iterator	it, ft=end();
  Learner		*lrn;
  int			old_class_id;

  old_class_id = lp->class_id;
  lp->class_id = 1;
  lp->outp = -lp->outp;	// inversion basique

  assert( !isLeaf() );	// si pas d'enfants, ça sert à rien

  for( it=begin(); it!=ft; ++it )
    {
      lrn = dynamic_cast<Learner *>( *it );
      assert( lrn );
      lrn->process(lp);
    }
  lp->outp = -lp->outp;	// retour à la normale
  lp->class_id = old_class_id;
}


void BadLearner::process(LearnParam *lp)
{
  const_iterator	it, ft=end();
  Learner		*lrn;

  int			old_class_id;

  old_class_id = lp->class_id;
  lp->class_id = 1;
  lp->outp = -lp->outp;	// inversion basique

  assert( !isLeaf() );	// si pas d'enfants, ça sert à rien

  for( it=begin(); it!=ft; ++it )
    {
      lrn = dynamic_cast<Learner *>( *it );
      assert( lrn );
      lrn->process(lp);
    }
  lp->outp = -lp->outp;	// retour à la normale
  lp->class_id = old_class_id;
}






