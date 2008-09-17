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
#include <si/learner/constLearner.h>

using namespace sigraph;
using namespace std;


ConstLearner::ConstLearner( bool allowsChildren, const string & synt ) 
  : Learner( allowsChildren, synt )
{
}


ConstLearner::~ConstLearner()
{
}


void ConstLearner::process(LearnParam *lp)
{
  const_iterator	il, fl=end();
  Learner		*lrn;

  for( il=begin(); il!=fl; ++il )
    {
      lrn = dynamic_cast<Learner *>( *il );
      assert( lrn != 0 );
      lrn->process(lp);
    }
}


void ConstLearner::process(LearnConstParam *lp)
{
  const_iterator	il, fl=end();
  ConstLearner		*lrn;

  for( il=begin(); il!=fl; ++il )
    {
      lrn = dynamic_cast<ConstLearner *>( *il );
      assert( lrn != 0 );
      lrn->process(lp);
    }
}





