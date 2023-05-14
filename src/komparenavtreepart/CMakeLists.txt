add_definitions(-DTRANSLATION_DOMAIN=\"kompare\")

configure_file(komparenavtreepart.json.in ${CMAKE_CURRENT_BINARY_DIR}/komparenavtreepart.json @ONLY)

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

target_link_libraries(komparenavtreepart
    Qt::Widgets
    KF${KF_MAJOR_VERSION}::Parts
    KF${KF_MAJOR_VERSION}::IconThemes
    KompareDiff2
)

install(TARGETS komparenavtreepart  DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf${QT_MAJOR_VERSION}/parts)

