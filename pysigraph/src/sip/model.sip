
%ModuleCode
#if SIP_VERSION >= 0x040700
#include <si/model/adaptiveTree.h>
#include <si/model/adaptiveLeaf.h>
#include <si/model/topAdaptive.h>
#endif
%End

namespace sigraph
{

class Model /Abstract/
{
%TypeHeaderCode
#include <si/model/model.h>
%End

%ConvertToSubClassCode
  // std::cout << "ConvertToSubClassCode Model\n";
  if( dynamic_cast<sigraph::Adaptive *>( sipCpp ) )
  {
    if( dynamic_cast<sigraph::AdaptiveTree *>( sipCpp ) )
    {
      sipClass = sipClass_sigraph_AdaptiveTree;
      *sipCppRet = dynamic_cast<sigraph::AdaptiveTree *>( sipCpp );
    }
    else if( dynamic_cast<sigraph::AdaptiveLeaf *>( sipCpp ) )
    {
      sipClass = sipClass_sigraph_AdaptiveLeaf;
      *sipCppRet = dynamic_cast<sigraph::AdaptiveLeaf *>( sipCpp );
    }
    else if( dynamic_cast<sigraph::TopAdaptive *>( sipCpp ) )
    {
      sipClass = sipClass_sigraph_TopAdaptive;
      *sipCppRet = dynamic_cast<sigraph::TopAdaptive *>( sipCpp );
    }
    else
    {
      sipClass = sipClass_sigraph_Adaptive;
      *sipCppRet = dynamic_cast<sigraph::Adaptive *>( sipCpp );
    }
  }
  else
  {
    sipClass = sipClass_sigraph_Model;
    *sipCppRet = dynamic_cast<sigraph::Model *>( sipCpp );
  }
%End

public:
  virtual ~Model();
  virtual sigraph::Model* clone() const = 0 /Factory/;
  virtual double prop( const sigraph::Clique *) = 0 /AutoGen/;
  virtual bool isAdaptive() const;

  virtual void setParent(sigraph::Model* m);
  virtual sigraph::Model *parent() = 0;
  virtual sigraph::TopModel *topModel() = 0;
  virtual double printDescription( sigraph::Clique*, bool = false );
  // virtual const sigraph::TopModel *topModel() const = 0;
  carto::GenericObject *graphObject();
%MethodCode
  sipRes = sipCpp->graphObject();
%End
  virtual void setBaseName( const std::string & basename );


  static sigraph::Model* fromObject( carto::GenericObject );
%MethodCode
  try
  {
    sipRes = a0->value<sigraph::Model *>();
  }
  catch( std::exception & e )
  {
    sipIsErr = 1;
    PyErr_SetString( PyExc_RuntimeError, e.what() );
    sipRes = 0;
  }
%End

  // conversion to Object
  carto::Object toObject();
%MethodCode
  sipRes = new carto::Object( carto::Object::value( sipCpp  ) );
%End

  // conversion to Object
  carto::Object ptrToObject();
%MethodCode
  sipRes = new carto::Object( carto::Object::value( sipCpp  ) );
%End


};


class Adaptive : sigraph::Model /Abstract/
{
%TypeHeaderCode
#include <si/model/adaptive.h>
%End

public:
  virtual ~Adaptive();
  virtual bool isAdaptive() const;
};


class AdaptiveTree : sigraph::Adaptive
{
%TypeHeaderCode
#include <si/model/adaptiveTree.h>
#include <si/dimreductor/dimreductor.h>
%End

public:
  virtual ~AdaptiveTree();

  typedef set_AdaptivePtr datatype;
  const sigraph::AdaptiveTree::datatype & children() const;
};


class AdaptiveLeaf : sigraph::Adaptive
{
%TypeHeaderCode
#include <si/model/adaptiveLeaf.h>
#include <si/optimizer/optimizer.h>
#if SIP_VERSION < 0x040700
#include "sipaimssipvector_FLOAT.h"
#endif
%End

public:
  virtual ~AdaptiveLeaf();

  sigraph::SubAdaptive & workEl() const;
  void setWorkEl(sigraph::SubAdaptive * /Transfer/);
  sigraph::SubAdaptive & evalEl();
  // const sigraph::SubAdaptive & evalEl() const;
  sigraph::SubAdaptive *workMemoEl() const;
  sigraph::AdapDescr* getAdapDescr();
  sigraph::CliqueDescr & cliqueDescr();
  // const sigraph::CliqueDescr & cliqueDescr() const;
  void setCliqueDescr(sigraph::CliqueDescr * /Transfer/);
  void update(sigraph::AdaptiveLeaf &);
  void init();

  virtual std::string getDataBaseName(const std::string &);
  const sigraph::DimReductor* dimreductor() const;
  const sigraph::Optimizer& optimizer() const;
  void setOptimizer(sigraph::Optimizer * /Transfer/);
  void setDimReductor(sigraph::DimReductor * /Transfer/);
  void setMean(vector_FLOAT&);
  void setStd(vector_FLOAT&);
  const vector_FLOAT &getMean() const;
  const vector_FLOAT &getStd() const;
};


class TopModel
{
%TypeHeaderCode
#include <si/model/topModel.h>
%End

public:
  virtual ~TopModel();
  set_STRING & significantLabels();
  const std::string & voidLabel() const;
  void setVoidLabel( const std::string & );
  carto::GenericObject * parentAO();
%MethodCode
  sipRes = sipCpp->parentAO();
%End
  double weight() const;
  void setWeight( double );
  virtual double confidenceFactor() const;

private:
  TopModel( const sigraph::TopModel & );
};


class TopAdaptive : sigraph::Adaptive, sigraph::TopModel
{
%TypeHeaderCode
#include <si/model/topAdaptive.h>
%End

public:
  virtual ~TopAdaptive();
  sigraph::Model* model();
};


class LearnStopCriterion
{
%TypeHeaderCode
#include <si/subadaptive/stopCriterion.h>
%End

public:
  LearnStopCriterion();
  virtual ~LearnStopCriterion();
  // virtual bool stops( const sigraph::SubAdaptive &, unsigned ) const;
  // virtual bool stoppable( const sigraph::SubAdaptive &, unsigned ) const;

  double MaxAppError;
  unsigned StopDelay;

  static sigraph::LearnStopCriterion *theCriterion;
};

class MReader
{
%TypeHeaderCode
#include <si/model/mReader.h>
%End

public:
  MReader(const std::string &);
%MethodCode
  sipCpp = new sipsigraph_MReader(*a0);
%End
private:
  MReader(const sigraph::MReader &);

public:
  virtual ~MReader();
  virtual sigraph::Model* readModel();
};


class MWriter
{
%TypeHeaderCode
#include <si/model/mWriter.h>
%End

public:
  MWriter(const std::string &, bool=false);
  virtual ~MWriter();

private:
  MWriter(const sigraph::MWriter &);

public:
  virtual void write(const sigraph::Model &);
%MethodCode
  try
  {
    sipCpp->write(*a0);
  }
  catch( std::exception & e )
  {
    sipIsErr = 1;
    PyErr_SetString( PyExc_RuntimeError, e.what() );
  }
%End
};

};

