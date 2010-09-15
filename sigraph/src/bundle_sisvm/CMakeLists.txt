
ADD_LIBRARY(bundle_sigraphsvm MODULE
  bundle/sisvmbundle.cc
)

TARGET_LINK_LIBRARIES( bundle_sigraphsvm sigraphsvm )

BRAINVISA_INSTALL( TARGETS bundle_sigraphsvm
                   DESTINATION lib
                   COMPONENT ${PROJECT_NAME} )
