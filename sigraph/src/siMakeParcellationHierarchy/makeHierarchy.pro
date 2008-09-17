TARGET = siMakeParcellationHierarchy

#!include ../../config-app

release:LIBS	+= -laimsalgo
debug:LIBS	+= -laimsalgo-debug
irix:LIBS	+= -laimsalgo
darwin:LIBS	+= -laimsalgo

SOURCES = \
          main.cc
