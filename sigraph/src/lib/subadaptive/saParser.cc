/*
 *  Copyright (C) 1998-2005 CEA
 *
 *  This software and supporting documentation were developed by
 *  	CEA/DSV/SHFJ
 *  	4 place du General Leclerc
 *  	91401 Orsay cedex
 *  	France
 *
 */

#include <si/subadaptive/saParser.h>
#include <si/subadaptive/subAdMlp.h>
#include <si/subadaptive/subAdLogGauss.h>
#include <si/subadaptive/subAdGauss.h>
#include <si/subadaptive/subAdMixGauss.h>
#include <si/model/adaptiveLeaf.h>
#include <graph/tree/tree.h>
#include <cartobase/stream/fileutil.h>
#include <si/graph/attrib.h>
#include <vector>

using namespace sigraph;
using namespace carto;
using namespace std;


SAParser::SAParser()
{
}


SAParser::~SAParser()
{
}


TreePostParser::FactorySet SAParser::factories()
{
  return sharedFactories();
}


TreePostParser::FactorySet & SAParser::sharedFactories()
{
  static TreePostParser::FactorySet	fs;

  fs[ "sub_ad_mlp"   ] = buildSubMlp;
  fs[ "sub_ad_gauss" ] = buildSubGauss;
  fs[ "sub_ad_loggauss" ] = buildSubGauss;
  fs[ "sub_ad_mixgauss" ] = buildSubMixGauss;
  fs[ "matrices_list" ] = buildMatriceList;

  return fs;
}


void SAParser::buildMatriceList( AttributedObject* parent, Tree* ao, 
			    const string &)
{
	AttributedObject	*gf = NULL;
	Model			*mod = NULL;
	AdaptiveLeaf		*al = NULL;
	SubAdMixGauss		*sgauss = NULL;
	
	//Object dic;

	if (!parent->getProperty("parent", gf))
	{
		std::cerr << "matrice list has no grandfather!"<< std::endl;
		return;
	}
	if (!gf->getProperty("pointer", mod))
	{
		std::cerr << "grandfather is not a model"<< std::endl;
		return;
	}
	if (!(al = dynamic_cast<AdaptiveLeaf *>(mod)))
	{
		std::cerr << "model is not an adaptiveleaf"<< std::endl;
		return;
	}
	SubAdaptive	&sad = al->workEl();
	if (!(sgauss = dynamic_cast<SubAdMixGauss *>(&sad)))
	{
		std::cerr << "subadaptive is not a SubAdMixGauss" << std::endl;
		return;
	}

	//if (ao->getProperty("matrices_list", dic))
	//{
	//	const GenericObject	*o = dic.get();
	Object	im;

	for(im = ao->objectIterator(); im->isValid(); im->next())
	{
		Object cv = im->currentValue();
		std::vector<float> m =
			cv->value<std::vector<float> >();
		sgauss->addMatrix(m);
	}
}

void SAParser::buildSubMlp( AttributedObject* parent, Tree* ao, 
			    const string & filename )
{
  if( ao->size() != 0 )
    {
      cerr << "warning : SubAdMlp with children (" << ao->size() << ")\n";
    }

  if( parent )
    {
      if( parent->getSyntax() == "ad_leaf" )
	{
	  if( parent->hasProperty( "pointer" ) )
	    {
	      Model		*mod;
	      AdaptiveLeaf	*adp;
	      string		wrk, evl, wrkm, evlm, name;
	      parent->getProperty( "pointer", mod );
	      adp = dynamic_cast<AdaptiveLeaf*>( mod );
	      if ( ! adp ) 
	      {
	        throw runtime_error( "Internal error: dynamic cast failed" );
	      }
	      parent->getProperty( SIA_WORK, wrk );
	      parent->getProperty( SIA_EVAL, evl );
	      parent->getProperty( SIA_WORKMEMO, wrkm );
	      parent->getProperty( SIA_EVALMEMO, evlm );
	      ao->getProperty( "name", name );
	      if( name != wrk && name != evl && name != wrkm && name != evlm )
		{
		  cerr << "sub_ad_mlp does not correspond to a work or eval "
		       << "part of the parent leaf!\n";
		}
	      else
		{
		  string	str;

		  //	Réseau
		  ao->getProperty( "net", str );
		  //	nom du fichier réseau
		  string	file = FileUtil::dirname( filename );
                  if( !file.empty() )
                    file += FileUtil::separator();
                  file += str;

		  SubAdMlp	sad( name, str, file );

		  parseSubMlp( parent, ao, sad );

		  if( name == wrk )
		    {
		      adp->setWork( sad );
		    }
		  else if( name == evl )
		    {
		      adp->setEval( sad );
		    }
		  else if( name == wrkm )
		    {
		      cout << "Mémo relu\n";
		      adp->setWorkMemo( sad );
		    }
		  else if( name == evlm )
		    adp->setEvalMemo( sad );
		}
	    }
	  else
	    {
	      cerr << "sub_ad_mlp parent has no pointer!\n";
	    }
	}
      else
	{
	  cerr << "sub_ad_mlp parent is NOT a ad_leaf!\n";
	}
    }
  else
    {
      cerr << "sub_ad_mlp without a parent!\n";
    }
}


