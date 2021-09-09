cmake_minimum_required( VERSION 2.6 )
find_package( brainvisa-cmake REQUIRED )
BRAINVISA_PROJECT()

BRAINVISA_FIND_PACKAGE( aims-free REQUIRED )
BRAINVISA_FIND_PACKAGE( brainvisa-share QUIET )

include( "${AIMS-FREE_USE_FILE}" )

find_package( SVM )
find_package( SWIG )

find_package( Doxygen )

set( SIGRAPH_RELATIVE_SIP_DIRECTORY
     "share/sulci-nonfree-${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}/sip" )

#set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W -Wall" )
#set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall" )

add_subdirectory( neuron )

# don't warn on obsolete classes within this library: we still provide obsolete
# features so we have to build them without warnings
add_definitions( "-DAIMSDATA_CLASS_NO_DEPREC_WARNING=1" )

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

math( EXPR result "${aims-free_VERSION_MINOR}+1" )
BRAINVISA_DEPENDENCY( RUN DEPENDS "aims-free" RUN ">= ${aims-free_VERSION};<< ${aims-free_VERSION_MAJOR}.${result}" )
BRAINVISA_DEPENDENCY( DEV DEPENDS "${PROJECT_NAME}" RUN "= ${${PROJECT_NAME}_VERSION}" )
BRAINVISA_DEPENDENCY( DEV DEPENDS "aims-free" DEV ">= ${aims-free_VERSION};<< ${aims-free_VERSION_MAJOR}.${result}" )
if( SVM_FOUND )
  BRAINVISA_DEPENDENCY( RUN DEPENDS "libsvm2" RUN )
endif()

# add install rules for zipped packages

set( SULCI_MODELS_DOWNLOAD_SOURCE "ftp:///ftp.cea.fr/pub/dsv/anatomist/data" CACHE FILEPATH "URL for sulci models data download" )

set( version_maj ${BRAINVISA_PACKAGE_VERSION_MAJOR} )
set( version_min ${BRAINVISA_PACKAGE_VERSION_MINOR} )
if( brainvisa_share_FOUND )
  set( version_maj ${brainvisa-share_VERSION_MAJOR} )
  set( version_min ${brainvisa-share_VERSION_MINOR} )
endif()

set( get_models_cmd
  "${CMAKE_CURRENT_SOURCE_DIR}/pysigraph/scripts/sigraph/bv_get_sulci_models.py" )
add_custom_target( install-sulci-models-data
  COMMAND "${PYTHON_HOST_EXECUTABLE}" "${get_models_cmd}" "-o" "$(BRAINVISA_INSTALL_PREFIX)/share/brainvisa-share-${version_maj}.${version_min}" "-i" "${SULCI_MODELS_DOWNLOAD_SOURCE}" "--no-update"
  )

add_dependencies( install-sulci-nonfree install-sulci-models-data )

BRAINVISA_CREATE_CMAKE_CONFIG_FILES()

