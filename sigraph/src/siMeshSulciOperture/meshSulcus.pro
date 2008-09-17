TARGET = siMeshSulciOperture

#!include ../../config-app

irix:LIBS += -laimsalgo
darwin:LIBS += -laimsalgo
release:LIBS	+= -laimsalgo
debug:LIBS	+= -laimsalgo-debug

SOURCES = \
          main.cc
