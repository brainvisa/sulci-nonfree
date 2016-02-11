#include <cstdlib>
#include <si/subadaptive/subAdaptive.h>
#include <si/learnable/vectorLearnable.h>
#include <graph/tree/tree.h>
#include <si/graph/attrib.h>
#include <cartobase/stream/fileutil.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <stdio.h>
#include <math.h>
#include <assert.h>

using namespace sigraph;
using namespace std;


double SubAdaptive::ForgetFactor = 0.01;
double SubAdaptive::LocalForgetFactor = 0.003;
double SubAdaptive::GenForgetFactor = 0.05;
double SubAdaptive::GenGoodForgetFactor = 0.05;
double SubAdaptive::GenBadForgetFactor = 0.05;





SubAdaptive::SubAdaptive( const string name ) 
  : _name( name ), _nStats( 0 ), _stream( 0 ), _tstream( 0 ), 
    _errorRate( 0.5 ), _genErrorRate( 0.5 ), _mg( 0.5 ), 
    _Mg( 0. ), _ml( 0.25 ), _Ml( 0.25 ), _ggErrorRate( 0.5 ), _mgl( 0.5 ), 
    _Mgl( 0. ), _mgg( 0.25 ), _Mgg( 0.25 ), _agErrorRate( 0.5 ), 
    _stepsSinceGenMin( 0 ), _gbErrorRate( 0.5 ), _mggb( 0.5 ),
    _misclassGoodRate( 0.5 ), _misclassBadRate( 0.5 ),
    _minOut( 0 ), _maxOut( 0 ), _learnfinished(false),
    _real_class_mode(RealClassFromPotential),
    _relianceWeightMethod( MeanGenError )
{
}


SubAdaptive::~SubAdaptive()
{
  delete _stream;
  delete _tstream;
}


void SubAdaptive::buildTree( Tree & tr ) const
{
  tr.setProperty( SIA_NAME, _name );

  vector<float>				vm, vs;

  for (unsigned int i = 0; i < _stats.size(); ++i)
    {
      const Stat	&st = (*_stats.find( i )).second;

      vm.push_back( st.mean );
      vs.push_back( st.sigma );
    }

  tr.setProperty( SIA_NSTATS, (int) _nStats );
  tr.setProperty( SIA_MEAN, vm );
  tr.setProperty( SIA_SIGMA, vs );
  tr.setProperty( SIA_ERROR_RATE, (float) _errorRate );
  tr.setProperty( SIA_GEN_ERROR_RATE, (float) _genErrorRate );
  tr.setProperty( SIA_GLOBAL_MIN_GERROR, (float) _mg );
  tr.setProperty( SIA_GLOBAL_MAX_GERROR, (float) _Mg );
  tr.setProperty( SIA_LOCAL_MIN_GERROR, (float) _ml );
  tr.setProperty( SIA_LOCAL_MAX_GERROR, (float) _Ml );
  tr.setProperty( SIA_GEN_GOOD_ERROR_RATE, (float) _ggErrorRate );
  tr.setProperty( SIA_GEN_BAD_ERROR_RATE, (float) _gbErrorRate );
  tr.setProperty( SIA_GLOBAL_GOOD_MIN_GERROR, (float) _mgg );
  tr.setProperty( SIA_GLOBAL_GOOD_MAX_GERROR, (float) _Mgg );
  tr.setProperty( SIA_LOCAL_GOOD_MIN_GERROR, (float) _mgl );
  tr.setProperty( SIA_LOCAL_GOOD_MAX_GERROR, (float) _Mgl );
  tr.setProperty( SIA_APP_GOOD_ERROR_RATE, (float) _agErrorRate );
  tr.setProperty( SIA_STEPS_SINCE_GEN_MIN, (int) _stepsSinceGenMin );
  tr.setProperty( SIA_GLOBAL_GEN_MIN_ERROR, (float) _mggb );
  tr.setProperty( SIA_MISCLASS_GOOD_RATE, (float) _misclassGoodRate );
  tr.setProperty( SIA_MISCLASS_BAD_RATE, (float) _misclassBadRate );
  tr.setProperty( "real_class_mode", getRealClassMode() );
  switch( relianceWeightMethod() )
  {
    case MeanGenError:
      tr.setProperty( SIA_RELIANCE_WEIGHT_METHOD, "mean_error" );
      break;
    case MisclassFrequency:
      tr.setProperty( SIA_RELIANCE_WEIGHT_METHOD, "misclass_frequency" );
      break;
  }
}


