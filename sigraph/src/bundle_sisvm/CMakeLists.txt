
ADD_LIBRARY(bundle_sigraphsvm MODULE
bundle/sisvmbundle.cc
)

TARGET_LINK_LIBRARIES(bundle_sigraphsvm
${SVM_LIBRARY}
aimsdicom
)

INSTALL(TARGETS bundle_sigraphsvm DESTINATION lib/)
