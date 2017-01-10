
ADD_SUBDIRECTORY(lib)
set(HEADERS ${LIB_HEADERS} PARENT_SCOPE)
if( DATAMIND_FOUND AND SVM_FOUND )
  include( "${DATAMIND_USE_FILE}" )
  ADD_SUBDIRECTORY(libsigraphsvm)
  set(HEADERS ${LIB_HEADERS} ${LIBSIGRAPHSVM_HEADERS} PARENT_SCOPE)
  IF(APPLE)
    ADD_SUBDIRECTORY(bundle_sisvm)
  ENDIF()
else()
  message( "WARNING: libsigraphsvm will not be build because of dependencies: DATAMIND_FOUND=\"${DATAMIND_FOUND}\" AND SVM_FOUND=\"${SVM_FOUND}\"" )
endif()

#------------------------------------------------------------------------------
# Build targets for commands
#------------------------------------------------------------------------------
foreach( _command
  readFolds
  siChangeModels
  siCopyNames
  siDivNamelist
  siDomTrain
  siEnergy
  siError
  siErrorStats
  siFlip
  siFunctionalGraphs
  siGraph2Label
  sigraph-config
  siLearn
  siLyxErrors
  siMakeColoredHierarchy
  siMakeModel
  siMakeParcellationHierarchy
  siMergeModels
  siMeshSulciOperture
  siMeshSulciProjection
  siMkModelEdges
  siMorpho
  siMultilabelCompare
  siParcellation
  siPIDcommand
  siPutTextureInModel
  siRelax
  siRevert
  siSulcalParcellation
  siTestModels
  siTriangModel
  siTryGauss )
  if( IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${_command}" )
    file( GLOB _pro "${CMAKE_CURRENT_SOURCE_DIR}/${_command}/*.pro" )
    BRAINVISA_GET_FILE_LIST_FROM_PRO( "${_pro}"
    TARGET _target
    SOURCES _proSources )
    set( _sources )
    foreach( _f ${_proSources} )
      set( _sources ${_sources} ${_command}/${_f} )
    endforeach( _f ${_proSources} )
    brainvisa_add_executable( ${_target} ${_sources} )
    target_link_libraries( ${_target} sigraph )
    BRAINVISA_INSTALL( TARGETS ${_target}
    DESTINATION bin
    COMPONENT ${PROJECT_NAME} )
  endif( IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${_command}" )
endforeach( _command )

