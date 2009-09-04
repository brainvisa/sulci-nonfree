BRAINVISA_GET_FILE_LIST_FROM_PRO(  ${CMAKE_CURRENT_SOURCE_DIR}/neuron.pro "HEADERS" _headers "SOURCES" _sources )
BRAINVISA_COPY_AND_INSTALL_HEADERS( _headers neur )
add_library( neuron SHARED ${_sources} )
set_property( TARGET neuron PROPERTY VERSION ${${PROJECT_NAME}_VERSION} )
BRAINVISA_INSTALL( TARGETS neuron
              DESTINATION lib
              COMPONENT ${PROJECT_NAME} )
