add_definitions(-DTRANSLATION_DOMAIN=\"kompare\")

add_library(komparedialogpages SHARED)

target_sources(komparedialogpages PRIVATE
    filessettings.cpp
    viewsettings.cpp
    diffpage.cpp
    filespage.cpp
    viewpage.cpp
)

generate_export_header(komparedialogpages BASE_NAME DIALOGPAGES)

target_link_libraries(komparedialogpages
    PUBLIC
        KompareDiff2
        KF${KF_MAJOR_VERSION}::Codecs
        KF${KF_MAJOR_VERSION}::Parts
        KF${KF_MAJOR_VERSION}::Completion
        KF${KF_MAJOR_VERSION}::KIOWidgets
        KF${KF_MAJOR_VERSION}::WidgetsAddons
	KF${KF_MAJOR_VERSION}::I18n
)

set_target_properties(komparedialogpages PROPERTIES
    VERSION   ${KOMPARE_LIB_VERSION}
    SOVERSION ${KOMPARE_LIB_SOVERSION}
)

install(TARGETS komparedialogpages ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)
