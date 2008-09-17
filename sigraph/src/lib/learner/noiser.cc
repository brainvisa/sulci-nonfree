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

#include <si/learner/noiser.h>

using namespace sigraph;
using namespace std;


Noiser::Noiser( const string & syntax ) : Learner( true, syntax )
{
}


Noiser::~Noiser()
{
}


void Noiser::process(LearnParam *lp)
{
  const_iterator	il, fl=end();
  Learner		*lrn;
  int			i;

  assert( !isLeaf() );	// si pas d'enfants, ça sert à rien

  for( i=0; i<10 && noise( lp->clique, lp->outp) == 0; ++i ) {}
  // exécute les descendants ssi noise() retourne true
  if( i < 10 )
    for( il=begin(); il!=fl; ++il )
      {
	lrn = dynamic_cast<Learner *>( *il );
	assert( lrn );
	lrn->process(lp);
      }
}





