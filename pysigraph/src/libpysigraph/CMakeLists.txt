BRAINVISA_GET_FILE_LIST_FROM_PRO(  ${CMAKE_CURRENT_SOURCE_DIR}/pysigraph.pro "HEADERS" _headers "SOURCES" _sources )
BRAINVISA_COPY_AND_INSTALL_HEADERS( _headers pysigraph headersTarget )
add_library( pysigraph SHARED ${_sources} )
add_dependencies( pysigraph ${headersTarget} )
set_property( TARGET pysigraph PROPERTY VERSION ${${PROJECT_NAME}_VERSION} )
target_link_libraries( pysigraph sigraph )
BRAINVISA_INSTALL( TARGETS pysigraph
              DESTINATION lib
              COMPONENT ${PROJECT_NAME} )
