TEMPLATE	= lib
TARGET		= neuron${BUILDMODEEXT}

#!include ../../config

INCBDIR = neur

HEADERS = \
	gauss/gaussian.h \
	gauss/gaussnet.h \
	stream/readstream.h \
	rand/rand.h \
	koho/kohonen.h \
	mlp/unit.h \
	mlp/pat.h \
	mlp/net.h \
	mlp/mlp.h \
	mlp/misc.h \
	mlp/lelm.h \
	mlp/link.h

SOURCES = \
	gauss/gaussian.cc \
	gauss/gaussnet.cc \
	stream/readstream.cc \
	rand/rand.cc \
	koho/kohonen.cc \
	mlp/unit.cc \
	mlp/pat.cc \
	mlp/net.cc \
	mlp/mlp.cc \
	mlp/misc.cc \
	mlp/lelm.cc \
	mlp/link.cc
