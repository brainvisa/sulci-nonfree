
%ModuleCode
#if SIP_VERSION >= 0x040700
#include <si/subadaptive/subadsvm.h>
#endif
%End


/* namespace sigraph
{ */

class SubAdSvm : sigraph::NonIncrementalSubAdaptive
{
%TypeHeaderCode
#include <si/subadaptive/subadsvm.h>
#if SIP_VERSION < 0x040700
#include "sipaimssipvector_DOUBLE.h"
#endif
// SIP doesn't allow using the same namespace (sigraph) as in another module
typedef sigraph::SubAdSvm SubAdSvm;
%End

%ConvertToSubClassCode
  if (dynamic_cast<sigraph::SubAdSvm *>(sipCpp))
	sipClass = sipClass_SubAdSvm;
  else
    sipClass = NULL;
%End

public:
  SubAdSvm( const std::string &, const std::string &, const std::string & );
  SubAdSvm( const SubAdSvm & );
  virtual ~SubAdSvm();

  sigraph::SubAdResponse *crossvalidation(const sigraph::SiDBLearnable &,
	const unsigned int) /Factory/;
  std::string getSvmMode() const;
  void setSvmWeight(int, double);
  void setGamma(double);
  void setC(double);
  void setEpsilon(double);
  void setNu(double);
  double getGamma();
  double getC();
  double getEpsilon();
  double getNu();

};

// };

