BRAINVISA_GET_FILE_LIST_FROM_PRO( "${CMAKE_CURRENT_SOURCE_DIR}/sip.pro" "HEADERS" _headers "SOURCES" _sources )
BRAINVISA_COPY_AND_INSTALL_HEADERS( _headers pysigraph headersTarget )

file( GLOB _sipTemplates RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/*.tpl" )
BRAINVISA_COPY_FILES( ${PROJECT_NAME}-dev
                      typessub.py ${_sipTemplates}
                      SOURCE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                      DESTINATION "${SIGRAPH_RELATIVE_SIP_DIRECTORY}"
                      IMMEDIATE )

file( GLOB _sipSources "${CMAKE_CURRENT_SOURCE_DIR}/*.sip" )

file( MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/${SIGRAPH_RELATIVE_SIP_DIRECTORY}" )

if( SIP_MODULE )
  set( extra_defs "-D" "SIP_MODULE=\"${SIP_MODULE}\"" )
endif()
if( SIP_ABI )
  string( REGEX MATCH "[^.]*" SIP_ABI_MAJOR_VERSION "${SIP_ABI}")
  set( extra_defs "${extra_defs}" "-D"
       "SIP_ABI_MAJOR_VERSION=${SIP_ABI_MAJOR_VERSION}")
endif()

execute_process( COMMAND "${PYTHON_HOST_EXECUTABLE}"
        "${AIMS-FREE_SIP_DIRECTORY}/maketemplates.py"
        -s "${AIMS-FREE_SIP_DIRECTORY}"
        -i "${CMAKE_CURRENT_SOURCE_DIR}/generatedtypes.py"
        -p "${CMAKE_CURRENT_SOURCE_DIR}"
        -o "${CMAKE_BINARY_DIR}/${SIGRAPH_RELATIVE_SIP_DIRECTORY}"
        -t "${AIMS-FREE_SIP_DIRECTORY}/typessub.py"
        -t "${CMAKE_CURRENT_SOURCE_DIR}/typessub.py"
        -l
        -P "${CMAKE_C_PREPROCESSOR}"
        ${extra_defs}
  OUTPUT_VARIABLE _generatedSipFileList 
  RESULT_VARIABLE _returnValue )

if( NOT _returnValue EQUAL 0 )
  message( SEND_ERROR "The sip generation from ${PROJECT_NAME} templates with \"${AIMS-FREE_SIP_DIRECTORY}/maketemplates.py\" returned an error:\n${_generatedSipFileList}\n========================")
endif( NOT _returnValue EQUAL 0 )

string( STRIP "${_generatedSipFileList}" _generatedSipFileList )
BRAINVISA_ADD_SIP_PYTHON_MODULE( sigraphsip
  sigraph 
  "${CMAKE_CURRENT_SOURCE_DIR}/sigraph.sip"
  SIP_SOURCES ${_generatedSipFileList} ${_sipSources}
  SIP_INCLUDE "${CMAKE_BINARY_DIR}/${SIGRAPH_RELATIVE_SIP_DIRECTORY}"
    "${AIMS-FREE_SIP_DIRECTORY}" )
add_dependencies( sigraphsip ${headersTarget} )
target_link_libraries( sigraphsip
  "${PYTHON_LIBRARY}"
  pysigraph
  "${AIMS_PYAIMS_LIB}" )
BRAINVISA_INSTALL( TARGETS sigraphsip
                   DESTINATION python/sigraph
                   COMPONENT ${PROJECT_NAME} )

# TODO: install .sip files in devel package
