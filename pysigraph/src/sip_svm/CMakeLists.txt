include_directories( ${SVM_INCLUDE_DIR} )

BRAINVISA_GET_FILE_LIST_FROM_PRO( "${CMAKE_CURRENT_SOURCE_DIR}/sipsvm.pro" "HEADERS" _headers "SOURCES" _sources )
# BRAINVISA_COPY_AND_INSTALL_HEADERS( _headers pysigraph headersTarget )


file( GLOB _sipSources "${CMAKE_CURRENT_SOURCE_DIR}/*.sip" )

BRAINVISA_ADD_SIP_PYTHON_MODULE( sigraphsvmsip
  sigraph 
  "${CMAKE_CURRENT_SOURCE_DIR}/sigraphsvm.sip"
  SIP_SOURCES ${_sipSources}
#   ${_generatedSipFileList} 
  SIP_INCLUDE "${CMAKE_BINARY_DIR}/${SIGRAPH_RELATIVE_SIP_DIRECTORY}"
    "${AIMS-FREE_SIP_DIRECTORY}" "${CMAKE_CURRENT_SOURCE_DIR}/../sip" )
# add_dependencies( sigraphsip ${headersTarget} )
target_link_libraries( sigraphsvmsip
  ${PYTHON_LIBRARIES}
  pysigraph sigraphsvm
  "${AIMS_PYAIMS_LIB}" )
BRAINVISA_INSTALL( TARGETS sigraphsvmsip
                   DESTINATION python/sigraph
                   COMPONENT ${PROJECT_NAME} )

# TODO: install .sip files in devel package
