TEMPLATE        = sip
TARGET		= sigraphsip

LIBBDIR = python/sigraph

#!include ../../config-local

LIBS = ${SIP_LIBS}

SIPS = sigraph.sip

HEADERS = adaptive.h \
        clique.h \
        learnable.h
