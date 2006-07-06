project(kompare)
add_subdirectory( interfaces ) 
add_subdirectory( libdiff2 ) 
add_subdirectory( libdialogpages ) 
add_subdirectory( komparenavtreepart ) 
add_subdirectory( komparepart ) 
add_subdirectory( pics ) 

include_directories(  ${CMAKE_CURRENT_SOURCE_DIR}/komparepart ${CMAKE_CURRENT_SOURCE_DIR}/libdiff2 ${CMAKE_CURRENT_SOURCE_DIR}/libdialogpages ${CMAKE_CURRENT_SOURCE_DIR}/interfaces ${CMAKE_CURRENT_SOURCE_DIR}/komparenavtreepart ${CMAKE_CURRENT_SOURCE_DIR}/komparepart)


########### next target ###############

set(kompare_SRCS main.cpp kompare_shell.cpp kompareurldialog.cpp )

kde4_automoc(${kompare_SRCS})

kde4_add_executable(kompare ${kompare_SRCS})

target_link_libraries(kompare  ${KDE4_KDECORE_LIBS} kparts kompareinterface dialogpages ktexteditor )

install(TARGETS kompare  DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES kompare.desktop  DESTINATION  ${XDG_APPS_DIR} )
install( FILES kompareui.rc  DESTINATION  ${DATA_INSTALL_DIR}/kompare )
install( FILES komparenavigationpart.desktop kompareviewpart.desktop  DESTINATION  ${SERVICETYPES_INSTALL_DIR} )




#original Makefile.am contents follow:

#SUBDIRS = interfaces libdiff2 libdialogpages komparenavtreepart komparepart pics
#
#INCLUDES = \
#	-I$(srcdir)/libdiff2 \
#	-I$(srcdir)/libdialogpages \
#	-I$(srcdir)/komparenavtreepart \
#	-I$(srcdir)/komparepart \
#	-I$(srcdir)/interfaces \
#	$(all_includes)
#
#noinst_HEADERS =  kompare_shell.h kompareurldialog.h
#
## let automoc handle all of the meta source files (moc)
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(EXTRACTRC) */*.rc */*.ui >> rc.cpp
#	$(XGETTEXT) `find . -name "*.cpp" -o -name "*.h"` -o $(podir)/kompare.pot
#
##########################################################################
## APPLICATION SECTION
##########################################################################
## this is the program that gets installed.  it's name is used for all
## of the other Makefile.am variables
#bin_PROGRAMS = kompare
#
## the application source, library search path, and link libraries
#kompare_SOURCES = main.cpp kompare_shell.cpp kompareurldialog.cpp
#kompare_LDFLAGS = $(all_libraries)
#kompare_LDADD   = $(LIB_KPARTS) \
#	$(top_builddir)/kompare/interfaces/libkompareinterface.la \
#	$(top_builddir)/kompare/libdialogpages/libdialogpages.la \
#	-lktexteditor
#
## this is where the desktop file will go
#xdg_apps_DATA = kompare.desktop
#
## this is where the shell's XML-GUI resource file goes
#shellrcdir = $(kde_datadir)/kompare
#shellrc_DATA = kompareui.rc
#
#kompareservicedir = $(kde_servicetypesdir)
#kompareservice_DATA = komparenavigationpart.desktop kompareviewpart.desktop
