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
    KompareDiff2
    KF6::Parts
    KF6::IconThemes
    KF6::I18n
    Qt6::Widgets
)

install(TARGETS komparenavtreepart  DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/parts)

