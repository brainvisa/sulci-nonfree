
ADD_LIBRARY(bundle_sigraphsvm MODULE
  bundle/sisvmbundle.cc
)

set_property( TARGET bundle_sigraphsvm PROPERTY OUTPUT_NAME sigraphsvm.${${PROJECT_NAME}_VERSION} )

TARGET_LINK_LIBRARIES( bundle_sigraphsvm sigraphsvm )

BRAINVISA_INSTALL( TARGETS bundle_sigraphsvm
                   DESTINATION lib
                   COMPONENT ${PROJECT_NAME} )
