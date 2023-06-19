
#ifndef SI_NO_SVMLIB

#include <cstdlib>
#include <si/subadaptive/subadsvm.h>
#include <si/model/mWriter.h>
#include <si/subadaptive/saParser.h>
#include <si/model/adaptiveLeaf.h>
#include <si/graph/attrib.h>
#include <graph/tree/tree.h>
#include <cartobase/stream/fileutil.h>
#include <cartobase/stream/fdinhibitor.h>
#include <cartobase/stream/directory.h>
#include <iostream>
#include <vector>

using namespace sigraph;
using namespace carto;
using namespace std;


namespace
{

  bool  mwriterParseSvm(MWriter &mw, const AttributedObject *ao)
  {
    string  filename;
    svm_model  *svm = NULL;

    if(ao->getProperty("net", filename)
        && ao->getProperty("netptr", svm) && svm)
    {
      string  file = mw.name();
      char  sep = FileUtil::separator();

      file = FileUtil::dirname(file) + sep + filename;
      Directory     dir( FileUtil::dirname(file));

      dir.mkdir();
      svm_save_model(file.c_str(), svm);
      return true;
    }
    return false;
  }

} // namespace


SubAdSvm::SubAdSvm(const SubAdSvm &sa) : NonIncrementalSubAdaptive(sa),
  _netFileName(sa._netFileName), _svmmode(sa._svmmode),
  _qualityslope(sa._qualityslope), _qualityshifted(sa._qualityshifted),
  _svm_param(sa._svm_param)
{
  MWriter::registerParser(&mwriterParseSvm);
  if (sa._svm)_svm = svm_clone_model(sa._svm, sa._svm_nodes, &_svm_nodes);
  else _svm = NULL;
}


SubAdSvm::SubAdSvm(const std::string name, const std::string filename, 
                    const std::string file)
  : NonIncrementalSubAdaptive(name), _svmmode(Regression),
  _qualityslope(1.), _qualityshifted(true)
{
  MWriter::registerParser(&mwriterParseSvm);
  _netFileName = filename;
  #include "locale.h"
  char  *locale = setlocale(LC_ALL, NULL);
  setlocale(LC_ALL,"C");
  if( FileUtil::fileStat( file ).find('+') == string::npos )
  {
    cout << "model not found\n";
    _svm = 0;
  }
  else
    _svm = svm_load_model(file.c_str());
  setlocale(LC_ALL, locale);
  if(!_svm)
  {
    _svm_nodes = 0;
    std::cout << "Warning : missing file '" << file
      << "', maybe you are trying to create it ?"
      << std::endl;
  }
  else  _svm_nodes = _svm->SV[0];
}


SubAdSvm::~SubAdSvm()
{
#if LIBSVM_VERSION >= 300
  if (_svm)
    svm_free_and_destroy_model(&_svm);
#else
  if (_svm)
    svm_destroy_model(_svm);
  //svm_destroy_param(&_svm_param); //FIXME
#endif
}

void  SubAdSvm::prepare(const std::vector<double> &v)
{
  prepare(&(v[0]), v.size());
}

void  SubAdSvm::prepare(const SiVectorLearnable &vl)
{
  prepare(vl.X());
}

void  SubAdSvm::prepare(const double *vec_raw, unsigned int size)
{
  std::vector<double>  *vec = normalizeSelected(vec_raw, size);

  _inputs.clear();
  _inputs.reserve(size);

  for(unsigned int i = 0; i < vec->size(); ++i)
    _inputs.push_back((*vec)[i]);
  delete vec;
}

struct svm_problem  *SubAdSvm::prepare(const SiDBLearnable &db)
{
  int      idb, fdb;
  struct svm_problem      *problem = new (struct svm_problem);
  std::vector<double>  *vec = NULL;
  SiVectorLearnable  *vl;
  unsigned int    i, j, k, dim;

  idb = 0;
  dim = db.getXcolsNumber();
        problem->l = db.size();

  /* calcul automatique des valeurs par défaut du svm_param */
  if (_svm_param.gamma < 0)
    _svm_param.gamma = 1. / dim;