void SAParser::parseSubAd( AttributedObject*, Tree* t, 
			   SubAdaptive & sad )
{
  //	usedinputs

  unsigned	i;
  string		mode;

  //	stats

  int	nstats;

  t->getProperty( SIA_NSTATS, nstats );
  sad.setNStats( nstats );

  vector<float>				sig, mean;
  map<unsigned, SubAdaptive::Stat>	ms;

  t->getProperty( SIA_SIGMA, sig );
  t->getProperty( SIA_MEAN, mean );

  if ( sig.size() != mean.size() )
  {
    throw runtime_error( "Internal error: sig and mean have different sizes" );
  }

  for( i=0; i< mean.size(); ++i )
    {
      ms[ i ].mean = mean[i];
      ms[ i ].sigma = sig[i];
    }
  sad.setStats( ms );

  //	Taux d'erreur
  float	erate = 0.5;

  t->getProperty( SIA_ERROR_RATE, erate );
  sad.setErrorRate( erate );
  erate = 0.5;
  t->getProperty( SIA_GEN_ERROR_RATE, erate );
  sad.setGenErrorRate( erate );
  if( !t->getProperty( SIA_GLOBAL_MIN_GERROR, erate ) )
    erate = 0.5;
  sad.setGlobalMinGErr( erate );
  if( !t->getProperty( SIA_GLOBAL_MAX_GERROR, erate ) )
    erate = 0.;
  sad.setGlobalMaxGErr( erate );
  if( !t->getProperty( SIA_LOCAL_MIN_GERROR, erate ) )
    erate = 0.25;
  sad.setLocalMinGErr( erate );
  if( !t->getProperty( SIA_LOCAL_MAX_GERROR, erate ) )
    erate = 0.25;
  sad.setLocalMaxGErr( erate );

  erate = 0.5;
  t->getProperty( SIA_GEN_GOOD_ERROR_RATE, erate );
  sad.setGenGoodErrorRate( erate );
  erate = 0.5;
  t->getProperty( SIA_GEN_BAD_ERROR_RATE, erate );
  sad.setGenBadErrorRate( erate );
  erate = 1.;
  t->getProperty( SIA_GLOBAL_GEN_MIN_ERROR, erate );
  sad.setGlobalGenGBError( erate );
  if( !t->getProperty( SIA_GLOBAL_GOOD_MIN_GERROR, erate ) )
    erate = 0.5;
  sad.setGlobalGoodMinGErr( erate );
  if( !t->getProperty( SIA_GLOBAL_GOOD_MAX_GERROR, erate ) )
    erate = 0.;
  sad.setGlobalGoodMaxGErr( erate );
  if( !t->getProperty( SIA_LOCAL_GOOD_MIN_GERROR, erate ) )
    erate = 0.25;
  sad.setLocalGoodMinGErr( erate );
  if( !t->getProperty( SIA_LOCAL_GOOD_MAX_GERROR, erate ) )
    erate = 0.25;
  sad.setLocalGoodMaxGErr( erate );

  erate = 0.5;
  t->getProperty( SIA_APP_GOOD_ERROR_RATE, erate );
  sad.setAppGoodErrorRate( erate );

  nstats = 0;
  t->getProperty( SIA_STEPS_SINCE_GEN_MIN, nstats );
  sad.setStepsSinceGenMin( nstats );

  erate = 0.5;
  t->getProperty( SIA_MISCLASS_GOOD_RATE, erate );
  sad.setMisclassGoodRate( erate );

  erate = 0.5;
  t->getProperty( SIA_MISCLASS_BAD_RATE, erate );
  sad.setMisclassBadRate( erate );

  if (t->getProperty("real_class_mode", mode))
  {
    if (mode == "class")
      sad.setRealClassMode(SubAdaptive::RealClassFromClassID);
    else if (mode == "potential")
      sad.setRealClassMode(SubAdaptive::RealClassFromPotential);
    else
      cerr << "SAParser::parseSubAd: unrecognized real_class_mode: " << mode
        << endl;
  }

  if( t->getProperty( SIA_RELIANCE_WEIGHT_METHOD, mode ) )
  {
    if( mode == "mean_error" )
      sad.setRelianceWeightMethod( SubAdaptive::MeanGenError );
    else if( mode == "misclass_frequency" )
      sad.setRelianceWeightMethod( SubAdaptive::MisclassFrequency );
  }
}


