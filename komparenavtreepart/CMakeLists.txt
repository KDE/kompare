
include_directories( ${CMAKE_SOURCE_DIR}/kompare/libdiff2 ${CMAKE_SOURCE_DIR}/kompare/komparepart  )


########### next target ###############

set(komparenavtreepart_PART_SRCS komparenavtreepart.cpp )

kde4_automoc(${komparenavtreepart_PART_SRCS})

kde4_add_plugin(komparenavtreepart WITH_PREFIX ${komparenavtreepart_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} komparenavtreepart )

target_link_libraries(komparenavtreepart  ${KDE4_KDECORE_LIBS} kparts diff2 )

install(TARGETS komparenavtreepart  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES komparenavtreepart.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )




#original Makefile.am contents follow:

##########################################################################
## KPART SECTION
##########################################################################
#
#INCLUDES = \
#	-I$(top_srcdir)/kompare/libdiff2 \
#	-I$(top_srcdir)/kompare/komparepart \
#	$(all_includes)
#
#noinst_HEADERS = \
#	komparenavtreepart.h
#
## let automoc handle all of the meta source files (moc)
#METASOURCES = AUTO
#
#kde_module_LTLIBRARIES = libkomparenavtreepart.la
#
## the Part's source, library search path, and link libraries
#libkomparenavtreepart_la_SOURCES = \
#	komparenavtreepart.cpp
#
#libkomparenavtreepart_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
#libkomparenavtreepart_la_LIBADD  = $(LIB_KPARTS) ../libdiff2/libdiff2.la
#
## this is where the desktop file will go
#partdesktopdir   = $(kde_servicesdir)
#partdesktop_DATA = komparenavtreepart.desktop
#
#
