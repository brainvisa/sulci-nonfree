
%ModuleCode
#if SIP_VERSION >= 0x040700
#include <si/subadaptive/subAdMlp.h>
#include <si/subadaptive/subAdGauss.h>
#include <si/subadaptive/subAdLogGauss.h>
#include <si/subadaptive/subAdMixGauss.h>
#endif
%End

namespace sigraph
{

class SubAdResponse
{
%TypeHeaderCode
#include <si/subadaptive/subAdResponse.h>
%End

%ConvertToSubClassCode
  if (dynamic_cast<sigraph::SubAdResponse *>(sipCpp))
  {
    sipClass = sipClass_sigraph_SubAdResponse;
    *sipCppRet = dynamic_cast<sigraph::SubAdResponse *>( sipCpp );
  }
%End

        public:
	virtual ~SubAdResponse();

        vector_DOUBLE     	*true_values;
        vector_DOUBLE     	*predict_values;
};


class SubAdaptive /Abstract/
{
%TypeHeaderCode
#include <si/subadaptive/subAdaptive.h>
#if SIP_VERSION < 0x040700
#include "sipaimssipvector_DOUBLE.h"
#endif
%End


%ConvertToSubClassCode
  if (dynamic_cast<sigraph::SubAdMlp *>(sipCpp))
  {
    sipClass = sipClass_sigraph_SubAdMlp;
    *sipCppRet = dynamic_cast<sigraph::SubAdMlp *>( sipCpp );
  }
  else if (dynamic_cast<sigraph::SubAdGauss *>(sipCpp))
  {
    if (dynamic_cast<sigraph::SubAdLogGauss *>(sipCpp))
    {
      sipClass = sipClass_sigraph_SubAdLogGauss;
      *sipCppRet = dynamic_cast<sigraph::SubAdLogGauss *>( sipCpp );
    }
    else
    {
      sipClass = sipClass_sigraph_SubAdGauss;
      *sipCppRet = dynamic_cast<sigraph::SubAdGauss *>( sipCpp );
    }
  }
  else if (dynamic_cast<sigraph::SubAdMixGauss *>(sipCpp))
  {
    sipClass = sipClass_sigraph_SubAdMixGauss;
    *sipCppRet = dynamic_cast<sigraph::SubAdMixGauss *>( sipCpp );
  }
  else
  {
    sipClass = sipClass_sigraph_SubAdaptive;
    *sipCppRet = dynamic_cast<sigraph::SubAdaptive *>( sipCpp );
  }
%End


public:
  virtual ~SubAdaptive();

  std::string name() const;
  virtual sigraph::SubAdResponse *train(sigraph::AdaptiveLeaf &,
	const sigraph::SiDBLearnable &,
	const sigraph::SiDBLearnable &) = 0 /Factory/;
  virtual sigraph::SubAdResponse *test(
	const sigraph::SiDBLearnable &) = 0 /Factory/;

  virtual sigraph::SubAdaptive* clone() const = 0 /Factory/;
  virtual void init() = 0;
  virtual std::string fileNames() const = 0;
  virtual void setStats(const vector_DOUBLE&, const vector_DOUBLE&);
  virtual double relianceWeight() const;

  double errorRate() const;
  double genErrorRate() const;
  double genGoodErrorRate() const;
  double genMeanErrorRate() const;
  double genBadErrorRate() const; 

  double getMean(unsigned ind);
  double getSigma(unsigned ind);

  void setGenGoodErrorRate(double);
  void setGenBadErrorRate(double);
  void setGenErrorRate(double);
  void setMisclassGoodRate(double);
  void setMisclassBadRate(double);

private:
  SubAdaptive( const sigraph::SubAdaptive & );
};

class IncrementalSubAdaptive : sigraph::SubAdaptive /Abstract/
{
%TypeHeaderCode
#include <si/subadaptive/incrementalSubAdaptive.h>
#if SIP_VERSION < 0x040700
#include "sipaimssipvector_DOUBLE.h"
#endif
%End

public:
  virtual ~IncrementalSubAdaptive();
  virtual double learn(sigraph::AdaptiveLeaf &,
	const sigraph::SiDBLearnable &, const sigraph::SiDBLearnable &) = 0;
};


class SubAdMlp : sigraph::IncrementalSubAdaptive
{
%TypeHeaderCode
#include <si/subadaptive/subAdMlp.h>
#if SIP_VERSION < 0x040700
#include "sipaimssipvector_DOUBLE.h"
#endif
%End

public:
  virtual ~SubAdMlp();
};


class SubAdGauss : sigraph::IncrementalSubAdaptive
{
%TypeHeaderCode
#include <si/subadaptive/subAdGauss.h>
#if SIP_VERSION < 0x040700
#include "sipaimssipvector_DOUBLE.h"
#endif
%End

public:
  virtual ~SubAdGauss();
  GaussNet & net();
  double defaultValue() const;
  void setDefaultValue(double);
};

class SubAdLogGauss : sigraph::SubAdGauss
{
%TypeHeaderCode
#include <si/subadaptive/subAdLogGauss.h>
#if SIP_VERSION < 0x040700
#include "sipaimssipvector_DOUBLE.h"
#endif
%End

public:
  virtual ~SubAdLogGauss();
};

class NonIncrementalSubAdaptive : sigraph::SubAdaptive /Abstract/
{
%TypeHeaderCode
#include <si/subadaptive/nonIncrementalSubAdaptive.h>
%End

public:
  NonIncrementalSubAdaptive();
  ~NonIncrementalSubAdaptive();
  virtual double learn(const sigraph::SiDBLearnable &) = 0;
};


class SubAdMixGauss : sigraph::NonIncrementalSubAdaptive
{
%TypeHeaderCode
#include <si/subadaptive/subAdMixGauss.h>
#if SIP_VERSION < 0x040700
#include "sipaimssipvector_DOUBLE.h"
#include "sipaimssipvector_FLOAT.h"
#endif
%End

public:
   SubAdMixGauss();
   virtual ~SubAdMixGauss();

   virtual void addMatrix(const vector_FLOAT &);
   virtual void addSqrtDet(double);
   virtual void reset();
};


};


class Gaussian
{
%TypeHeaderCode
#include <neur/gauss/gaussian.h>
%End

public:
  virtual ~Gaussian();
  void init(unsigned, bool);
  void setCenterCoord(unsigned, double);
};



class GaussNet
{
%TypeHeaderCode
#include <neur/gauss/gaussnet.h>
%End

public:
  virtual ~GaussNet();
  virtual void init(unsigned, unsigned, bool = true );
  Gaussian *gauss(unsigned);
  double weight( unsigned) const;
  void setWeight( unsigned, double);
};