void SubAdaptive::setStats(const std::vector<double> &mean,
			   const std::vector<double> &sigma)
{
	assert(mean.size() == sigma.size());

	for (unsigned int i = 0; i < mean.size(); ++i)
	{
		_stats[i].mean = mean[i];
		_stats[i].sigma = sigma[i];
	}
}

void SubAdaptive::resetStats()
{
  _nStats = 0;
  unsigned int size = _stats.size();
  _stats.erase( _stats.begin(), _stats.end() );

  for (unsigned int i = 0; i < size; ++i)
    {
      _stats[ i ].mean = 0.;
      _stats[ i ].sigma = 1.;
    }

  _errorRate = 0.5;
  _genErrorRate = 0.5;
  _mg = 0.5;
  _Mg = 0.;
  _ml = 0.25;
  _Ml = 0.25;
  _ggErrorRate = 0.5;
  _mgl = 0.25;
  _Mgl = 0;
  _mgg = 0.5;
  _agErrorRate = 0.5;
  _gbErrorRate = 0.5;
  _stepsSinceGenMin = 0;
  _mggb = 1.;
  _misclassGoodRate = 0.5;
  _misclassBadRate = 0.5;
}


void SubAdaptive::learnStats( const vector<double> &vec, double )
{
  double			val;

  for (unsigned int i = 0; i < _stats.size(); ++i)
    {
      Stat 	& s = _stats[i];
      val = vec[i];

      /*_stats[n].mean += vec[n];
	_stats[n].sigma += vec[n] * vec[n];*/

      s.sigma = sqrt( ( s.sigma * s.sigma 
			+ ( val - s.mean ) * ( val - s.mean ) 
			/ (_nStats + 1) ) 
		      * _nStats / (_nStats + 1) );
      s.mean = ( s.mean * _nStats + val ) / (_nStats + 1);
    }

  ++_nStats;
}


bool SubAdaptive::openFile( const string & filename )
{
  if( fileOpened() )
    return( false );

  string	newname;

  if( filename == "" || filename[ filename.size()-1 ] == '/' 
      || filename[ filename.size()-1 ] == '\\' )
    newname = chooseFilename( filename );
  else
    newname = filename;

  cout << "opening file " << newname << endl;
  _stream = new ofstream( newname.c_str() );

  if( !*_stream )
    {
      cerr << "warning : file " << newname << " not opened.\n";
      delete _stream;
      _stream = 0;
      return( false );
    }

  string::size_type pos = newname.rfind( '.' );
  if( pos == string::npos )
    pos = newname.size();
  newname.insert( pos, "-tst" );
  cout << "opening file " << newname << endl;
  _tstream = new ofstream( newname.c_str() );

  if( !*_tstream )
    {
      cerr << "warning : file " << newname << " not opened.\n";
      delete _tstream;
      _tstream = 0;
      return false;
    }

  return true;
}


void SubAdaptive::closeFile()
{
  delete _stream;
  _stream = 0;
  delete _tstream;
  _tstream = 0;
}


string SubAdaptive::chooseFilename( const string & basename ) const
{
  return( basename + _name + ".dat" );
}


void SubAdaptive::subFiles( const string &, set<string> & ) const
{
}


void SubAdaptive::updateErrors( double err )
{
  ++_stepsSinceGenMin;
  _genErrorRate = (1. - GenForgetFactor) * _genErrorRate 
    + GenForgetFactor * err;
  if( _genErrorRate < _mg )
    _mg = _genErrorRate;
  if( _genErrorRate > _Mg )
    _Mg = _genErrorRate;
  if( _genErrorRate < _ml )
    _ml = _genErrorRate;
  else
    _ml = (1. - LocalForgetFactor) * _ml + LocalForgetFactor * _genErrorRate;
  if( _genErrorRate > _Ml )
    _Ml = _genErrorRate;
  else
    _Ml = (1. - LocalForgetFactor) * _Ml + LocalForgetFactor * _genErrorRate;
}


void SubAdaptive::updateGoodErrors( double err )
{
  _ggErrorRate = (1. - GenGoodForgetFactor) * _ggErrorRate 
    + GenGoodForgetFactor * err;
  if( _ggErrorRate < _mgg )
    _mgg = _ggErrorRate;
  if( _ggErrorRate + _gbErrorRate < _mggb * 2 )
    {
      _mggb = ( _ggErrorRate + _gbErrorRate ) / 2;
      _stepsSinceGenMin = 0;
    }
  if( _ggErrorRate > _Mgg )
    _Mgg = _ggErrorRate;
  if( _ggErrorRate < _mgl )
    _mgl = _ggErrorRate;
  else
    _mgl = (1. - LocalForgetFactor) * _mgl + LocalForgetFactor * _ggErrorRate;
  if( _ggErrorRate > _Mgl )
    _Mgl = _ggErrorRate;
  else
    _Mgl = (1. - LocalForgetFactor) * _Mgl + LocalForgetFactor * _ggErrorRate;
}


