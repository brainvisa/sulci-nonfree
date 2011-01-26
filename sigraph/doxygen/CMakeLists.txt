IF ( DOXYGEN_FOUND )
  get_filename_component(cur_dir "${CMAKE_CURRENT_SOURCE_DIR}" PATH)
  get_filename_component(component_name ${cur_dir} NAME)
  SET( DOXYFILE_HTML_HEADER "\"${CMAKE_CURRENT_SOURCE_DIR}/head_${component_name}.html\"" )
  set( _doxygenInput "${CMAKE_BINARY_DIR}/include/si" )
  set(aims_version "${aims-free_VERSION_MAJOR}.${aims-free_VERSION_MINOR}")
  set( DOXYFILE_TAGFILES "${CMAKE_BINARY_DIR}/share/doc/aimsalgo-${aims_version}/doxygen/aimsalgo.tag=../../aimsalgo-${aims_version}/doxygen ${CMAKE_BINARY_DIR}/share/doc/aimsdata-${aims_version}/doxygen/aimsdata.tag=../../aimsdata-${aims_version}/doxygen ${CMAKE_BINARY_DIR}/share/doc/graph-${aims_version}/doxygen/graph.tag=../../graph-${aims_version}/doxygen ${CMAKE_BINARY_DIR}/share/doc/cartobase-${aims_version}/doxygen/cartobase.tag=../../cartobase-${aims_version}/doxygen ${CMAKE_BINARY_DIR}/share/doc/cartodata-${aims_version}/doxygen/cartodata.tag=../../cartodata-${aims_version}/doxygen ${CMAKE_BINARY_DIR}/share/doc/neuron-${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}/doxygen/neuron.tag=../../neuron--${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}/doxygen")

  BRAINVISA_GENERATE_DOXYGEN_DOC( _doxygenInput  head_${component_name}.html ${component_name}.gif
                                  COMPONENT ${component_name} )
  add_dependencies( ${component_name}-doxygen aimsalgo-doxygen aimsdata-doxygen graph-doxygen cartobase-doxygen cartodata-doxygen neuron-doxygen )
ENDIF ( DOXYGEN_FOUND )
