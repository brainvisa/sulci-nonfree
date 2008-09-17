TEMPLATE	= bundle
TARGET		= sigraphsvm${BUILDMODEEXT}

#!include ../../config

INCBDIR = si

SOURCES =			\
    bundle/sisvmbundle.cc

LIBS	= ${LIBS_SVM}

LIBS	+= -laimsdicom${BUILDMODEEXT}

