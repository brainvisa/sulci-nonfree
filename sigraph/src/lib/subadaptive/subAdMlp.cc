/*
 *  Copyright (C) 1998-2006 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 */

#include <cstdlib>
#include <si/subadaptive/subAdMlp.h>
#include <si/model/adaptiveLeaf.h>
#include <si/model/mWriter.h>
#include <graph/tree/tree.h>
#include <aims/vector/vector.h>
#include <cartobase/stream/fileutil.h>
#include <cartobase/stream/directory.h>
#include <stdio.h>
#include <assert.h>

using namespace sigraph;
using namespace carto;
using namespace std;

namespace
{
  bool mwriterParseMlp( MWriter & mw, const AttributedObject* ao )
  {
    string       filename;
    mlp<double,double> *n;
    if( ao->getProperty( "net", filename ) 
        && ao->getProperty( "netptr", n ) )
      {
        string	file = mw.name();
        char	sep = FileUtil::separator();

        file = FileUtil::dirname( file ) + sep + filename;
        Directory     dir( FileUtil::dirname( file ) );

        dir.mkdir();
        n->save( file.c_str() );
        return true;
      }
    return false;
  }
}


SubAdMlp::SubAdMlp( const string name ) : IncrementalSubAdaptive( name ), 
  _net( "SubAdMlp", 2, &AimsVector<int,2>( 1,1 )[0] /*(int[2]) {1, 1}*/ ), _eta( 0.1 )
{
  MWriter::registerParser( &mwriterParseMlp );
}


SubAdMlp::SubAdMlp( const string name, const string filename, 
		    const string file ) 
	: IncrementalSubAdaptive( name ), _eta( 0.1 ),
	_netFileName( filename )
{
  MWriter::registerParser( &mwriterParseMlp );
  _net.load( file.c_str() );
}


SubAdMlp::SubAdMlp( const char* nom, int nc, int* couch ) 
	: IncrementalSubAdaptive(), _net( nom, nc, couch ), _eta( 0.1 )
{
  MWriter::registerParser( &mwriterParseMlp );
}


SubAdMlp::~SubAdMlp()
{
}

void SubAdMlp::prepare(const std::vector<double> &v)
{
	prepare(&(v[0]), v.size());
}

void SubAdMlp::prepare(const SiVectorLearnable &vl)
{
	prepare(vl.X());
}

void SubAdMlp::prepare(const double *vec_raw, unsigned int size)
{
	std::vector<double>	*vec = normalizeSelected(vec_raw, size);
	assert(vec->size() >= (unsigned) _net.ni());
        // FIXME: usedInputs is not used nor applied anymore !

	for(int i = 0; i < _net.ni(); ++i)
		_net.u()[i].set_o((*vec)[i]);
	delete vec;
}


SubAdResponse *SubAdMlp::train(AdaptiveLeaf &al,
				const SiDBLearnable &train,
				const SiDBLearnable &test)
{
	learn(al, train, test);
	return NULL; // no response
};


double SubAdMlp::learn(AdaptiveLeaf &al,
				const SiDBLearnable &train,
				const SiDBLearnable &tst)
{
	unsigned int		itrn, itst, ftrn, ftst;
	//FIXME : synchroniser les ndata aux _ndata de l'AdaptiveLeaf
	unsigned int		ndata = 0;
	bool			learnFinished = false;
	double			err = 0.;
	int	old_cycle_trn, cur_cycle_trn;
	int	old_cycle_tst, cur_cycle_tst;
	int	cycles_number = train.getCycles();
	SiVectorLearnable	*v;

	itrn = 0;
	ftrn = train.size();
	itst = 0;
	ftst = tst.size();
	v = train[itrn];
	cur_cycle_trn = old_cycle_trn = v->cycle();
	delete v;
	v = tst[itst];
	cur_cycle_tst = old_cycle_tst = v->cycle();
	delete v;
	unsigned int ind = 0;
	
	for(int c = 1; !learnFinished && itrn < ftrn && itst < ftst; ++c)
	{
		cout << "\r\tLearning cycle " << setw(5) << c << " / "
                        << setw(5) << cycles_number << flush;
		for (; old_cycle_trn == cur_cycle_trn and itrn < ftrn;
			++itrn, ++ndata)
		{
			++ind;
			v = train[itrn];
			learn(*v);
			cur_cycle_trn = v->cycle();
			delete v;
		}
		old_cycle_trn = cur_cycle_trn;

		if(c % _testPeriod == 0)
		{
			for (; !learnFinished and
				old_cycle_tst == cur_cycle_tst and itst < ftst;
				++itst, ++ndata)
			{
				v = tst[itst];
				err = test(*v);
				cur_cycle_tst = v->cycle();
				if(stepsSinceGenMin() == 0 || !al.workMemoEl())
					al.memorize();
				//Y < 0 => class=0 (c'est vraiment pas top)
	        	        if (v->getY(0) < 0 and (al.learnFinished() or
					al.checkLearnFinished()))
	        	                learnFinished = true;
				delete v;
			}
			old_cycle_tst = cur_cycle_tst;
		}
	}
	cout << endl;
	//Force finishing learning and set work element to workMemo (best min)
	al.setLearnState(AdaptiveLeaf::STOPPABLE);
	al.forceLearnFinished();
	//current subadaptive instance should be different than al.workEl()
	return al.workEl().genErrorRate();
}


