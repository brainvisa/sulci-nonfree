cmake_minimum_required( VERSION 2.6 )
find_package( brainvisa-cmake REQUIRED )
BRAINVISA_PROJECT()

BRAINVISA_FIND_PACKAGE( aims-free REQUIRED )
include( "${AIMS-FREE_USE_FILE}" )

BRAINVISA_FIND_PACKAGE( datamind )
find_package( SVM )

find_package( Doxygen )

set( SIGRAPH_RELATIVE_SIP_DIRECTORY
     "share/sulci-${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}/sip" )

add_subdirectory( neuron )
add_subdirectory( sigraph )

if( AIMS_PYAIMS_LIBRARY )
  add_subdirectory( pysigraph )
else()
  message( "WARNING: pysigraph will not be build due to a missing dependency from aims-free: AIMS_PYAIMS_LIBRARY=\"${AIMS_PYAIMS_LIBRARY}\"" )
endif()


BRAINVISA_CREATE_CMAKE_CONFIG_FILES()

