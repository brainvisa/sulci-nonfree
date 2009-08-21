
#ifndef PYSIGRAPH_LEARNABLE_H
#define PYSIGRAPH_LEARNABLE_H

#include <si/learnable/vectorLearnable.h>

inline PyObject* pysigraphConvertFrom_sigraph_VectorLearnableP( void * a )
{
  return sipConvertFromInstance( a, sipClass_sigraph_VectorLearnable, 0 );
}


inline void* pysigraphConvertTo_sigraph_VectorLearnableP( PyObject * o )
{
  int isErr = 0;
  return sipConvertToInstance( o, sipClass_sigraph_VectorLearnable, 0, 0, 0, 
                               &isErr );
}


inline int pysigraphVectorLearnableP_Check( PyObject* o )
{
  return sipCanConvertToInstance( o, sipClass_sigraph_VectorLearnable,
                                  SIP_NOT_NONE | SIP_NO_CONVERTORS );
}

#endif

