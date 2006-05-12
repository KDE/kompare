
include_directories( ${CMAKE_SOURCE_DIR}/kompare/libdialogpages ${CMAKE_SOURCE_DIR}/kompare/komparepart ${CMAKE_SOURCE_DIR}/kompare/interfaces  )


########### next target ###############


########### install files ###############





#original Makefile.am contents follow:

#INCLUDES = \
#	-I$(top_srcdir)/kompare/libdialogpages \
#	-I$(top_srcdir)/kompare/komparepart \
#	-I$(top_srcdir)/kompare/interfaces $(all_includes)
#
#noinst_HEADERS = \
#	levenshteintable.h \
#	kompare.h \
#	kompareprocess.h \
#	komparemodellist.h \
#	diffmodel.h \
#	difference.h \
#	diffhunk.h
#
## let automoc handle all of the meta source files (moc)
#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libdiff2.la
#
## the Part's source, library search path, and link libraries
#libdiff2_la_SOURCES = \
#	kompareprocess.cpp \
#	komparemodellist.cpp \
#	diffmodellist.cpp \
#	diffmodel.cpp \
#	difference.cpp \
#	diffhunk.cpp \
#	levenshteintable.cpp \
#	parser.cpp \
#	parserbase.cpp \
#	cvsdiffparser.cpp \
#	diffparser.cpp \
#	perforceparser.cpp
#
#libdiff2_la_LDFLAGS = $(all_libraries)
#libdiff2_la_LIBADD  = $(LIB_KFILE)
#
