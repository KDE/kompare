#########################################################################
# KPART SECTION
#########################################################################

INCLUDES = $(all_includes)

noinst_HEADERS =  kdiff_part.h kdiff_actions.h kdiffnavigationtree.h \
        kdiffmodellist.h kdiffview.h prefsbase.h \
        kdiffprocess.h diffprefs.h diffsettings.h generalprefs.h \
        generalsettings.h kdiffprefdlg.h miscprefs.h miscsettings.h \
        settingsbase.h diffmodel.h difference.h diffhunk.h \
        kdifflistview.h kdiffconnectwidget.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

kde_module_LTLIBRARIES = libkdiffpart.la

# the Part's source, library search path, and link libraries
libkdiffpart_la_SOURCES = kdiff_part.cpp kdiff_actions.cpp \
	kdiffnavigationtree.cpp diffmodel.cpp difference.cpp diffhunk.cpp \
	kdiffview.cpp diffsettings.cpp kdiffprocess.cpp settingsbase.cpp \
	miscsettings.cpp generalsettings.cpp kdiffprefdlg.cpp diffprefs.cpp \
	miscprefs.cpp generalprefs.cpp prefsbase.cpp kdiffmodellist.cpp \
	kdifflistview.cpp kdiffconnectwidget.cpp

libkdiffpart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
libkdiffpart_la_LIBADD  = $(LIB_KPARTS) $(LIB_KFILE)

# this is where the desktop file will go
partdesktopdir   = $(kde_servicesdir)
partdesktop_DATA = kdiff_part.desktop

# this is where the part's XML-GUI resource file goes
partrcdir   = $(kde_datadir)/kdiffpart
partrc_DATA = kdiffpartui.rc

messages: rc.cpp
	$(XGETTEXT) -C *.cpp *.h -o $(podir)/kdiff.pot
