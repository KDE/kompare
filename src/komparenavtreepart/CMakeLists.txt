add_definitions(-DTRANSLATION_DOMAIN=\"kompare\")

configure_file(komparenavtreepart.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/komparenavtreepart.desktop @ONLY)

add_library(komparenavtreepart MODULE)

target_sources(komparenavtreepart PRIVATE
    komparenavtreepart.cpp
)

ecm_qt_declare_logging_category(komparenavtreepart
    HEADER komparenavviewdebug.h
    IDENTIFIER KOMPARENAVVIEW
    CATEGORY_NAME "kompare.navview"
    DESCRIPTION "Kompare Navigation TreeView"
    EXPORT kompare
)

kcoreaddons_desktop_to_json(komparenavtreepart ${CMAKE_CURRENT_BINARY_DIR}/komparenavtreepart.desktop)

target_link_libraries(komparenavtreepart
    Qt::Widgets
    KF5::Parts
    KF5::IconThemes
    KompareDiff2
)

install(TARGETS komparenavtreepart  DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf${QT_MAJOR_VERSION}/parts)

