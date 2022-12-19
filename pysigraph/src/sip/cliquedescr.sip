
%ModuleCode
#include <si/fold/foldDescr3.h>
#include <si/fold/foldDescr5.h>
#include <si/fold/interFoldDescr5.h>
%End

namespace sigraph
{

class CliqueDescr /Abstract/
{
%TypeHeaderCode
#include <si/descr/cliqueDescr.h>
#ifndef PYAIMSSIP_VECTOR_STRING_DEFINED
#define PYAIMSSIP_VECTOR_STRING_DEFINED
#if SIP_VERSION < 0x040700
#include "sipaimssipvector_STRING.h"
#include "sipaimssipvector_DOUBLE.h"
#endif
typedef std::vector<std::string> vector_STRING;
#endif
%End

%ConvertToSubClassCode
  if( dynamic_cast<sigraph::AdapDescr *>( sipCpp ) )
    if( dynamic_cast<sigraph::FoldDescr5 *>( sipCpp ) )
    {
      sipType = sipType_sigraph_FoldDescr5;
      *sipCppRet = dynamic_cast<sigraph::FoldDescr5 *>( sipCpp );
    }
    else if( dynamic_cast<sigraph::FoldDescr4 *>( sipCpp ) )
    {
      sipType = sipType_sigraph_FoldDescr4;
      *sipCppRet = dynamic_cast<sigraph::FoldDescr4 *>( sipCpp );
    }
    else if( dynamic_cast<sigraph::FoldDescr3 *>( sipCpp ) )
    {
      sipType = sipType_sigraph_FoldDescr3;
      *sipCppRet = dynamic_cast<sigraph::FoldDescr3 *>( sipCpp );
    }
    else if( dynamic_cast<sigraph::FoldDescr2 *>( sipCpp ) )
    {
      sipType = sipType_sigraph_FoldDescr2;
      *sipCppRet = dynamic_cast<sigraph::FoldDescr2 *>( sipCpp );
    }
    else
    {
      sipType = sipType_sigraph_AdapDescr;
      *sipCppRet = dynamic_cast<sigraph::AdapDescr *>( sipCpp );
    }
  else
  {
    sipType = sipType_sigraph_CliqueDescr;
    *sipCppRet = dynamic_cast<sigraph::CliqueDescr *>( sipCpp );
  }
%End

public:
  virtual ~CliqueDescr();
  virtual sigraph::CliqueDescr *clone() const = 0;
  virtual vector_STRING descriptorsNames() const;
  virtual bool makeVector(const sigraph::Clique* cl, vector_DOUBLE & vec,
		carto::GenericObject* model = 0);
  virtual void preProcess(vector_DOUBLE &vec, carto::GenericObject* model = 0);
};


class AdapDescr : sigraph::CliqueDescr /Abstract/
{
%TypeHeaderCode
#include <si/descr/adapDescr.h>
#if SIP_VERSION < 0x040700
#include "sipaimssipvector_STRING.h"
#include "sipaimssipvector_DOUBLE.h"
#endif
%End

public:
  virtual ~AdapDescr();

  virtual sigraph::SiDBLearnable & getSiDBLearnable() = 0;
  virtual void clearDB();

protected:
  AdapDescr();
};


class FoldDescr2 : sigraph::AdapDescr
{
%TypeHeaderCode
#include <si/fold/foldDescr2.h>
#if SIP_VERSION < 0x040700
#include "sipaimssipvector_STRING.h"
#include "sipaimssipvector_DOUBLE.h"
#endif
%End

public:
  FoldDescr2();
  virtual ~FoldDescr2();
};


class FoldDescr3 : sigraph::AdapDescr
{
%TypeHeaderCode
#include <si/fold/foldDescr3.h>
#if SIP_VERSION < 0x040700
#include "sipaimssipvector_STRING.h"
#include "sipaimssipvector_DOUBLE.h"
#endif
%End

public:
  FoldDescr3();
  virtual ~FoldDescr3();
};


class FoldDescr4 : sigraph::AdapDescr
{
%TypeHeaderCode
#include <si/fold/foldDescr4.h>
#if SIP_VERSION < 0x040700
#include "sipaimssipvector_STRING.h"
#include "sipaimssipvector_DOUBLE.h"
#endif
%End

public:
  FoldDescr4();
  virtual ~FoldDescr4();
};


class FoldDescr5 : sigraph::AdapDescr
{
%TypeHeaderCode
#include <si/fold/foldDescr5.h>
#if SIP_VERSION < 0x040700
#include "sipaimssipvector_STRING.h"
#include "sipaimssipvector_DOUBLE.h"
#endif
%End

public:
  FoldDescr5();
  virtual ~FoldDescr5();
};

};