void SubAdaptive::updateBadErrors( double err )
{
  _gbErrorRate = (1. - GenBadForgetFactor) * _gbErrorRate 
    + GenBadForgetFactor * err;
  if( _ggErrorRate + _gbErrorRate < _mggb * 2 )
    {
      _mggb = ( _ggErrorRate + _gbErrorRate ) / 2;
      _stepsSinceGenMin = 0;
    }
}

double SubAdaptive::getLearnedLabel(const SiVectorLearnable &vl) const
{
	return normalizedPotential(vl.y());
}


SubAdResponse *SubAdaptive::test(const SiDBLearnable &tst)
{
	unsigned int				i, size = tst.size();
	std::vector<double>			*true_labels = NULL;
	std::vector<double>			*pred_labels = NULL;

	true_labels = new std::vector<double>(size);
	pred_labels = new std::vector<double>(size);

	for (i = 0; i < size; ++i)
	{
		SiVectorLearnable	*vl = tst[i];
		(*true_labels)[i] = getLearnedLabel(*vl);
		(*pred_labels)[i] = test(*vl);
		delete vl;
	}
	return new SubAdResponse(true_labels, pred_labels);
}



double SubAdaptive::test(const SiVectorLearnable &v)
{
	std::vector<double>	vec = v.X();
	double			outp = v.y();
	double			response = prop(vec);
	double			r = fabs(response - outp);

	updateErrors(r);
	if (outp < 0.2)		updateGoodErrors( r );
	else if(outp > 0.8)	updateBadErrors( r );

	if( _tstream )
	{
		*_tstream << "  " << setw( 8 ) << outp << "  " << r << "  "
			<< _genErrorRate << "  " << _mg << "  " << _Mg << "  "
			<< _ml << "  " << _Ml << "  " << _ggErrorRate << "  "
			<< _mgg << "  " << _gbErrorRate << "  " << _mggb
			<< endl;
	}
	return response;
}


void SubAdaptive::init()
{
  _errorRate = 0.5;
  _genErrorRate = 0.5;
  _ggErrorRate = 0.5;
  _gbErrorRate = 0.5;
  _agErrorRate = 0.5;
  _mg = 0.5;
  _Mg = 0;
  _ml = 0.25;
  _Ml = 0.25;
  _mgg = 0.5;
  _Mgg = 0;
  _mgl = 0.25;
  _Mgl = 0.25;
  _stepsSinceGenMin = 0;
  _mggb = 1.;
}

std::vector<double>*
SubAdaptive::normalizeSelected(const double *vec, unsigned int size)
{
	vector<unsigned>::const_iterator	ii, fi;
	std::vector<double>		*v = NULL;
	unsigned int			i;

	v = new std::vector<double>(size);
	for(i = 0; i < size; ++i)
	{
		struct Stat & stat = _stats[i];

		if( stat.sigma != 0 )
			(*v)[i] = (vec[i] - stat.mean) / stat.sigma;
		else	(*v)[i] = vec[i] - stat.mean;
		//        prévenir les overflow...
		if(fabs((*v)[i]) > 700.)
			(*v)[i] = (*v)[i] > 0 ? 700. : -700.;
	}
	return v;
}


double SubAdaptive::relianceWeight() const
{
  double  fac = 0;
  switch( _relianceWeightMethod )
  {
  case MeanGenError:
    fac = 1. - genMeanErrorRate() * 2;
    if( fac < 0 )
      fac = 0;
    break;
  case MisclassFrequency:
    fac = 1. - misclassRate() * 2;
    if( fac < 0 )
      fac = 0;
    break;
  }
  return fac;
}


double SubAdaptive::genMeanErrorRate() const
{ 
  /* If tested database is too small or pathological (for some uncommon
  sulci and related edges), the error rate could be NaN (Not a Number).
  Thus, the generalization power of this model is unknown. So we inhibit it
  with a rate up to 0.5 which gives a null weight of this model in the
  simulated anneal. */
  if (std::isnan(_ggErrorRate))	return 0.5;
  else if (std::isnan(_gbErrorRate))	return 0.5;
  else	return( ( _ggErrorRate + _gbErrorRate ) / 2 );
}