double SubAdMlp::learn(const SiVectorLearnable &vl)
{
  double outp = getLearnedLabel(vl);
  prepare(vl);

  //	propager
  _net.prop();

  //	Ecriture
  //double saveout = 0;
  if( fileOpened() )
    {
      for( int i=0; i<_net.ni(); ++i )
	*_stream << _net.u()[i].o() << "\t";
      //saveout = outp;
    }

  //	apprendre
  _net.backprop( &outp, _eta );
  double res = fabs( outp - _net.u()[_net.fo()].o() );

  //	taux d'erreur
  setErrorRate( (1. - ForgetFactor) * errorRate() + ForgetFactor * res );
  if( outp < 0.5 )
    setAppGoodErrorRate( (1. - ForgetFactor) * appGoodErrorRate() 
			 + ForgetFactor * res );

  if( fileOpened() )
    *_stream << _net.u()[_net.fo()].o() << "\t" 
	     << outp << "\t" 
	     << res << "\t" << errorRate() << "\t" << appGoodErrorRate() 
	     << endl;

  return( res );
}


double SubAdMlp::prop( const vector<double> & vec)
{
  prepare(vec);

  _net.prop();

  double	nout = _net.u()[_net.fo()].o();
  double	outp = unNormalizedPotential(nout);

  //	écriture
  if( _tstream )
    {
      for( int i=0; i<_net.ni(); ++i )
	*_tstream << _net.u()[i].o() << "\t";
      *_tstream << _net.u()[_net.fo()].o();
    }

  return( outp );
}


void SubAdMlp::init()
{
	SubAdaptive::init();
	_net.rand_w( -1., 1. );
}


void SubAdMlp::buildTree( Tree & tr ) const
{
	tr.setSyntax( "sub_ad_mlp" );
	SubAdaptive::buildTree( tr );
	tr.setProperty( "net", _netFileName );
	tr.setProperty( "netptr", (mlp<double,double> *) &_net );
	tr.setProperty( "eta", (float) _eta );
	tr.setProperty( "min_out", (float) _minOut );
	tr.setProperty( "max_out", (float) _maxOut );
	tr.setProperty( "testPeriod", (int) _testPeriod );
}


string SubAdMlp::chooseFilename( const string & basename ) const
{
  string	name = basename + _netFileName;
  string::size_type	pos = name.rfind( '.' );

  if( pos == string::npos )
    pos = name.size();
  name.replace( pos, name.size()-pos, ".dat" );

  return name;
}


void SubAdMlp::subFiles( const string & prefix, 
			 set<string> & listNames ) const
{
  listNames.insert( prefix + FileUtil::separator() + fileNames() );
}


void SubAdMlp::setBaseName( const string & basename )
{
  string	name = fileNames();
  string::size_type	pos = name.rfind( '/' );

  if( pos == string::npos ) pos = 0;
  else ++pos;
  name.insert( pos, basename + "_" );
  setFileNames( name );
}


void SubAdMlp::learnStats( const vector<double> &vec, double outp )
{
  if( nStats() == 0 || outp < _minOut )
    _minOut = outp;
  if( nStats() == 0 || outp > _maxOut )
    _maxOut = outp;

  if( _maxOut < 1 )	// hack pas propre pour stats sans mauvais ex.
    _maxOut = 1;

  SubAdaptive::learnStats( vec, outp );
}

double SubAdMlp::getLearnedLabel(const SiVectorLearnable &vl) const
{
	return normalizedPotential(vl.y());
}

double SubAdMlp::test(const SiVectorLearnable &vl)
{
  double outp = getLearnedLabel(vl);
  prepare(vl);

  //	propager
  _net.prop();

  double	nout = _net.u()[_net.fo()].o();
  double	r = fabs( outp - nout );

  //	Ecriture
  if( _tstream )
    {
      //cout << "writing in TEST stream\n";
      for( int i=0; i<_net.ni(); ++i )
	*_tstream << _net.u()[i].o() << "\t";
      *_tstream << nout << "\t" << outp;
    }
  //else
  //cout << "don't write test\n";

  updateErrors( r );
  if( outp < 0.2 )
    updateGoodErrors( r );
  else if( outp > 0.8 )
    updateBadErrors( r );

  if( _tstream )
    {
      *_tstream << "\t" << r << "\t" << _genErrorRate << "\t" << _mg << "\t" 
		<< _Mg << "\t" << _ml << "\t" << _Ml << "\t" << _ggErrorRate 
		<< "\t" << _mgg << "\t" << _gbErrorRate << "\t" 
		<<  ( _ggErrorRate + _gbErrorRate ) / 2 
		<< "\t" << _mggb << endl;
    }

  return nout;
}


#include <cartobase/object/object_d.h>
#define _TMP_ mlp<double, double> *
INSTANTIATE_GENERIC_OBJECT_TYPE( _TMP_ )
#undef _TMP_