void	SAParser::parseSubMlp(AttributedObject *parent, Tree *t, SubAdMlp &sad)
{
	parseSubAd(parent, t, sad);

	float		f;
	unsigned int	nb;

	t->getProperty("eta", f);
	sad.setEta(f);
	if(t->getProperty("min_out", f))
		sad.setMinOut(f);
	if(t->getProperty("max_out", f))
		sad.setMaxOut(f);
	if(!t->getProperty("testPeriod", nb)) nb = 10;
	sad.setTestPeriod(nb);
}

static SubAdGauss *factorySubAddGauss(std::string name, std::string type)
{
	if (type == "sub_ad_gauss")
		  return new SubAdGauss(name);
	else if (type == "sub_ad_loggauss")
		  return new SubAdLogGauss(name);
	return NULL;
}


void SAParser::buildSubGauss( AttributedObject* parent, Tree* ao, 
			      const string & )
{
  if( ao->size() != 0 )
    {
      cerr << "warning : SubAdGauss with children (" << ao->size() << ")\n";
    }

  if( parent )
    {
      if( parent->getSyntax() == "ad_leaf" )
	{
	  if( parent->hasProperty( "pointer" ) )
	    {
	      Model		*mod;
	      AdaptiveLeaf	*adp;
	      string		wrk, evl, name;
	      parent->getProperty( "pointer", mod );
	      adp = dynamic_cast<AdaptiveLeaf*>( mod );
	      if ( ! adp ) 
	      {
	        throw runtime_error( "Internal error: dynamic cast failed" );
	      }
	      parent->getProperty( "work", wrk );
	      parent->getProperty( "eval", evl );
	      ao->getProperty( "name", name );
	      if( name != wrk && name != evl )
		{
		  cerr << "sub_ad_gauss does not correspond to a work or eval "
		       << "part of the parent leaf!\n";
		}
	      else
		{
		  float		etaW = 0.01, etaC = 0.01, etaS = 1.01, dv = 0;
		  string	lrnfunc = "";

		  //ao->getProperty( "ninputs", ninp );
		  //ao->getProperty( "ngauss", ngss );
		  ao->getProperty( "etaW", etaW );
		  ao->getProperty( "etaC", etaC );
		  ao->getProperty( "etaS", etaS );
		  ao->getProperty( "default_value", dv );

                  SubAdGauss *sad = factorySubAddGauss(name, ao->getSyntax());
		  parseSubAd( parent, ao, *sad );

		  //	params gaussiennes
		  GaussNet & gn = sad->net();
		  int	ninp, ngss, ssig = 1;
		  vector<float>	wt, sg, ct;

		  ao->getProperty( "gauss_weights", wt );
		  ao->getProperty( "gauss_sigma", sg );
		  ao->getProperty( "gauss_centers", ct );
		  ao->getProperty( "gauss_samesigma", ssig );
		  ao->getProperty( "gauss_learn_func", lrnfunc );

		  ninp = ct.size() / wt.size();
		  ngss = wt.size();
		  gn.init( ninp, ngss, ssig );
		  sad->setEtaW( etaW );
		  sad->setEtaC( etaC );
		  sad->setEtaS( etaS );
		  sad->setDefaultValue( dv );

		  if( lrnfunc == "" || lrnfunc == "gradient" )
		    {
		      gn.randInit = &GaussNet::randInitAll;
		      gn.learn = &GaussNet::learnAll;
		    }
		  else if( lrnfunc == "gradient_weights" )
		    {
		      gn.randInit = &GaussNet::randInitWeights;
		      gn.learn = &GaussNet::learnWeights;
		    }
		  else if( lrnfunc == "gradient_centers" )
		    {
		      gn.randInit = &GaussNet::randInitCenters;
		      gn.learn = &GaussNet::learnCenters;
		    }
		  else if( lrnfunc == "gradient_sigma" )
		    {
		      gn.randInit = &GaussNet::randInitSigma;
		      gn.learn = &GaussNet::learnSigma;
		    }
		  else if( lrnfunc == "gradient_wtsig" )
		    {
		      gn.randInit = &GaussNet::randInitWtSig;
		      gn.learn = &GaussNet::learnWtSig;
		    }
		  else if( lrnfunc == "gradient_mulsig" )
		    {
		      gn.randInit = &GaussNet::randInitAll;
		      gn.learn = &GaussNet::learnMulSig;
		    }
		  else if( lrnfunc == "cycle" )
		    {
		      gn.randInit = &GaussNet::randInitAll;
		      gn.learn = &GaussNet::learnCycle;
		    }

		  unsigned	i, j, n = (unsigned) ngss, m = (unsigned) ninp;
		  Gaussian	*gs;
		  vector<float>::const_iterator	ic;

		  for( i=0, ic=ct.begin(); i<n; ++i )
		    {
		      gn.setWeight( i, wt[i] );
		      gs = gn.gauss( i );
		      if( ssig )
			gs->setSigma( 0, sg[i] );
		      else
			{
			  for( j=0; j<m; ++j )
			    gs->setSigma( j, sg[ i*m + j ] );
			}
		      for( j=0; j<m; ++j, ++ic )
			gs->setCenterCoord( j, *ic );
		    }

		  if( name == wrk )
		    {
		      adp->setWork( *sad );
		    }
		  if( name == evl )
		    {
		      adp->setEval( *sad );
		    }
                  delete sad;
		}
	    }
	  else
	    {
	      cerr << "sub_ad_gauss parent has no pointer!\n";
	    }
	}
      else
	{
	  cerr << "sub_ad_gauss parent is NOT a ad_leaf!\n";
	}
    }
  else
    {
      cerr << "sub_ad_gauss without a parent!\n";
    }
}


