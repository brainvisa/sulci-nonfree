
add_subdirectory(lib)
set(HEADERS ${HEADERS} PARENT_SCOPE)
#------------------------------------------------------------------------------
# Build targets for commands
#------------------------------------------------------------------------------
foreach( _command
  neurlearn
  neurmakenet
  neurpatimport )
  if( IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${_command}" )
    file( GLOB _pro "${CMAKE_CURRENT_SOURCE_DIR}/${_command}/*.pro" )
    BRAINVISA_GET_FILE_LIST_FROM_PRO( "${_pro}"
    TARGET _target
    SOURCES _proSources )
    set( _sources )
    foreach( _f ${_proSources} )
      set( _sources ${_sources} ${_command}/${_f} )
    endforeach( _f ${_proSources} )
    add_executable( ${_target} ${_sources} )
    target_link_libraries( ${_target} neuron )
    BRAINVISA_INSTALL( TARGETS ${_target}
    DESTINATION bin
    COMPONENT ${PROJECT_NAME} )
  endif( IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${_command}" )
endforeach( _command )

