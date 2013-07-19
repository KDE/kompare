
########### next target ###############

set(dialogpages_PART_SRCS
	filessettings.cpp
	viewsettings.cpp
	pagebase.cpp
	diffpage.cpp
	filespage.cpp
	viewpage.cpp )


kde4_add_library(komparedialogpages SHARED ${dialogpages_PART_SRCS})

target_link_libraries(komparedialogpages  ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${LIBKOMPAREDIFF2_LIBRARIES})

set_target_properties(komparedialogpages PROPERTIES VERSION ${GENERIC_LIB_VERSION}
SOVERSION ${GENERIC_LIB_SOVERSION} )

install(TARGETS komparedialogpages ${INSTALL_TARGETS_DEFAULT_ARGS})

