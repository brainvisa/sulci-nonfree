
#ifndef PYSIGRAPH_CLIQUE_H
#define PYSIGRAPH_CLIQUE_H

#ifdef sipType_sigraph_Clique
// avoid being included too early (sip 4.3 has this problem, 4.4 is OK)
#include <si/graph/clique.h>

inline PyObject* pysigraphConvertFrom_sigraph_CliqueP( void * a )
{
  return sipConvertFromType( a, sipType_sigraph_Clique, 0 );
}


inline void* pysigraphConvertTo_sigraph_CliqueP( PyObject * o )
{
  int isErr = 0;
  return sipConvertToType( o, sipType_sigraph_Clique, 0, 0, 0,
                           &isErr );
}


inline int pysigraphCliqueP_Check( PyObject* o )
{
  return sipCanConvertToType( o, sipType_sigraph_Clique,
                              SIP_NOT_NONE | SIP_NO_CONVERTORS );
}


inline PyObject* pysigraphConvertFrom_rc_ptr_Clique( const void * a )
{
  return sipConvertFromType( const_cast<void *>( a ),
                             sipType_rc_ptr_Clique, 0 );
}


inline carto::rc_ptr<sigraph::Clique>* pysigraphConvertTo_rc_ptr_Clique( PyObject * o )
{
  int isErr = 0;
  return static_cast<carto::rc_ptr<sigraph::Clique>*>( sipConvertToType( o,
      sipType_rc_ptr_Clique, 0, 0, 0, &isErr ) );
}


inline int pysigraphRcptrClique_Check( PyObject* o )
{
  return sipCanConvertToType( o, sipType_rc_ptr_Clique,
                              SIP_NOT_NONE | SIP_NO_CONVERTORS );
}

#endif
#endif

