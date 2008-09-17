TEMPLATE        = sip
TARGET		= sigraphsvmsip

LIBBDIR = python/sigraph

#!include ../../config-local

LIBS = ${SIP_LIBS} ${SVM_LIBS}
SIPINCLUDE += ${SIPINC_SVM}

SIPS = sigraphsvm.sip

