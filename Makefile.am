SUBDIRS = interfaces libdiff2 libdialogpages komparenavtreepart komparepart pics

INCLUDES = \
	-I$(srcdir)/libdiff2 \
	-I$(srcdir)/libdialogpages \
	-I$(srcdir)/komparenavtreepart \
	-I$(srcdir)/komparepart \
	-I$(srcdir)/interfaces \
	$(all_includes)

noinst_HEADERS =  kompare_shell.h kompareurldialog.h

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
kompare_SOURCES = main.cpp kompare_shell.cpp kompareurldialog.cpp
kompare_LDFLAGS = $(all_libraries)
kompare_LDADD   = $(LIB_KPARTS) \
	$(top_builddir)/kompare/interfaces/libkompareinterface.la \
	$(top_builddir)/kompare/libdialogpages/libdialogpages.la

# this is where the desktop file will go
xdg_apps_DATA = kompare.desktop

# this is where the shell's XML-GUI resource file goes
shellrcdir = $(kde_datadir)/kompare
shellrc_DATA = kompareui.rc

kompareservicedir = $(kde_servicetypesdir)
kompareservice_DATA = komparenavigationpart.desktop kompareviewpart.desktop
