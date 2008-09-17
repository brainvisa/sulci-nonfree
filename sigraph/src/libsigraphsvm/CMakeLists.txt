
getFromPro(sisvm.pro HEADERS SOURCES)

SHFJ_COPY_HEADERS(include/si ${HEADERS})

ADD_LIBRARY(sigraphsvm SHARED
${SOURCES}
)

TARGET_LINK_LIBRARIES(sigraphsvm
sigraph
)

INSTALL(TARGETS sigraphsvm DESTINATION lib/)
