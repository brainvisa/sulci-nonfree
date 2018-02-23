BRAINVISA_GET_FILE_LIST_FROM_PRO(  ${CMAKE_CURRENT_SOURCE_DIR}/anafold.pro "HEADERS" _headers "SOURCES" _sources )
BRAINVISA_COPY_AND_INSTALL_HEADERS( _headers anafold headersTarget )
BRAINVISA_ADD_MOC_FILES( _sources ${_headers} )
set(HEADERS ${_headers} PARENT_SCOPE)
add_library( anafold MODULE ${_sources})
add_dependencies( anafold ${headersTarget})

# The plugin should have anatomist version number, not sigraph version,
# beacause it will be looked for from anatomist, (anatomist plugins list),
# using anatomist version number

if( APPLE )
  # Setting VERSION or SOVERSION on a MODULE cause a link error on MacOS 10.5 with cmake 2.6:
  #  i686-apple-darwin9-g++-4.0.1: -compatibility_version only allowed with -dynamiclib
  set_property( TARGET anafold PROPERTY OUTPUT_NAME anafold.${anatomist-free_VERSION} )
elseif( CMAKE_VERSION VERSION_LESS "3.5" )
    set_target_properties( anafold
        PROPERTIES SOVERSION ${anatomist-free_VERSION} )
else()
  # in cmake 3.5 on linux, making a "module" removes the version number in the
  # .so so the suffix has to be forced.
  set_target_properties( anafold
    PROPERTIES SOVERSION ${anatomist-free_VERSION} SUFFIX ".so.${anatomist-free_VERSION}" )
endif()

target_link_libraries( anafold ${ANATOMIST-FREE_LIBRARIES} sigraph )
BRAINVISA_INSTALL( TARGETS anafold
                   DESTINATION lib
                   COMPONENT ${PROJECT_NAME} )
