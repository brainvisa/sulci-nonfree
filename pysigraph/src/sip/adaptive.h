
#ifndef PYSIGRAPH_ADAPTIVE_H
#define PYSIGRAPH_ADAPTIVE_H

#include <si/model/adaptive.h>

inline PyObject* pysigraphConvertFrom_sigraph_AdaptiveP( void * a )
{
  return sipConvertFromType( a, sipType_sigraph_Adaptive, 0 );
}


inline void* pysigraphConvertTo_sigraph_AdaptiveP( PyObject * o )
{
  int isErr = 0;
  return sipConvertToType( o, sipType_sigraph_Adaptive, 0, 0, 0, &isErr );
}


inline int pysigraphAdaptiveP_Check( PyObject* o )
{
  return sipCanConvertToType( o, sipType_sigraph_Adaptive,
                              SIP_NOT_NONE | SIP_NO_CONVERTORS );
}

#endif


