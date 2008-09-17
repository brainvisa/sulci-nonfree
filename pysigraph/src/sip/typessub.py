
typessub.update( {
    'sigraph::Adaptive *' :
    { 'typecode' : 'AdaptivePtr',
      'pyFromC' : 'pysigraphConvertFrom_sigraph_AdaptiveP',
      'CFromPy' : 'pysigraphConvertTo_sigraph_AdaptiveP',
      'castFromSip' : '(sigraph::Adaptive *)',
      'deref' : '',
      'pyderef' : '',
      'address' : '', 
      'pyaddress' : '', 
      'defScalar' : '',
      'new' : 'new sigraph::Adaptive *', 
      'NumType' : 'PyArray_OBJECT', 
      'PyType' : 'sigraph::Adaptive *',
      'sipClass' : 'sigraph_Adaptive*',
      'typeinclude' : '#include <si/model/adaptive.h>', 
      'sipinclude' : '#if SIP_VERSION < 0x040700\n'
        '#include "sipsigraphsipsigraphAdaptive.h"\n'
        '#endif\n'
        '#include <pysigraph/adaptive.h>', 
      'module' : 'sigraph', 
      'testPyType' : 'pysigraphAdaptiveP_Check', 
    },

    'sigraph::VectorLearnable *' :
    { 'typecode' : 'VectorLearnablePtr',
      'pyFromC' : 'pysigraphConvertFrom_sigraph_VectorLearnableP',
      'CFromPy' : 'pysigraphConvertTo_sigraph_VectorLearnableP',
      'castFromSip' : '(sigraph::VectorLearnable *)',
      'deref' : '',
      'pyderef' : '',
      'address' : '', 
      'pyaddress' : '', 
      'defScalar' : '',
      'new' : 'new sigraph::VectorLearnable *', 
      'NumType' : 'PyArray_OBJECT', 
      'PyType' : 'sigraph::VectorLearnable *',
      'sipClass' : 'sigraph_VectorLearnable*',
      'typeinclude' : '#include <si/learnable/vectorLearnable.h>', 
      'sipinclude' : '#if SIP_VERSION < 0x040700\n'
        '#include "sipsigraphsipsigraphVectorLearnable.h"\n'
        '#endif\n'
        '#include <pysigraph/learnable.h>', 
      'module' : 'sigraph', 
      'testPyType' : 'pysigraphVectorLearnableP_Check', 
    }, 

    'sigraph::Clique *' :
    { 'typecode' : 'CliquePtr',
      'pyFromC' : 'pysigraphConvertFrom_sigraph_CliqueP',
      'CFromPy' : 'pysigraphConvertTo_sigraph_CliqueP',
      'castFromSip' : '(sigraph::Clique *)',
      'deref' : '',
      'pyderef' : '',
      'address' : '', 
      'pyaddress' : '', 
      'defScalar' : '',
      'new' : 'new sigraph::Clique *', 
      'NumType' : 'PyArray_OBJECT', 
      'PyType' : 'sigraph::Clique *',
      'sipClass' : 'sigraph_Clique*',
      'typeinclude' : '#include <si/graph/clique.h>', 
      'sipinclude' : '#if SIP_VERSION < 0x040700\n'
        '#include "sipsigraphsipsigraphClique.h"\n'
        '#endif\n'
        '#include <pysigraph/clique.h>', 
      'module' : 'sigraph', 
      'testPyType' : 'pysigraphCliqueP_Check', 
    }, 

    } ),

completeTypesSub( typessub )
