
#include <si/model/adaptiveTree.h>
#include <si/mixer/meanMixer.h>
#include <graph/tree/tree.h>

using namespace sigraph;
using namespace std;


AdaptiveTree::AdaptiveTree( const string mix_method ) : Adaptive()
{
  if( mix_method == "mean_mixer" )
    {
      _mixer = new MeanMixer;
    }
  else
    _mixer = new MeanMixer;
}


AdaptiveTree::~AdaptiveTree()
{
  iterator	id, fd = end();

  for( id = begin(); id!=fd; ++id )
    delete ( *id );

  delete _mixer;
}

void	AdaptiveTree::generateDataBase(Learner &learner,
		const std::string &prefix,
		const std::list<Clique *> *lrnCliques,
		const std::list<Clique *> *tstCliques,
		int cycles, int cycles_tst)
{
	const_iterator		i, f;

	for (i = begin(), f = end(); i != f; ++i)
		(*i)->generateDataBase(learner, prefix, lrnCliques, tstCliques,
						cycles, cycles_tst);
}


bool AdaptiveTree::checkLearnFinished()
{
  if( !_learnfinished )
    {
      const_iterator	i, f=end();
      bool		s = true;

      for( i=begin(); i!=f; ++i )
	{
	  if( !(*i)->checkLearnFinished() )
	    s = false;
	}
      setLearnFinished( s );
    }
  return( _learnfinished );
}


double AdaptiveTree::prop( const Clique* cl )
{
  vector<double>	res;
  const_iterator	i, f=end();

  for( i=begin(); i!=f; ++i ) res.push_back( (*i)->prop( cl ) );
  return( _mixer->mix( res ) );
}


double AdaptiveTree::prop( const Clique* cl, 
			   const map<Vertex*, string> & changes )
{
  vector<double>	res;
  const_iterator	i, f=end();

  for( i=begin(); i!=f; ++i ) res.push_back( (*i)->prop( cl, changes ) );
  return( _mixer->mix( res ) );
}


bool AdaptiveTree::doesOutputChange
( const Clique* cl, const map<Vertex*, string> & changes ) const
{
  const_iterator        i, f=end();

  for( i=begin(); i!=f; ++i )
    if( (*i)->doesOutputChange( cl, changes ) )
      return true;
  return false;
}


double AdaptiveTree::eval( const Clique* cl )
{
  vector<double>	res;
  const_iterator	i, f=end();

  for( i=begin(); i!=f; ++i ) res.push_back( (*i)->eval( cl ) );
  return( _mixer->mix( res ) );
}


double AdaptiveTree::errorRate() const
{
  vector<double>	res;
  const_iterator	i, f=end();

  for( i=begin(); i!=f; ++i ) res.push_back( (*i)->errorRate() );
  return( _mixer->mix( res ) );
}


double AdaptiveTree::genErrorRate() const
{
  vector<double>	res;
  const_iterator	i, f=end();

  for( i=begin(); i!=f; ++i ) res.push_back( (*i)->genErrorRate() );
  return( _mixer->mix( res ) );
}


double AdaptiveTree::relianceWeight() const
{
  vector<double>        res;
  const_iterator        i, f=end();

  for( i=begin(); i!=f; ++i ) res.push_back( (*i)->relianceWeight() );
  return( _mixer->mix( res ) );
}


void AdaptiveTree::buildTree( Tree & tr ) const
{
  Adaptive::buildTree( tr );

  tr.setSyntax( "ad_tree" );

  Tree	*tree = new Tree;

  tr.insert( tree );
  _mixer->buildTree( *tree );

  Tree	*child;

  const_iterator	ic, fc=end();
  for( ic=begin(); ic!=fc; ++ic )
    {
      child = new Tree;
      (*ic)->buildTree( *child );
      tr.insert( child );
    }
}


void AdaptiveTree::resetStats()
{
  const_iterator	i, f=end();

  for( i=begin(); i!=f; ++i )
    (*i)->resetStats();
}


void AdaptiveTree::init()
{
  const_iterator	i, f=end();

  for( i=begin(); i!=f; ++i )
    (*i)->init();

  Adaptive::init();
}


void	AdaptiveTree::trainStats(Learner &learner,
		const std::list<Clique *> &cliques)
{
  const_iterator	i, f=end();

  for( i=begin(); i!=f; ++i )
    (*i)->trainStats(learner, cliques);
}


bool AdaptiveTree::openFile( const string & basename )
{
  const_iterator	i, f=end();
  bool			res = true;

  for( i=begin(); i!=f; ++i )
    res &= (*i)->openFile( basename );

  return( res );
}


void AdaptiveTree::closeFile()
{
  const_iterator	i, f=end();

  for( i=begin(); i!=f; ++i )
    (*i)->closeFile();
}


void AdaptiveTree::subFiles( const string & prefix, 
			     set<string> & listNames ) const
{
  const_iterator	i, f=end();

  for( i=begin(); i!=f; ++i )
    (*i)->subFiles( prefix, listNames );
}


void AdaptiveTree::setBaseName( const string & basename )
{
  const_iterator	i, f=end();

  for( i=begin(); i!=f; ++i )
    (*i)->setBaseName( basename );
}


double AdaptiveTree::printDescription( Clique* cl, bool naming )
{
  vector<double>	res;
  const_iterator	i, f=end();

  for( i=begin(); i!=f; ++i )
    res.push_back( (*i)->printDescription( cl, naming ) );
  return( _mixer->mix( res ) );
}


void AdaptiveTree::forceLearnFinished()
{
  if( !_learnfinished )
    {
      const_iterator	i, f=end();

      for( i=begin(); i!=f; ++i )
	(*i)->forceLearnFinished();
    }
  _learnfinished = true;
}




