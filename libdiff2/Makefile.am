INCLUDES = -I$(top_srcdir)/kompare/komparepart $(all_includes)

noinst_HEADERS = \
	kompare.h \
	kompareprocess.h \
	komparemodellist.h \
	diffmodel.h \
	difference.h \
	diffhunk.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

lib_LTLIBRARIES = libdiff2.la

# the Part's source, library search path, and link libraries
libdiff2_la_SOURCES = \
	kompareprocess.cpp \
	komparemodellist.cpp \
	diffmodel.cpp \
	difference.cpp \
	diffhunk.cpp

libdiff2_la_LDFLAGS = -module -avoid-version -no-undefined \
	$(KDE_PLUGIN) \
	$(all_libraries)
libdiff2_la_LIBADD  = $(LIB_KPARTS) $(LIB_KFILE)

# this is where the desktop file will go
desktopdir   = $(kde_datadir)/kompare
desktop_DATA = libdiff2.desktop

