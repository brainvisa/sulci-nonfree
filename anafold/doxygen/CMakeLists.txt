find_package( Doxygen )
IF ( DOXYGEN_FOUND )
  SET( DOXYFILE_HTML_HEADER "\"${CMAKE_CURRENT_SOURCE_DIR}/head_anafold.html\"" )
  set( _doxygenInput ${HEADERS} )
  get_filename_component(cur_dir "${CMAKE_CURRENT_SOURCE_DIR}" PATH)
  get_filename_component(component_name ${cur_dir} NAME)
  BRAINVISA_GENERATE_DOXYGEN_DOC( _doxygenInput  head_anafold.html anafold.gif
                                  INPUT_PREFIX "${CMAKE_BINARY_DIR}/include/${component_name}"
                                  COMPONENT ${component_name} )
ENDIF ( DOXYGEN_FOUND )
