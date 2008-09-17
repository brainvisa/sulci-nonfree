
#ifndef PYSIGRAPH_LEARNABLE_H
#define PYSIGRAPH_LEARNABLE_H

#include <si/learnable/vectorLearnable.h>

inline PyObject* pysigraphConvertFrom_sigraph_VectorLearnableP( void * a )
{
#if SIP_VERSION >= 0x040400
  return sipConvertFromInstance( a, sipClass_sigraph_VectorLearnable, 0 );
#else
  return sipMapCppToSelfSubClass( a, sipClass_sigraph_VectorLearnable );
#endif
}


inline void* pysigraphConvertTo_sigraph_VectorLearnableP( PyObject * o )
{
  int isErr = 0;
#if SIP_VERSION >= 0x040400
  return sipConvertToInstance( o, sipClass_sigraph_VectorLearnable, 0, 0, 0, 
                               &isErr );
#else
  return sipForceConvertTo_sigraph_VectorLearnable( o, &isErr );
#endif
}


inline int pysigraphVectorLearnableP_Check( PyObject* o )
{
  return sipIsSubClassInstance( o, sipClass_sigraph_VectorLearnable );
}

#endif

