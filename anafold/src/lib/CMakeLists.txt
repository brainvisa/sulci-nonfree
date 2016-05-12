BRAINVISA_GET_FILE_LIST_FROM_PRO(  ${CMAKE_CURRENT_SOURCE_DIR}/anafold.pro "HEADERS" _headers "SOURCES" _sources )
BRAINVISA_COPY_AND_INSTALL_HEADERS( _headers anafold headersTarget )
BRAINVISA_ADD_MOC_FILES( _sources ${_headers} )
set(HEADERS ${_headers} PARENT_SCOPE)
add_library( anafold MODULE ${_sources})
add_dependencies( anafold ${headersTarget})

if( APPLE )
  # Setting VERSION or SOVERSION on a MODULE cause a link error on MacOS 10.5 with cmake 2.6:
  #  i686-apple-darwin9-g++-4.0.1: -compatibility_version only allowed with -dynamiclib
  set_property( TARGET anafold PROPERTY OUTPUT_NAME anafold.${${PROJECT_NAME}_VERSION} )
else()
  # in cmake 3.5 on linux, making a "module" removes the version number in the
  # .so so the suffix has to be forced.
  set_target_properties( anafold
    PROPERTIES SOVERSION ${${PROJECT_NAME}_VERSION} SUFFIX ".so.${${PROJECT_NAME}_VERSION}" )
endif()

target_link_libraries( anafold ${ANATOMIST-FREE_LIBRARIES} sigraph )
BRAINVISA_INSTALL( TARGETS anafold
                   DESTINATION lib
                   COMPONENT ${PROJECT_NAME} )
