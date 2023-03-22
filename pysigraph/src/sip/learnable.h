
#ifndef PYSIGRAPH_LEARNABLE_H
#define PYSIGRAPH_LEARNABLE_H

#include <si/learnable/vectorLearnable.h>

inline PyObject* pysigraphConvertFrom_sigraph_VectorLearnableP( void * a )
{
  return sipConvertFromType( a, sipType_sigraph_VectorLearnable, 0 );
}


inline void* pysigraphConvertTo_sigraph_VectorLearnableP( PyObject * o )
{
  int isErr = 0;
  return sipConvertToType( o, sipType_sigraph_VectorLearnable, 0, 0, 0,
                           &isErr );
}


inline int pysigraphVectorLearnableP_Check( PyObject* o )
{
  return sipCanConvertToType( o, sipType_sigraph_VectorLearnable,
                              SIP_NOT_NONE | SIP_NO_CONVERTORS );
}

#endif

