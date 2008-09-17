
#ifndef PYSIGRAPH_CLIQUE_H
#define PYSIGRAPH_CLIQUE_H

#ifdef sipClass_sigraph_Clique
// avoid being included too early (sip 4.3 has this problem, 4.4 is OK)
#include <si/graph/clique.h>

inline PyObject* pysigraphConvertFrom_sigraph_CliqueP( void * a )
{
#if SIP_VERSION >= 0x040400
  return sipConvertFromInstance( a, sipClass_sigraph_Clique, 0 );
#else
  return sipMapCppToSelfSubClass( a, sipClass_sigraph_Clique );
#endif
}


inline void* pysigraphConvertTo_sigraph_CliqueP( PyObject * o )
{
  int isErr = 0;
#if SIP_VERSION >= 0x040400
  return sipConvertToInstance( o, sipClass_sigraph_Clique, 0, 0, 0, 
                               &isErr );
#else
  return sipForceConvertTo_sigraph_Clique( o, &isErr );
#endif
}


inline int pysigraphCliqueP_Check( PyObject* o )
{
  return sipIsSubClassInstance( o, sipClass_sigraph_Clique );
}

#endif
#endif

