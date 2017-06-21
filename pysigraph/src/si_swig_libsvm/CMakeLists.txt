IF ( SWIG_FOUND )
  
  INCLUDE(${SWIG_USE_FILE})
  include_directories(${PYTHON_INCLUDE_PATH})
  include_directories(${SVM_INCLUDE_DIRS}/../python)
  include_directories(${SVM_INCLUDE_DIRS})
  include_directories("${CMAKE_BINARY_DIR}/include")
  set( SWIG_FLAGS "" )
  SET_SOURCE_FILES_PROPERTIES(si_swig_libsvm.i PROPERTIES CPLUSPLUS ON)
  set_source_files_properties(si_swig_libsvm.i PROPERTIES SWIG_FLAGS "-includeall")
  
  SWIG_ADD_MODULE(_si_swig_libsvm PYTHON si_swig_libsvm.i )
  
  SWIG_LINK_LIBRARIES( _si_swig_libsvm "${PYTHON_LIBRARY}" sigraph_libsvm)
  
  set_property( TARGET __si_swig_libsvm PROPERTY LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/python/sigraph )
  set_property( TARGET __si_swig_libsvm PROPERTY OUTPUT_NAME _si_swig_libsvm )
  set_property( TARGET __si_swig_libsvm PROPERTY PREFIX "" )

ENDIF ( SWIG_FOUND )

