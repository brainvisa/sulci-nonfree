BRAINVISA_GET_FILE_LIST_FROM_PRO(  ${CMAKE_CURRENT_SOURCE_DIR}/sisvm.pro "HEADERS" _headers "SOURCES" _sources )
BRAINVISA_COPY_AND_INSTALL_HEADERS( _headers si headersTarget )
set(LIBSIGRAPHSVM_HEADERS ${_headers} PARENT_SCOPE)
include_directories( "${SVM_INCLUDE_DIRS}")
add_library( sigraphsvm SHARED ${_sources} )
add_dependencies( sigraphsvm ${headersTarget} )
set_property( TARGET sigraphsvm PROPERTY VERSION ${${PROJECT_NAME}_VERSION} )
target_link_libraries( sigraphsvm sigraph sigraph_libsvm )
BRAINVISA_INSTALL( TARGETS sigraphsvm
                   DESTINATION lib
                   COMPONENT ${PROJECT_NAME} )

