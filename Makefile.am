SUBDIRS = pics

INCLUDES= $(all_includes)

noinst_HEADERS =  kdiff_shell.h kdiff_part.h kdiff_actions.h \
	kdiffview.h diffview.h misc.h prefsbase.h \
	kdiffprocess.h diffprefs.h diffsettings.h generalprefs.h \
	generalsettings.h kdiffprefdlg.h miscprefs.h miscsettings.h \
	settingsbase.h diffmodel.h difference.h diffhunk.h \
	kcomparedialog.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) -C *.cpp *.h -o $(podir)/kdiff.pot

KDE_ICON = AUTO

# this Makefile creates both a KPart application and a KPart
#########################################################################
# APPLICATION SECTION
#########################################################################
# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = kdiff

# the application source, library search path, and link libraries
kdiff_SOURCES = main.cpp kdiff_shell.cpp kcomparedialog.cpp
kdiff_LDFLAGS = $(KDE_RPATH) $(all_libraries)
kdiff_LDADD   = $(LIB_KPARTS) libkdiffpart.la

# this is where the desktop file will go
shelldesktopdir   = $(kde_appsdir)/Utilities
shelldesktop_DATA = kdiff.desktop

# this is where the shell's XML-GUI resource file goes
shellrcdir   = $(kde_datadir)/kdiff
shellrc_DATA = kdiffui.rc

#########################################################################
# KPART SECTION
#########################################################################
kde_module_LTLIBRARIES = libkdiffpart.la

# the Part's source, library search path, and link libraries
libkdiffpart_la_SOURCES = kdiff_part.cpp kdiff_actions.cpp \
	diffmodel.cpp difference.cpp diffhunk.cpp kdiffview.cpp diffview.cpp \
	misc.cpp diffsettings.cpp kdiffprocess.cpp settingsbase.cpp \
	miscsettings.cpp generalsettings.cpp kdiffprefdlg.cpp \
	diffprefs.cpp miscprefs.cpp generalprefs.cpp prefsbase.cpp
libkdiffpart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
libkdiffpart_la_LIBADD  = $(LIB_KPARTS) $(LIB_KFILE)

# this is where the desktop file will go
partdesktopdir   = $(kde_servicesdir)
partdesktop_DATA = kdiff_part.desktop

# this is where the part's XML-GUI resource file goes
partrcdir   = $(kde_datadir)/kdiffpart
partrc_DATA = kdiffpartui.rc

#bin_PROGRAMS = kdiff
#kdiff_SOURCES = miscsettings.cpp miscprefs.cpp kdiffprefdlg.cpp \
#	generalsettings.cpp generalprefs.cpp \
#	diffprefs.cpp kmaindiffview.cpp \
#	kdiffview.cpp prefsbase.cpp

#unifiedgrammar.cpp: unifiedgrammar.y
#	bison -o unifiedgrammar.cpp unifiedgrammar.y

#unifiedgrammar.h: unifiedgrammar.y
#	bison -d -o unifiedgrammar.c unifiedgrammar.y && rm unifiedgrammar.c

#unifiedlexer.cpp: unifiedlexer.l
#	flex -+ -ounifiedlexer.cpp unifiedlexer.l

rc.cpp:
	$(EXTRACTRC) *.rc > rc.cpp
