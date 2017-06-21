set( _headers
     libsvm.h )
set( _sources
     libsvm.cc )

BRAINVISA_COPY_AND_INSTALL_HEADERS( _headers si/sisvm headersTarget )
set(SILIBSVM_HEADERS "si/sisvm/libsvm.h" PARENT_SCOPE)
ADD_LIBRARY(sigraph_libsvm SHARED ${_sources} )
add_dependencies( sigraph_libsvm ${headersTarget} )
set_property( TARGET sigraph_libsvm PROPERTY VERSION ${${PROJECT_NAME}_VERSION} )

include_directories( "${CMAKE_BINARY_DIR}/include" "${SVM_INCLUDE_DIRS}")

target_link_libraries( sigraph_libsvm ${SVM_LIBRARIES} )
BRAINVISA_INSTALL( TARGETS sigraph_libsvm DESTINATION lib COMPONENT
                   ${PROJECT_NAME} )
