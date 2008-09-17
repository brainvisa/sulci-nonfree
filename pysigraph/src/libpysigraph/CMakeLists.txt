
getFromPro(pysigraph.pro HEADERS SOURCES)

SHFJ_COPY_HEADERS(include/pysigraph ${HEADERS})

ADD_LIBRARY(pysigraph SHARED
${SOURCES}
)

TARGET_LINK_LIBRARIES(pysigraph
pyaims
${PYTHON_LIBRARY}
)

INSTALL(TARGETS pysigraph DESTINATION lib/)
