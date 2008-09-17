TEMPLATE	= lib
TARGET		= sigraphsvm${BUILDMODEEXT}

#!include ../../config

INCBDIR = si

HEADERS =  \
    plugin/sisvmplugin.h		\
    subadaptive/subadsvm.h		\
    subadaptive/svmparser.h

SOURCES = \
    plugin/sisvmplugin.cc		\
    subadaptive/subadsvm.cc		\
    subadaptive/svmparser.cc

LIBS	= ${LIBS_SVM}

