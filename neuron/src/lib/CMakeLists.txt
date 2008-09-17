
getFromPro(neuron.pro HDRS SRCS)

SHFJ_COPY_HEADERS(include/neur ${HDRS})

ADD_LIBRARY(neuron
${SRCS}
)

TARGET_LINK_LIBRARIES(neuron
)

INSTALL(TARGETS neuron DESTINATION lib/)
