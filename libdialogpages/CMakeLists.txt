kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kompare/libdiff2 ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = \
#	-I$(top_srcdir)/kompare/libdiff2 \
#	$(all_includes)
#
#noinst_HEADERS = \
#	settingsbase.h \
#	diffsettings.h \
#	filessettings.h \
#	viewsettings.h \
#	pagebase.h \
#	diffpage.h \
#	filespage.h \
#	viewpage.h
#
## let automoc handle all of the meta source files (moc)
#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libdialogpages.la
#
## the library's source, library search path, and link libraries
#libdialogpages_la_SOURCES = \
#	settingsbase.cpp \
#	diffsettings.cpp \
#	filessettings.cpp \
#	viewsettings.cpp \
#	pagebase.cpp \
#	diffpage.cpp \
#	filespage.cpp \
#	viewpage.cpp
#
#libdialogpages_la_LDFLAGS = $(all_libraries)
#libdialogpages_la_LIBADD  = $(LIB_KFILE)
