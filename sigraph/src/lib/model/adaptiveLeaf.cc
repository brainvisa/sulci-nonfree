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
 */

#include <si/model/adaptiveLeaf.h>
#include <si/graph/attrib.h>
#include <si/subadaptive/subAdMlp.h>
#include <si/subadaptive/stopCriterion.h>
#include <si/learner/learner.h>
#include <si/finder/adapFinder.h>
#include <cartobase/stream/fileutil.h>
#include <graph/tree/tree.h>
#include <iostream>
#include <assert.h>

using namespace sigraph;
using namespace carto;
using namespace std;


AdaptiveLeaf::AdaptiveLeaf( const CliqueDescr *cd, const SubAdaptive *work, 
			    const SubAdaptive *eval )
  : Adaptive(), _work( work ? work->clone() : new SubAdMlp ), 
    _eval( eval ? eval->clone() : 0 ), _cliqueDescr( cd ? cd->clone() : 0 ), 
    _lrnState( LEARNING ), _workMemo( 0 ), _evalMemo( 0 ), _ndataMemo( 0 ),
    _dimreductor(NULL), _optimizer(NULL)
{
}


AdaptiveLeaf::~AdaptiveLeaf()
{
  delete _work;
  delete _eval;
  delete _cliqueDescr;
  delete _workMemo;
  delete _evalMemo;
  delete _dimreductor;
  delete _optimizer;
}


void AdaptiveLeaf::buildTree( Tree & tr ) const
{
  Adaptive::buildTree( tr );

  tr.setSyntax( SIA_AD_LEAF_SYNTAX );
  tr.setProperty( SIA_WORK, _work->name() );

  Tree	*w = new Tree, *e, *cd = new Tree, *dimreductor = new Tree, *opt = new Tree;
  _work->buildTree( *w );
  tr.insert( w );

  if( _eval )
    {
      e = new Tree;
      _eval->buildTree( *e );
      tr.insert( e );
      tr.setProperty( SIA_EVAL, _eval->name() );
    }

  tr.insert( cd );
  _cliqueDescr->buildTree( *cd );

  tr.setProperty( SIA_LEARN_STATE, (int) _lrnState );
  if (_mean.size() > 0) tr.setProperty("mean", (std::vector<float>) _mean);
  if (_std.size() > 0) tr.setProperty("std", (std::vector<float>) _std);
  //	sauver aussi _workMemo, _eval Memo et _ndataMemo
  if( _lrnState == STOPPABLE )
    {
      tr.setProperty( SIA_NB_LEARN_DATA_MEMO, (int) _ndataMemo );
      if( _workMemo )
	{
	  e = new Tree;
	  _workMemo->buildTree( *e );
	  tr.insert( e );
	  tr.setProperty( SIA_WORKMEMO, _workMemo->name() );
	}
      if( _evalMemo )
	{
	  e = new Tree;
	  _evalMemo->buildTree( *e );
	  tr.insert( e );
	  tr.setProperty( SIA_EVALMEMO, _evalMemo->name() );
	}
    }
  if (_dimreductor)
  {
    tr.insert( dimreductor );
    _dimreductor->buildTree( *dimreductor );
  }
  if (_optimizer)
  {
    tr.insert( opt );
    _optimizer->buildTree( *opt );
  }
}


void AdaptiveLeaf::resetStats()
{
  _work->resetStats();
  if( _eval )
    _eval->resetStats();

  AdapDescr	*ad = dynamic_cast<AdapDescr *>( _cliqueDescr );
  if( ad )
    ad->reset();
  setLearnState( LEARNING );
}


void AdaptiveLeaf::init()
{
  setLearnState( LEARNING );
  _work->init();
  if( _eval )
    _eval->init();

  Adaptive::init();
}


bool AdaptiveLeaf::openFile( const string & basename )
{
  bool	res = true;

  res &= _work->openFile( basename );
  if( _eval )
    res &= _eval->openFile( basename );

  return( res );
}


void AdaptiveLeaf::closeFile()
{
  _work->closeFile();
  if( _eval )
    _eval->closeFile();
}


void AdaptiveLeaf::subFiles( const string & prefix, 
			     set<string> & listNames ) const
{
  _work->subFiles( prefix, listNames );
  if( _eval )
    _eval->subFiles( prefix, listNames );
}


void AdaptiveLeaf::setBaseName( const string & basename )
{
  _work->setBaseName( basename );
  if( _eval )
    _eval->setBaseName( basename );
}


double AdaptiveLeaf::printDescription( Clique* cl, bool withnames )
{
  vector<double>	vec;
  AttributedObject	*ao = graphObject();
  double		w;
  TopModel		*tm = topModel();

  if( tm )
    w = tm->weight();
  else w = 1.;

  _cliqueDescr->makeVector( cl, vec, ao );
  _cliqueDescr->preProcess( vec, ao );

  cl->setProperty( SIA_POT_VECTOR, vec );

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
  float p = _work->prop( *redvec );

  if( withnames )
    {
      cl->setProperty( SIA_DESCRIPTOR_NAMES, 
			_cliqueDescr->descriptorsNames() );
      cl->setProperty( SIA_DESCRIPTOR, _cliqueDescr->name() );
    }

  return( p * w * _work->relianceWeight() );
}


