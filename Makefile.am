SUBDIRS = kdiffpart pics

INCLUDES= $(all_includes) -I$(top_srcdir)/kdiff/kdiff/kdiffpart

noinst_HEADERS =  kdiff_shell.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) -C *.cpp *.h -o $(podir)/kdiff.pot

KDE_ICON = AUTO

#########################################################################
# APPLICATION SECTION
#########################################################################
# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = kdiff

# the application source, library search path, and link libraries
kdiff_SOURCES = main.cpp kdiff_shell.cpp kcomparedialog.cpp
kdiff_LDFLAGS = $(KDE_RPATH) $(all_libraries)
kdiff_LDADD   = $(LIB_KPARTS) $(top_builddir)/kdiff/kdiff/kdiffpart/libkdiffpart.la

# this is where the desktop file will go
shelldesktopdir   = $(kde_appsdir)/Utilities
shelldesktop_DATA = kdiff.desktop

# this is where the shell's XML-GUI resource file goes
shellrcdir   = $(kde_datadir)/kdiff
shellrc_DATA = kdiffui.rc

rc.cpp:
	$(EXTRACTRC) *.rc > rc.cpp