  /* Création du svm_problem */
  _svm_nodes = (struct svm_node *) malloc(sizeof(svm_node) *
    (problem->l * (dim + 1)));
  problem->y = new double[problem->l];
  problem->x = new struct svm_node * [problem->l];
  for (i = 0, j = 0, idb = 0, fdb = db.size();
    idb < fdb; ++idb, ++i, ++j)
  {
    problem->x[i] = &_svm_nodes[j];
    vl = db[idb];
    std::vector<double> v = vl->X();
    std::vector<double>::const_iterator it, ft;
    vec = normalizeSelected(&(v[0]), vl->sizeX());
    problem->y[i] = getLearnedLabel(*vl);
    delete vl;
    for (k = 0; k != vec->size(); ++j, ++k)
    {
      _svm_nodes[j].index = k + 1;
      _svm_nodes[j].value = (*vec)[k];
    }
    delete vec;
    _svm_nodes[j].index = -1;
    _svm_nodes[j].value = 0;
  }
        return problem;
}

double SubAdSvm::learn(const SiDBLearnable &train)
{
#if LIBSVM_VERSION >= 300
  if (_svm) svm_free_and_destroy_model(&_svm);
#else
  if (_svm) svm_destroy_model(_svm);
#endif
  struct svm_problem  *svm_prob = prepare(train);
  struct svm_node    *tmp = NULL;
  const char    *error_msg;
  fdinhibitor    fdi(stdout);

  error_msg = svm_check_parameter(svm_prob, &_svm_param);
  if (error_msg)
  {
    std::cerr << "Error: " << error_msg << std::endl;
    return 0.;
  }
  fdi.close();
  _svm = svm_train(svm_prob, &_svm_param);
  fdi.open();
  tmp = svm_clean_model(_svm);
  free(_svm_nodes);
  _svm_nodes = tmp;
  delete[] svm_prob->x;
  delete[] svm_prob->y;
  delete svm_prob;
  return 0.; /* Valeur non significative */
}

double SubAdSvm::getLearnedLabel(const SiVectorLearnable &vl) const
{
  switch(_svmmode)
  {
    case OneClass:
      return vl.y() == 0. ? 1. : -1.;
    case Classifier:
    case Probability:
    case Quality:
    case Decision:
      return vl.y();
    case Regression:
    default:
      return normalizedPotential(vl.y());
  }
}

SubAdResponse  *SubAdSvm::crossvalidation(const SiDBLearnable &train,
    const unsigned int nbfolds)
{
  std::vector<double>::const_iterator  iw, fw;
  fdinhibitor    fdi(stdout);
  struct svm_problem  *svm_prob = prepare(train);
  std::vector<double>  *true_labels = NULL, *pred_labels = NULL;
  unsigned int      i, size = svm_prob->l;

  true_labels = new std::vector<double>(size);
  pred_labels = new std::vector<double>(size);

  srand(0);
  fdi.close();
  /* Initialize random seed to a constant value before each
     cross-validation process */
  svm_cross_validation(svm_prob, &_svm_param, nbfolds,&(*pred_labels)[0]);
  fdi.open();
  for (i = 0; i < (unsigned) train.size(); ++i)
  {
    SiVectorLearnable  *vl = train[i];
    (*true_labels)[i] = getLearnedLabel(*vl);

    delete vl;
  }
  return new SubAdResponse(true_labels, pred_labels);
}

double SubAdSvm::prop( const std::vector<double> & vec )
{
  prepare(vec);
  vector<struct svm_node>  svmvec(_inputs.size() + 1);
  unsigned    i, n = _inputs.size();
  unsigned int     dim = vec.size();
  vector<double>    probs(2);

  for(i = 0; i < n; ++i)
  {
    svmvec[i].index = i + 1;
    svmvec[i].value = _inputs[i];
  }
  svmvec[i].index = -1;

  double  nout = 0.;
  switch(_svmmode)
  {
    case Classifier:
    case Regression:
    case OneClass:
      nout = svm_predict(_svm, &svmvec[0]);
      break;
    case Probability:
      svm_predict_probability(_svm, &svmvec[0], &probs[0]);
      nout = probs[1];
      break;
    case Quality:
      nout = svm_predict_quality(_svm, &svmvec[0], dim,
          _qualityslope, _qualityshifted);
      break;
    case Decision:
      nout = svm_predict_decision(_svm, &svmvec[0]);
      break;
  }
  double outp = unNormalizedPotential(nout);

  //  écriture
  if( _tstream )
  {
    for( unsigned i=0; i<_inputs.size(); ++i )
      *_tstream << _inputs[i] << "\t";
    *_tstream << probs[0] << "\t" << probs[1]
      << endl;
  }

  return( outp );
}


