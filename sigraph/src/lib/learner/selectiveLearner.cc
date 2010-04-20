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

#include <si/learner/selectiveLearner.h>
#include <cartobase/exception/assert.h>
#include <iostream>
#include <assert.h>

using namespace sigraph;
using namespace std;


SelectiveLearner::SelectiveLearner( bool allowsChildren, const string & synt )
  : ConstLearner( allowsChildren, synt ), _ready( false )
{
}


SelectiveLearner::~SelectiveLearner()
{
  if( _ready )
    regfree( &_filter );
}


void SelectiveLearner::initRegexp()
{
  string	pattern = ".*";

  getProperty( "pattern", pattern );
  cout << "SelectiveLearner pattern : " << pattern << endl;
  ASSERT( !regcomp( &_filter, pattern.c_str(), REG_NOSUB | REG_EXTENDED ) );
  _ready = true;
}


bool SelectiveLearner::checkClique( const Clique* cl )
{
  if( !_ready )
    initRegexp();

  string	attname = "label", str = "";

  getProperty( "filter_attribute", attname );
  if( !cl->getProperty( attname, str ) )
    {
      //cout << "checkClique: pas d'attribut " << attname << endl;
      return( false );
    }

  if( !regexec( &_filter, str.c_str(), 0, 0, 0 ) )
    {
      //cout << "check clique: attribute " << attname << "; value : " << str 
      //	   << endl;
      return( true );
    }
  //cout << "check clique: attribute " << attname << " loup�\n";
  return( false );
}


void SelectiveLearner::process(LearnParam *lp)
{
  if(!checkClique(lp->clique))
    return;

  const_iterator	il, fl=end();
  Learner		*lrn;

  for( il=begin(); il!=fl; ++il )
    {
      lrn = dynamic_cast<Learner *>( *il );
      ASSERT( lrn != 0 );
      lrn->process(lp);
    }
}


void SelectiveLearner::process(LearnConstParam *lp)
{
  if(!checkClique(lp->clique))
    return;

  const_iterator	il, fl=end();
  ConstLearner		*lrn;

  for( il=begin(); il!=fl; ++il )
    {
      lrn = dynamic_cast<ConstLearner *>( *il );
      ASSERT( lrn != 0 );
      lrn->process(lp);
    }
}





