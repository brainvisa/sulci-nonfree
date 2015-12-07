cmake_minimum_required( VERSION 2.6 )
find_package( brainvisa-cmake REQUIRED )
BRAINVISA_PROJECT()

BRAINVISA_FIND_PACKAGE( aims-free REQUIRED )
include( "${AIMS-FREE_USE_FILE}" )

BRAINVISA_FIND_PACKAGE( datamind )
find_package( SVM )

find_package( Doxygen )

set( SIGRAPH_RELATIVE_SIP_DIRECTORY
     "share/sulci-private-${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}/sip" )

add_subdirectory( neuron )
add_subdirectory( sigraph )

if( AIMS_PYAIMS_LIBRARY )
  add_subdirectory( pysigraph )
else()
  message( "WARNING: pysigraph will not be built due to a missing dependency from aims-free: AIMS_PYAIMS_LIBRARY=\"${AIMS_PYAIMS_LIBRARY}\"" )
endif()

BRAINVISA_FIND_PACKAGE( anatomist-free )
if( anatomist-free_FOUND )
    include( "${ANATOMIST-FREE_USE_FILE}" )
    add_subdirectory( anafold )
endif()

BRAINVISA_DEPENDENCY( RUN DEPENDS "aims-free" RUN ">= ${aims-free_VERSION};<< ${soma-base_VERSION_MAJOR}.${result}" )
BRAINVISA_DEPENDENCY( DEV DEPENDS "${PROJECT_NAME}" RUN "== ${${PROJECT_NAME}_VERSION}" )
BRAINVISA_DEPENDENCY( DEV DEPENDS "aims-free" DEV ">= ${aims-free_VERSION};<< ${soma-base_VERSION_MAJOR}.${result}" )
BRAINVISA_DEPENDENCY( RUN DEPENDS "libsvm2" RUN )
BRAINVISA_DEPENDENCY( RUN RECOMMENDS "sulci-models" RUN )

BRAINVISA_CREATE_CMAKE_CONFIG_FILES()