double SubAdSvm::test(const SiVectorLearnable &vl)
{
  double  outp = getLearnedLabel(vl);
  
  prepare(vl);

  //  propager
  vector<struct svm_node>  svmvec( _inputs.size() + 1 );
  unsigned      i, dim = _inputs.size();
  for( i=0; i < dim; ++i )
    {
      svmvec[i].index = i + 1;
      svmvec[i].value = _inputs[i];
    }
  svmvec[i].index = -1;
  vector<double>  probs(2);

  double  nout = 0.;
  switch( _svmmode )
    {
    case Classifier:
    case Regression:
    case OneClass:
      nout = svm_predict( _svm, &svmvec[0] );
      break;
    case Probability:
      svm_predict_probability( _svm, &svmvec[0], &probs[0] );
      nout = probs[1];
      break;
    case Quality:
      nout = svm_predict_quality( _svm, &svmvec[0], dim,
        _qualityslope, _qualityshifted );
      break;
    case Decision:
      nout = svm_predict_decision( _svm, &svmvec[0] );
      break;
    }

  double  r = fabs( outp - nout );

  //  Ecriture
  if( _tstream )
    {
      //cout << "writing in TEST stream\n";
      for( unsigned i=0, n=_inputs.size(); i<n; ++i )
  *_tstream << _inputs[i] << "\t";
      *_tstream << nout << "\t" << outp;
    }

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

string SubAdSvm::getSvmMode(void) const
{
  static string  mode[] = {"classifier", "probability",
        "regression", "quality",
        "decision", "oneclass"};
  return mode[_svmmode];
}

void SubAdSvm::buildTree( Tree & tr ) const
{
  tr.setSyntax("sub_ad_svm");
  SubAdaptive::buildTree(tr);
  string  kernel_mode[] = {"linear", "poly", "rbf", "sigmoid"};
  tr.setProperty("net", _netFileName);
  tr.setProperty("netptr", _svm);
  tr.setProperty("min_out", (float) _minOut);
  tr.setProperty("max_out", (float) _maxOut);
  tr.setProperty("svm_mode", getSvmMode());
  tr.setProperty("kernel_type", kernel_mode[_svm_param.kernel_type]);
  tr.setProperty("quality_slope", _qualityslope);
  if (_svm_param.kernel_type == POLY)
    tr.setProperty("degree", (float) _svm_param.degree);
  if (_svm_param.kernel_type == POLY || _svm_param.kernel_type == SIGMOID)
    tr.setProperty("coef0", (float) _svm_param.coef0);
  if (_svm_param.kernel_type == POLY ||
    _svm_param.kernel_type == SIGMOID ||
    _svm_param.kernel_type == RBF)
    tr.setProperty("gamma", (float) _svm_param.gamma);
  if (_svm_param.svm_type == EPSILON_SVR ||
    _svm_param.svm_type == NU_SVR ||
                _svm_param.svm_type == C_SVC)
    tr.setProperty("C", (float) _svm_param.C);
  if (_svm_param.svm_type == NU_SVC || _svm_param.svm_type == NU_SVR ||
    _svm_param.svm_type == ONE_CLASS)
    tr.setProperty("nu", (float) _svm_param.nu);
  if (_svm_param.svm_type == EPSILON_SVR)
    tr.setProperty("epsilon", (float) _svm_param.p);
  tr.setProperty("quality_shifted_bad_output", (int) _qualityshifted);
}

string SubAdSvm::chooseFilename(const string & basename) const
{
  string      name = basename + _netFileName;
  string::size_type  pos = name.rfind('.');

  if(pos == string::npos)
    pos = name.size();
  name.replace(pos, name.size()-pos, ".dat");
  //  cout << "filename : " << name << endl;

  return name;
}

void  SubAdSvm::subFiles(const string & prefix, set<string> & listNames) const
{
  /* cout << "subFiles : " << prefix + FileUtil::separator() + fileNames() 
     << endl; */
  listNames.insert( prefix + FileUtil::separator() + fileNames() );
}
                                                                                
                                                                                
void  SubAdSvm::setBaseName(const string & basename)
{
  string          name = fileNames();
  string::size_type     pos = name.rfind('/');
                                                                                
  if(pos == string::npos)
  pos = 0;
  else  ++pos;
  name.insert(pos, basename + "_");
  setFileNames(name);
}
                                                                                
                                                                                
void SubAdSvm::learnStats(const vector<double> &vec, double outp)
{
  if(nStats() == 0 || outp < _minOut)
    _minOut = outp;
  if(nStats() == 0 || outp > _maxOut)
    _maxOut = outp;

  if(_maxOut < 1)     // hack pas propre pour stats sans mauvais ex.
    _maxOut = 1;

  SubAdaptive::learnStats(vec, outp);
}


#endif



