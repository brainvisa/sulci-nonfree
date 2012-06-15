BRAINVISA_COPY_DIRECTORY( ${CMAKE_CURRENT_SOURCE_DIR}/doc
  share/doc/pysigraph-${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}
  ${PROJECT_NAME} )

# Add python commands as executables
# message("pysigraph : ${CMAKE_CURRENT_SOURCE_DIR}")
file( GLOB _commands "${CMAKE_CURRENT_SOURCE_DIR}/bin/*.py" )
foreach(_command ${_commands})
  get_filename_component( _command_path "${_command}" ABSOLUTE)
  get_filename_component( _command_name "${_command}" NAME)
#   message("Add pysigraph command ${_command}, _command_path ${_command_path}")
  BRAINVISA_ADD_EXECUTABLE( ${_command_name} "${_command_path}" IS_SCRIPT )
endforeach()

unset(_commands)
unset(_command_path)
unset(_command_name)

# BRAINVISA_COPY_DIRECTORY( ${CMAKE_CURRENT_SOURCE_DIR}/bin
#   bin ${PROJECT_NAME} )
BRAINVISA_COPY_DIRECTORY( ${CMAKE_CURRENT_SOURCE_DIR}/scripts
  scripts ${PROJECT_NAME} )
BRAINVISA_COPY_DIRECTORY( ${CMAKE_CURRENT_SOURCE_DIR}/tests
  tests ${PROJECT_NAME} )
BRAINVISA_COPY_PYTHON_DIRECTORY( ${CMAKE_CURRENT_SOURCE_DIR}/python 
                                  ${PROJECT_NAME} )

add_subdirectory( src )

