#########################################################################
# KPART SECTION
#########################################################################

INCLUDES = \
	-I$(top_srcdir)/kompare/libdiff2 \
	-I$(top_srcdir)/kompare/komparepart \
	$(all_includes)

noinst_HEADERS = \
	komparenavtreepart.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

kde_module_LTLIBRARIES = libkomparenavtreepart.la

# the Part's source, library search path, and link libraries
libkomparenavtreepart_la_SOURCES = \
	komparenavtreepart.cpp

libkomparenavtreepart_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
libkomparenavtreepart_la_LIBADD  = $(LIB_KPARTS) ../libdiff2/libdiff2.la

# this is where the desktop file will go
partdesktopdir   = $(kde_servicesdir)
partdesktop_DATA = komparenavtreepart.desktop


