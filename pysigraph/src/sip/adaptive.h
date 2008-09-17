
#ifndef PYSIGRAPH_ADAPTIVE_H
#define PYSIGRAPH_ADAPTIVE_H

#include <si/model/adaptive.h>

inline PyObject* pysigraphConvertFrom_sigraph_AdaptiveP( void * a )
{
#if SIP_VERSION >= 0x040400
  return sipConvertFromInstance( a, sipClass_sigraph_Adaptive, 0 );
#else
  return sipMapCppToSelfSubClass( a, sipClass_sigraph_Adaptive );
#endif
}


inline void* pysigraphConvertTo_sigraph_AdaptiveP( PyObject * o )
{
  int isErr = 0;
#if SIP_VERSION >= 0x040400
  return sipConvertToInstance( o, sipClass_sigraph_Adaptive, 0, 0, 0, &isErr );
#else
  return sipForceConvertTo_sigraph_Adaptive( o, &isErr );
#endif
}


inline int pysigraphAdaptiveP_Check( PyObject* o )
{
  return sipIsSubClassInstance( o, sipClass_sigraph_Adaptive );
}

#endif