/* double AdaptiveLeaf::prop( const Clique* cl, 
			   const map<Vertex*, string> & changes  )
{
  double	pot;

  if( cl->getProperty( SIA_POTENTIAL, pot ) && 
      !_cliqueDescr->hasChanged( cl, changes, graphObject() ) )
    return pot;
  else
    return prop( cl );
} */


bool AdaptiveLeaf::doesOutputChange
( const Clique* cl, const map<Vertex*, string> & changes  ) const
{
  return _cliqueDescr->hasChanged( cl, changes, graphObject() );
}


bool AdaptiveLeaf::checkLearnFinished()
{
  /*cout << "AdaptiveLeaf::learnFinished, state : " << (int) _lrnState 
    << ", learnfinished : " << _learnfinished << endl;*/
  if( !_learnfinished )
    {
      switch( _lrnState )
	{
	case STOPPABLE:
	  if( LearnStopCriterion::theCriterion->stops( *_work, _ndata ) )
	    {
	      setLearnFinished( true );
	      _lrnState = STOPPED;
	      cout << "\t\t(stop apprentissage)       " << flush;
	      revert();
	    }
	  break;
	case STOPPED:
	  //	Ca devrait avoir ��fait avant !
	  cout << "checkLearnFinished : Ca devrait avoir ��fait avant !\n";
	  setLearnFinished( true );
	  break;
	default:	// (LEARNING)
	  if( _workMemo && 
	      LearnStopCriterion::theCriterion->stoppable( *_work, _ndata ) )
	    {
	      _lrnState = STOPPABLE;
	      cout << "\t\t(apprentissage stoppable)" << flush;
	    }
	  break;
	}
    }
  return( _learnfinished );
}


void AdaptiveLeaf::memorize()
{
  //cout << "- m�o mod�e -\n";
  delete _workMemo;
  delete _evalMemo;
  _workMemo = _work->clone();

  string	name = _workMemo->fileNames();
  string::size_type pos = name.rfind( '.' );
  if( pos == string::npos )
    pos = name.size();
  name.insert( pos, "-memo" );
  _workMemo->setFileNames( name );
  name = _workMemo->name();
  name += "-memo";
  _workMemo->setName( name );

  if( _eval )
    {
      _evalMemo = _eval->clone();
      name = _evalMemo->fileNames();
      pos = name.rfind( '.' );
      if( pos == string::npos )
	pos = name.size();
      name.insert( pos, "-memo" );
      _evalMemo->setFileNames( name );
      name = _evalMemo->name();
      name += "-memo";
      _evalMemo->setName( name );
    }
  else
    _evalMemo = 0;
  _ndataMemo = nbLearnData();
}


void AdaptiveLeaf::revert()
{
  if( _workMemo )
    {
      SubAdaptive	*sa = _work;
      _work = _workMemo;
      _work->getStreams( *sa );
      _workMemo = 0;
      delete sa;

      string	name = _work->fileNames();
      string::size_type pos = name.rfind( "-memo" );
      assert( pos != string::npos );
      name.erase( pos, 5 );
      _work->setFileNames( name );
      name = _work->name();
      assert( name.rfind( "-memo" ) == name.size() - 5 );
      name.erase( name.size()-5, 5 );
      _work->setName( name );

      if( _evalMemo )
	{
	  sa = _eval;
	  _eval = _evalMemo;
	  _evalMemo = 0;
	  _eval->getStreams( *sa );
	  delete sa;
	  name = _eval->fileNames();
	  pos = name.rfind( "-memo" );
	  assert( pos != string::npos );
	  name.erase( pos, 5 );
	  _eval->setFileNames( name );
	  name = _eval->name();
	  assert( name.rfind( "-memo" ) == name.size() - 5 );
	  name.erase( name.size()-5, 5 );
	  _eval->setName( name );
	}
      setNbLearnData( _ndataMemo );
      _ndataMemo = 0;
    }
  else
    cerr << "attempt to revert an unmemorized AdaptiveLeaf (BUG)\n";
}


void AdaptiveLeaf::setLearnState( State s )
{
  _lrnState = s;
  if( s != STOPPABLE )
    {
      _ndataMemo = 0;
      delete _workMemo;
      _workMemo = 0;
      delete _evalMemo;
      _evalMemo = 0;
    }
  if( s == STOPPED )
    setLearnFinished( true );
  else
    setLearnFinished( false );
}


