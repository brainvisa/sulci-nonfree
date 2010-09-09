/* Copyright (c) 1995-2006 CEA
 *
 *  This software and supporting documentation were developed by
 *      CEA/DSV/SHFJ
 *      4 place du General Leclerc
 *      91401 Orsay cedex
 *      France
 *
 */

#include <cstdlib>
#include <si/subadaptive/svmparser.h>
#include <si/subadaptive/subadsvm.h>
#include <si/subadaptive/saParser.h>
#include <si/model/adaptiveLeaf.h>
#include <si/graph/attrib.h>
#include <graph/tree/tree.h>
#include <cartobase/stream/fileutil.h>
#include <datamind/libsvm/libsvm.h>
#include <cartobase/exception/assert.h>

using namespace sigraph;
using namespace carto;
using namespace std;


void SvmSAParser::buildSubSvm( AttributedObject* parent, Tree* ao,
                               const string & filename )
{
  // cout << "SvmSAParser::buildSubSvm\n";
  if( ao->size() != 0 )
  {
    cerr << "warning : SubAdSvm with children (" << ao->size() << ")\n";
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
        ASSERT( adp );
        parent->getProperty( SIA_WORK, wrk );
        parent->getProperty( SIA_EVAL, evl );
        ao->getProperty( "name", name );
        if( name != wrk && name != evl )
        {
          cerr << "sub_ad_svm does not correspond to a work or eval "
              << "part of the parent leaf!\n";
        }
        else
        {
          string	str;

          //	network
          ao->getProperty( "net", str );
          //	name of SVM
          string	file = FileUtil::dirname( filename );
          if( !file.empty() )
            file += FileUtil::separator();
          file += str;

          SubAdSvm	sad( name, str, file );

          parseSubSvm( parent, ao, sad );

          if( name == wrk )
          {
            adp->setWork( sad );
          }
          else if( name == evl )
          {
            adp->setEval( sad );
          }
        }
      }
      else
      {
        cerr << "sub_ad_svm parent has no pointer!\n";
      }
    }
    else
    {
      cerr << "sub_ad_svm parent is NOT a ad_leaf!\n";
    }
  }
  else
  {
    cerr << "sub_ad_svm without a parent!\n";
  }
}


void SvmSAParser::parseSubSvm(AttributedObject *parent, Tree *t, SubAdSvm &sad)
{
  SAParser::parseSubAd(parent, t, sad);

  float		nb, qs, f;
  string		mode;
  int		qsh = 1;

  struct svm_parameter &svm_param = sad.getSvmParameter();

  svm_param.nr_weight = 2;
  svm_param.weight_label = (int *) malloc(sizeof(int) * 2);
  svm_param.weight = (double *) malloc(sizeof(double) * 2);
  svm_param.weight_label[0] = 0;
  svm_param.weight_label[1] = 1;
  svm_param.weight[0] = svm_param.weight[1] = 0.5;
  svm_param.svm_type = C_SVC;
  svm_param.kernel_type = RBF;
  svm_param.degree = 3;
  svm_param.gamma = -1;
  svm_param.coef0 = 0;
  svm_param.nu = 0.5;
  svm_param.cache_size = 40;
  svm_param.C = 1;
  svm_param.eps = 1e-3;
  svm_param.p = 0.1;
  svm_param.shrinking = 1;
  svm_param.probability = 0;

  if (t->getProperty("min_out", nb))
    sad.setMinOut( nb );
  if (t->getProperty("max_out", nb))
    sad.setMaxOut( nb );
  if (t->getProperty("quality_slope", qs))
    sad.setQualitySlope(qs);
  if (t->getProperty("svm_mode", mode))
    {
      if (mode == "classifier")
        {
          sad.setSvmMode(SubAdSvm::Classifier);
          svm_param.svm_type = C_SVC;
        }
      else if (mode == "probability")
        {
          sad.setSvmMode( SubAdSvm::Probability );
          svm_param.svm_type = C_SVC;
          svm_param.probability = 1;
        }
      else if (mode == "regression")
        {
          sad.setSvmMode( SubAdSvm::Regression );
          svm_param.svm_type = EPSILON_SVR;
        }
      else if (mode == "oneclass")
	{
	  sad.setSvmMode( SubAdSvm::OneClass );
          svm_param.svm_type = ONE_CLASS;
	}
      else if (mode == "quality")
        {
          sad.setSvmMode(SubAdSvm::Quality);
          svm_param.svm_type = C_SVC;
        }
      else if ( mode == "decision" )
        {
          sad.setSvmMode(SubAdSvm::Decision);
          svm_param.svm_type = C_SVC;
        }
      else cerr << "SAParser::parseSubSvm: unrecognized svm_mode: "
                << mode << endl;
    }
  if (t->getProperty("quality_shifted_bad_output", qsh))
    sad.setQualityShiftedBadOutput(qsh);
  if (t->getProperty("kernel_type", mode))
    {
      if (mode == "linear")
        svm_param.kernel_type = LINEAR;
      else if (mode == "poly")
        svm_param.kernel_type = POLY;
      else if (mode == "rbf")
        svm_param.kernel_type = RBF;
      else if (mode == "sigmoid")
        svm_param.kernel_type = SIGMOID;
    }
  if (t->getProperty("degree", f))
    svm_param.degree = f;
  if (t->getProperty("gamma", f))
    svm_param.gamma = f;
  if (t->getProperty("coef0", f))
    svm_param.coef0 = f;
  if (t->getProperty("C", f))
    svm_param.C = f;
  if (t->getProperty("nu", f))
    svm_param.nu = f;
  if (t->getProperty("epsilon", f))
    svm_param.p = f;
}


#include <cartobase/object/object_d.h>
INSTANTIATE_GENERIC_OBJECT_TYPE( svm_model * )

