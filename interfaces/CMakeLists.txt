



########### next target ###############

set(kompareinterface_LIB_SRCS kompareinterface.cpp )

kde4_automoc(${kompareinterface_LIB_SRCS})

kde4_add_library(kompareinterface SHARED ${kompareinterface_LIB_SRCS})

target_link_libraries(kompareinterface  ${KDE4_KDECORE_LIBS} )

set_target_properties(kompareinterface PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} kompareinterface )


########### install files ###############





#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#
#lib_LTLIBRARIES =  libkompareinterface.la
#
#libkompareinterface_la_LDFLAGS = -no-undefined $(all_libraries)
#libkompareinterface_la_SOURCES = kompareinterface.cpp
#libkompareinterface_la_LIBADD  = $(LIB_QT)
#
