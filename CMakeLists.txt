cmake_minimum_required( VERSION 2.6 )
find_package( brainvisa-cmake REQUIRED )
BRAINVISA_PROJECT()

BRAINVISA_FIND_PACKAGE( aims-free REQUIRED )
include( "${AIMS-FREE_USE_FILE}" )

BRAINVISA_FIND_PACKAGE( datamind )
find_package( SVM )

add_subdirectory( neuron )
add_subdirectory( sigraph )

if( AIMS-PYAIMS_LIB )
  add_subdirectory( pysigraph )
endif()


BRAINVISA_CREATE_CMAKE_CONFIG_FILES()

