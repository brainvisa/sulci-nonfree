
add_subdirectory( src )
add_subdirectory( doxygen )
# qt translation files 
BRAINVISA_ADD_TRANSLATION( "shared" "share/anatomist-${anatomist-free_VERSION_MAJOR}.${anatomist-free_VERSION_MINOR}" ${PROJECT_NAME})
BRAINVISA_COPY_DIRECTORY( "${CMAKE_CURRENT_SOURCE_DIR}/shared"
                          "share/anatomist-${anatomist-free_VERSION_MAJOR}.${anatomist-free_VERSION_MINOR}"
                          ${PROJECT_NAME} )
