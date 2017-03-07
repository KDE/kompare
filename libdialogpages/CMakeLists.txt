
########### next target ###############

set(dialogpages_PART_SRCS
	filessettings.cpp
	viewsettings.cpp
	diffpage.cpp
	filespage.cpp
	viewpage.cpp )


add_library(komparedialogpages SHARED ${dialogpages_PART_SRCS})

generate_export_header(komparedialogpages BASE_NAME DIALOGPAGES)

target_link_libraries(komparedialogpages
    KompareDiff2
    KF5::Codecs
    KF5::Parts
    KF5::WidgetsAddons
)

set_target_properties(komparedialogpages PROPERTIES VERSION ${KOMPARE_LIB_VERSION}
SOVERSION ${KOMPARE_LIB_SOVERSION} )

install(TARGETS komparedialogpages ${INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)