void SAParser::buildSubMixGauss( AttributedObject* parent, Tree* ao, 
			      const string & )
{
  if( ao->size() > 1 )
    {
      cerr << "warning : SubAdMixGauss with more than one children ("
           << ao->size() << ")\n";
    }

  if( parent )
    {
      if( parent->getSyntax() == "ad_leaf" )
	{
	  if( parent->hasProperty( "pointer" ) )
	    {
	      Model		*mod;
	      AdaptiveLeaf	*adp;
	      string		wrk, evl, name;
	      parent->getProperty( "pointer", mod );
	      adp = dynamic_cast<AdaptiveLeaf*>( mod );
	      if ( ! adp ) 
	      {
	        throw runtime_error( "Internal error: dynamic cast failed" );
	      }
	      parent->getProperty( "work", wrk );
	      parent->getProperty( "eval", evl );
              ao->setProperty( "parent", (AttributedObject *) parent);
	      ao->getProperty( "name", name );
	      if( name != wrk && name != evl )
		{
		  cerr << "sub_ad_gauss does not correspond to a work or eval "
		       << "part of the parent leaf!\n";
		}
	      else
		{
                  std::vector<float>	sqrtdets;
		  ao->getProperty( "sqrtdets", sqrtdets);
		  SubAdMixGauss	sad(name, sqrtdets);
		  parseSubAd( parent, ao, sad );
		  if( name == wrk ) adp->setWork( sad );
		  if( name == evl ) adp->setEval( sad );
		}
	    }
	  else
	    {
	      cerr << "sub_ad_mixgauss parent has no pointer!\n";
	    }
	}
      else
	{
	  cerr << "sub_ad_mixgauss parent is NOT a ad_leaf!\n";
	}
    }
  else
    {
      cerr << "sub_ad_mixgauss without a parent!\n";
    }
}

