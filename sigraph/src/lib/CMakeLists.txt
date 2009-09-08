BRAINVISA_GET_FILE_LIST_FROM_PRO(  ${CMAKE_CURRENT_SOURCE_DIR}/sigraph.pro "HEADERS" _headers "SOURCES" _sources )
BRAINVISA_COPY_AND_INSTALL_HEADERS( _headers si headersTarget )
add_library( sigraph SHARED ${_sources} )
add_dependencies( sigraph ${headersTarget} )
set_property( TARGET sigraph PROPERTY VERSION ${${PROJECT_NAME}_VERSION} )
target_link_libraries( sigraph ${AIMS-FREE_LIBRARIES} neuron )
BRAINVISA_INSTALL( TARGETS sigraph
              DESTINATION lib
              COMPONENT ${PROJECT_NAME} )
