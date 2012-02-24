BRAINVISA_GET_FILE_LIST_FROM_PRO(  ${CMAKE_CURRENT_SOURCE_DIR}/anafold.pro "HEADERS" _headers "SOURCES" _sources )
BRAINVISA_COPY_AND_INSTALL_HEADERS( _headers anafold headersTarget )
BRAINVISA_ADD_MOC_FILES( _sources ${_headers} )
set(HEADERS ${_headers} PARENT_SCOPE)
add_library( anafold MODULE ${_sources})
add_dependencies( anafold ${headersTarget})
if( APPLE )
  set_property( TARGET anafold PROPERTY OUTPUT_NAME anafold.${${PROJECT_NAME}_VERSION} )
else()
  set_property( TARGET anafold PROPERTY VERSION ${${PROJECT_NAME}_VERSION} )
endif()
target_link_libraries( anafold ${ANATOMIST-FREE_LIBRARIES} ${SULCI-PRIVATE_LIBRARIES} )
BRAINVISA_INSTALL( TARGETS anafold
                   DESTINATION lib
                   COMPONENT ${PROJECT_NAME} )