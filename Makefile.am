SUBDIRS = komparepart pics

INCLUDES= -I$(top_srcdir)/kompare/komparepart $(all_includes)

noinst_HEADERS =  kompare_shell.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(EXTRACTRC) */*.rc */*.ui >> rc.cpp
	$(XGETTEXT) `find . -name "*.cpp" -o -name "*.h"` -o $(podir)/kompare.pot

KDE_ICON = AUTO

#########################################################################
# APPLICATION SECTION
#########################################################################
# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = kompare

# the application source, library search path, and link libraries
kompare_SOURCES = main.cpp kompare_shell.cpp kcomparedialog.cpp
kompare_LDFLAGS = $(KDE_RPATH) $(all_libraries)
kompare_LDADD   = $(LIB_KPARTS) $(top_builddir)/kompare/komparepart/libkomparepart.la

# this is where the desktop file will go
shelldesktopdir   = $(kde_appsdir)/Development
shelldesktop_DATA = kompare.desktop

# this is where the shell's XML-GUI resource file goes
shellrcdir   = $(kde_datadir)/kompare
shellrc_DATA = kompareui.rc

