#########################################################################
# KPART SECTION
#########################################################################

INCLUDES = $(all_includes)

noinst_HEADERS = \
	kompare_part.h \
	kompare_actions.h \
	komparenavigationtree.h \
	komparemodellist.h \
	kompareview.h \
	prefsbase.h \
	kompareprocess.h \
	diffprefs.h \
	diffsettings.h \
	generalprefs.h \
	generalsettings.h \
	kompareprefdlg.h \
	miscprefs.h \
	miscsettings.h \
	settingsbase.h \
	diffmodel.h \
	difference.h \
	diffhunk.h \
	komparelistview.h \
	kompareconnectwidget.h \
	komparesaveoptionsbase.h \
	komparesaveoptionswidget.h \
	kompare.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

kde_module_LTLIBRARIES = libkomparepart.la

# the Part's source, library search path, and link libraries
libkomparepart_la_SOURCES = \
	kompare_part.cpp \
	kompare_actions.cpp \
	komparenavigationtree.cpp \
	kompareconnectwidget.cpp \
	kompareview.cpp \
	komparelistview.cpp \
	komparemodellist.cpp \
	diffmodel.cpp \
	difference.cpp \
	diffhunk.cpp \
	kompareprocess.cpp \
	settingsbase.cpp \
	diffsettings.cpp \
	generalsettings.cpp \
	miscsettings.cpp \
	kompareprefdlg.cpp \
	prefsbase.cpp \
	diffprefs.cpp \
	generalprefs.cpp \
	miscprefs.cpp \
	komparesaveoptionsbase.ui \
	komparesaveoptionswidget.cpp

libkomparepart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
libkomparepart_la_LIBADD  = $(LIB_KPARTS) $(LIB_KFILE)

# this is where the desktop file will go
partdesktopdir   = $(kde_servicesdir)
partdesktop_DATA = kompare_part.desktop

# this is where the part's XML-GUI resource file goes
partrcdir   = $(kde_datadir)/komparepart
partrc_DATA = komparepartui.rc

messages: rc.cpp
	$(XGETTEXT) -C *.cpp *.h -o $(podir)/kompare.pot
