#########################################################################
# KPART SECTION
#########################################################################

INCLUDES = -I$(top_srcdir)/kompare/libdiff2 -I$(top_srcdir)/kompare/interfaces $(all_includes)

noinst_HEADERS = \
	kompare_part.h \
	komparesplitter.h \
	prefsbase.h \
	diffprefs.h \
	diffsettings.h \
	viewprefs.h \
	viewsettings.h \
	kompareprefdlg.h \
	settingsbase.h \
	komparelistview.h \
	kompareconnectwidget.h \
	komparesaveoptionsbase.h \
	komparesaveoptionswidget.h \
	kompare_qsplitter.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

kde_module_LTLIBRARIES = libkomparepart.la

# the Part's source, library search path, and link libraries
libkomparepart_la_SOURCES = \
	kompare_part.cpp \
	kompareconnectwidget.cpp \
	komparesplitter.cpp \
	komparelistview.cpp \
	settingsbase.cpp \
	diffsettings.cpp \
	viewsettings.cpp \
	kompareprefdlg.cpp \
	prefsbase.cpp \
	diffprefs.cpp \
	viewprefs.cpp \
	komparesaveoptionsbase.ui \
	komparesaveoptionswidget.cpp

libkomparepart_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
libkomparepart_la_LIBADD  = $(LIB_KPARTS) $(LIB_KFILE) \
	../libdiff2/libdiff2.la \
	../interfaces/libkompareinterface.la

# this is where the desktop file will go
partdesktopdir   = $(kde_servicesdir)
partdesktop_DATA = komparepart.desktop

# this is where the part's XML-GUI resource file goes
partrcdir   = $(kde_datadir)/kompare
partrc_DATA = komparepartui.rc

