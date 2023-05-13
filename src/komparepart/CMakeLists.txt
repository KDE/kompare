add_definitions(-DTRANSLATION_DOMAIN=\"kompare\")

configure_file(komparepart.json.in ${CMAKE_CURRENT_BINARY_DIR}/komparepart.json @ONLY)

add_library(komparepart MODULE)

target_sources(komparepart PRIVATE
    kompare_part.cpp
    kompare_partfactory.cpp
    kompareconnectwidget.cpp
    komparesplitter.cpp
    komparelistview.cpp
    kompareprefdlg.cpp
    komparesaveoptionsbase.cpp
    komparesaveoptionswidget.cpp
    kompareview.cpp

    kompare_part.qrc
)

ecm_qt_declare_logging_category(komparepart
    HEADER komparepartdebug.h
    IDENTIFIER KOMPAREPART
    CATEGORY_NAME "kompare.part"
    DESCRIPTION "Kompare KParts plugin"
    EXPORT kompare
)

ki18n_wrap_ui(komparepart komparesaveoptionsbase.ui)

target_link_libraries(komparepart
    komparedialogpages
    kompareinterface
    KompareDiff2
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::JobWidgets
    Qt::PrintSupport
)

install(TARGETS komparepart  DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf${QT_MAJOR_VERSION}/parts)
