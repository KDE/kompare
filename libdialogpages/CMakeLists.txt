
########### next target ###############

set(dialogpages_PART_SRCS
	filessettings.cpp
	viewsettings.cpp
	diffpage.cpp
	filespage.cpp
	viewpage.cpp )


add_library(komparedialogpages SHARED ${dialogpages_PART_SRCS})

include_directories(${LIBKOMPAREDIFF2_INCLUDE_DIR})

generate_export_header(komparedialogpages BASE_NAME DIALOGPAGES)

target_link_libraries(komparedialogpages
    KF5::Codecs
    KF5::Parts
    KF5::WidgetsAddons
    ${LIBKOMPAREDIFF2_LIBRARIES}
)

set_target_properties(komparedialogpages PROPERTIES VERSION ${GENERIC_LIB_VERSION}
SOVERSION ${GENERIC_LIB_SOVERSION} )

install(TARGETS komparedialogpages ${INSTALL_TARGETS_DEFAULT_ARGS})

