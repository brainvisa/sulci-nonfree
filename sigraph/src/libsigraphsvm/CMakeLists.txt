BRAINVISA_GET_FILE_LIST_FROM_PRO(  ${CMAKE_CURRENT_SOURCE_DIR}/sisvm.pro "HEADERS" _headers "SOURCES" _sources )
BRAINVISA_COPY_AND_INSTALL_HEADERS( _headers si headersTarget )
add_library( sisvm SHARED ${_sources} )
add_dependencies( sisvm ${headersTarget} )
set_property( TARGET sisvm PROPERTY VERSION ${${PROJECT_NAME}_VERSION} )
target_link_libraries( sisvm sigraph datamind_libsvm )
BRAINVISA_INSTALL( TARGETS sisvm
                   DESTINATION lib
                   COMPONENT ${PROJECT_NAME} )

