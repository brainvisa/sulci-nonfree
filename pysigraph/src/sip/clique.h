
#ifndef PYSIGRAPH_CLIQUE_H
#define PYSIGRAPH_CLIQUE_H

#ifdef sipClass_sigraph_Clique
// avoid being included too early (sip 4.3 has this problem, 4.4 is OK)
#include <si/graph/clique.h>

inline PyObject* pysigraphConvertFrom_sigraph_CliqueP( void * a )
{
  return sipConvertFromInstance( a, sipClass_sigraph_Clique, 0 );
}


inline void* pysigraphConvertTo_sigraph_CliqueP( PyObject * o )
{
  int isErr = 0;
  return sipConvertToInstance( o, sipClass_sigraph_Clique, 0, 0, 0, 
                               &isErr );
}


inline int pysigraphCliqueP_Check( PyObject* o )
{
  return sipCanConvertToInstance( o, sipClass_sigraph_Clique, 
                                  SIP_NOT_NONE | SIP_NO_CONVERTORS );
}


inline PyObject* pysigraphConvertFrom_rc_ptr_Clique( const void * a )
{
  return sipConvertFromInstance( const_cast<void *>( a ),
                                 sipClass_rc_ptr_Clique, 0 );
}


inline carto::rc_ptr<sigraph::Clique>* pysigraphConvertTo_rc_ptr_Clique( PyObject * o )
{
  int isErr = 0;
  return static_cast<carto::rc_ptr<sigraph::Clique>*>( sipConvertToInstance( o,
      sipClass_rc_ptr_Clique, 0, 0, 0, &isErr ) );
}


inline int pysigraphRcptrClique_Check( PyObject* o )
{
  return sipCanConvertToInstance( o, sipClass_rc_ptr_Clique,
                                  SIP_NOT_NONE | SIP_NO_CONVERTORS );
}

#endif
#endif

