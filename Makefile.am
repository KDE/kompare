SUBDIRS = pics

bin_PROGRAMS = kdiff
kdiff_SOURCES = miscsettings.cpp miscprefs.cpp kdiffprefdlg.cpp \
	generalsettings.cpp generalprefs.cpp diffsettings.cpp \
	diffprefs.cpp kdiffprocess.cpp kmaindiffview.cpp \
	kdiffview.cpp main.cpp kdiffapp.cpp prefsbase.cpp settingsbase.cpp
#	diffformatbase.cpp unifieddiff.cpp rcsdiff.cpp eddiff.cpp \
#	contextdiff.cpp normaldiff.cpp
#	unifiedgrammar.cpp unifiedlexer.cpp
	
INCLUDES= $(all_includes)
kdiff_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kdiff_LDADD = $(LIB_KDEUI) $(LIB_KHTML) $(LIB_KFILE)

noinst_HEADERS = kdiffapp.h kdiffview.h kmaindiffview.h prefsbase.h \
	kdiffprocess.h diffprefs.h diffsettings.h generalprefs.h \
	generalsettings.h kdiffprefdlg.h miscprefs.h miscsettings.h
#	diffformatbase.h unifieddiff.h rcsdiff.h eddiff.h \
#	contextdiff.h normaldiff.h
#	settingsbase.h unifiedgrammar.h
	
METASOURCES = AUTO

kdelnk_DATA = kdiff.desktop
kdelnkdir = $(kde_appsdir)/Utilities

rcdir = $(kde_datadir)/kdiff
rc_DATA = kdiffui.rc

KDE_ICON = AUTO

unifiedgrammar.cpp: unifiedgrammar.y
	bison -o unifiedgrammar.cpp unifiedgrammar.y

unifiedgrammar.h: unifiedgrammar.y
	bison -d -o unifiedgrammar.c unifiedgrammar.y && rm unifiedgrammar.c

unifiedlexer.cpp: unifiedlexer.l
	flex -+ -ounifiedlexer.cpp unifiedlexer.l

messages: rc.cpp
	$(XGETTEXT) -C *.cpp *.h -o $(podir)/kdiff.pot

rc.cpp:
	$(EXTRACTRC) *.rc > rc.cpp
