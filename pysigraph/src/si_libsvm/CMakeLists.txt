set( _headers )
set( _sources
     si_libsvm.c )

BRAINVISA_COPY_AND_INSTALL_HEADERS( _headers si/sisvm headersTarget )

ADD_LIBRARY(_si_libsvm SHARED ${_sources} )

set_property( TARGET _si_libsvm PROPERTY LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/python/sigraph )
set_property( TARGET _si_libsvm PROPERTY PREFIX "" )
if( PYTHON_FLAGS )
    set_target_properties( _si_libsvm PROPERTIES COMPILE_DEFINITIONS ${PYTHON_FLAGS} )
endif()

include_directories( "${CMAKE_BINARY_DIR}/include" "${PYTHON_INCLUDE_PATH}")
target_link_libraries( _si_libsvm "${PYTHON_LIBRARY}" )

BRAINVISA_INSTALL( TARGETS _si_libsvm DESTINATION python/sigraph COMPONENT ${PROJECT_NAME} )
