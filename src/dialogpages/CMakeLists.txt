add_definitions(-DTRANSLATION_DOMAIN=\"kompare\")

add_library(komparedialogpages SHARED)

target_sources(komparedialogpages PRIVATE
    settingsbase.cpp
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
        KF6::Codecs
        KF6::Parts
        KF6::Completion
        KF6::KIOWidgets
        KF6::WidgetsAddons
        KF6::I18n
)

set_target_properties(komparedialogpages PROPERTIES
    VERSION   ${KOMPARE_LIB_VERSION}
    SOVERSION ${KOMPARE_LIB_SOVERSION}
)

install(TARGETS komparedialogpages ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)
