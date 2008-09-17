
getFromPro(sip.pro SIP_HDRS SIP_SRCS)

SHFJ_COPY_HEADERS(include/pysigraph ${SIP_HDRS})

FILE(GLOB globbedTpl           ${CMAKE_CURRENT_SOURCE_DIR}/*.tpl)
FILE(GLOB relativeTpl RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.tpl)
FILE(GLOB globbedSip           ${CMAKE_CURRENT_SOURCE_DIR}/*.sip)
FILE(GLOB relativeSip RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.sip)

# Copy sip & tpl files for further compilation
COPY_FILES(${CMAKE_BINARY_DIR}/sip ${globbedSip})
COPY_FILES(${CMAKE_BINARY_DIR}/tpl ${globbedTpl})


SET(SIP_TYPESSUB                  ${CMAKE_CURRENT_SOURCE_DIR}/typessub.py)
SET(makeTemplatesGeneratedTypePy  ${CMAKE_CURRENT_SOURCE_DIR}/generatedtypes.py)
SET(TPL_DIRECTORY                 ${CMAKE_BINARY_DIR}/tpl)


SIP_MAKE_TEMPLATES(${makeTemplatesGeneratedTypePy} 
${TPL_DIRECTORY} 
${CMAKE_CURRENT_BINARY_DIR}/sip 
${globbedTpl}
)


# Get the .sip files that have been created
FILE(GLOB SIPPED_TPL ${CMAKE_CURRENT_BINARY_DIR}/sip/*.sip)


# copy the sipped files to a common directory so that external libraries
# can access them for further compilation
COPY_FILES(${CMAKE_BINARY_DIR}/sip ${SIPPED_TPL})



SET(SIP_INCLUDES 
${PYAIMS_SIP_DIR}
${CMAKE_BINARY_DIR}/sip 
${PYQT_SIP_DIR}
)
SET(SIP_CONCAT_PARTS 1)
SET(SIP_TAGS ALL WS_X11 Qt_3_3_0)
SET(SIP_DISABLE_FEATURES 
VendorID 
Qt_STYLE_WINDOWSXP 
Qt_STYLE_INTERLACE
Qt_STYLE_WINDOWSXP
)
SET(SIP_EXTRA_OPTIONS -e)
SET(SIP_EXTRA_DEPENDENCIES ${SIPPED_TPL} ${globbedSip})
SET(SIP_MAIN_FILE          ${CMAKE_BINARY_DIR}/sip/sigraph.sip)

ADD_SIP_PYTHON_MODULE(sigraphsip ${SIP_MAIN_FILE}
pysigraph
sigraph
)


INSTALL(TARGETS sigraphsip DESTINATION python/sigraph)
