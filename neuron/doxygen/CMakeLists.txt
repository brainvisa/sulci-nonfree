IF ( DOXYGEN_FOUND )
  get_filename_component(cur_dir "${CMAKE_CURRENT_SOURCE_DIR}" PATH)
  get_filename_component(component_name ${cur_dir} NAME)
  SET( DOXYFILE_HTML_HEADER "\"${CMAKE_CURRENT_SOURCE_DIR}/head_${component_name}.html\"" )
  set( _doxygenInput ${HEADERS} )
  BRAINVISA_GENERATE_DOXYGEN_DOC( _doxygenInput  head_${component_name}.html graph.gif
                                  INPUT_PREFIX "${CMAKE_BINARY_DIR}/include/neur" 
                                  COMPONENT ${component_name} )
ENDIF ( DOXYGEN_FOUND )