void AdaptiveLeaf::forceLearnFinished()
{
  if( _lrnState == STOPPABLE && _workMemo )
    revert();
  _lrnState = STOPPED;
  _learnfinished = true;
}


void	AdaptiveLeaf::update(sigraph::AdaptiveLeaf &ad)
{
	delete _work;
	_work = ad._work->clone();
	delete _eval;
	_eval = ad._eval ? ad._eval->clone() : NULL;
	delete _cliqueDescr;
	_cliqueDescr = ad._cliqueDescr ? ad._cliqueDescr->clone() : NULL;
	_lrnState = ad._lrnState;
	_learnfinished = ad._learnfinished;
	_ndata = ad._ndata;
	delete _workMemo;
	_workMemo = ad._workMemo ? ad._workMemo->clone() : NULL;
	if (_evalMemo) delete _evalMemo;
	_evalMemo = ad._evalMemo ? ad._evalMemo->clone() : NULL;
	_ndataMemo = ad._ndataMemo;
}


void	AdaptiveLeaf::getVectors(AdapDescr &ad, Learner &learner,
				const std::list<Clique *> &cliques,
				int cycles, int &cur_cycle)
{
	std::list<Clique *>::const_iterator		ic, fc = cliques.end();
	LearnConstParam		lp;

	lp.adap = this;
	lp.descr = &ad;

	for (int c = 1; c <= cycles; ++c)
	{
		cout << "\r\tGeneration cycle " << setw(5) << c << " / "
                        << setw(5) << cycles << flush;
		cur_cycle++;
		lp.cycle = cur_cycle;
		for (ic = cliques.begin(); ic != fc; ++ic)
		{
			lp.clique = *ic;
			// bon exemple par d�aut
			lp.class_id = 0;
			if (!(*ic)->getProperty( "learn_potential", lp.outp))
				lp.outp = -1.;
			learner.getVectors(&lp);
		}
	}
	cout << endl;
}

AdapDescr	*AdaptiveLeaf::getAdapDescr(void)
{
	AdapDescr	*ad = dynamic_cast<AdapDescr*>(_cliqueDescr);
	if (!ad)
	{	
		std::cerr << "warning : no adaptive descriptor found : "
			  << "skip learning" << std::endl;
		return NULL;
	}
	return ad;
}

std::string	AdaptiveLeaf::getDataBaseName(const std::string &prefix)
{
	std::string		basename;
	AttributedObject	*modV = graphObject();
	modV->getProperty("model_file", basename);
	string::size_type	pos = basename.rfind('.');
	//FIXME : on devrait peut-�re checker plus t� si le type est '.mod'
	//FIXME : on devrait peut-�re les mettre dans un sous-r�ertoire data
	if (pos == string::npos) pos = basename.size();
	basename = basename.erase(pos) + "_" + _work->name();

	return prefix + FileUtil::separator() + basename;
}

void	AdaptiveLeaf::generateDataBase(Learner &learner,
				const std::string &prefix,
				const std::list<Clique *> *lrnCliques,
				const std::list<Clique *> *tstCliques,
				int cycles, int cycles_tst)
{
	int			cur_cycle = 0;
	int			split;
	std::string		basename;
	AdapDescr		*ad = getAdapDescr();

	if (!ad)	return;
	getVectors(*ad, learner, *lrnCliques, cycles, cur_cycle);
	split = ad->getGeneratedVectors().size();
	if (tstCliques)
		getVectors(*ad, learner, *tstCliques, cycles_tst, cur_cycle);
	ad->updateSiDBLearnable();
	SiDBLearnable &sidblearnable = ad->getSiDBLearnable();
	sidblearnable.setSplit(split);
	sidblearnable.setCycles(cycles);
	basename = getDataBaseName(prefix);
}

void AdaptiveLeaf::trainStats(Learner &learner,
	const std::list<Clique *> &cliques)
{
	std::list<Clique *>::const_iterator		ic, fc;
	LearnConstParam					lp;
	unsigned int					i, s = cliques.size();

	lp.adap = this;
	lp.descr = getAdapDescr();


	for (ic = cliques.begin(), i = 0, fc = cliques.end();
					ic != fc; ++ic, ++i)
	{
		cout << "\r\tCliques " << setw(3) << (i + 1) << " / "
			<< setw(3) << s << flush;
		lp.clique = *ic;
		// bon exemple par d�aut
		lp.class_id = 0;
		if (!(*ic)->getProperty( "learn_potential", lp.outp))
			lp.outp = -1.;
		learner.getVectors(&lp);
	}
	cout << endl;
}

void AdaptiveLeaf::centerAndReduce(std::vector<double> &vec)
{
	assert(vec.size() == _mean.size());

	for (unsigned int i = 0; i < vec.size(); ++i)
	{
		if( _std[i] != 0 )
			vec[i] = (vec[i] - _mean[i]) / _std[i];
		else	vec[i] = vec[i] - _mean[i];
	}
}





