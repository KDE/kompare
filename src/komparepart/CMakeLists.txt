add_definitions(-DTRANSLATION_DOMAIN=\"kompare\")

configure_file(komparepart.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/komparepart.desktop @ONLY)

set( komparepart_PART_SRCS
     kompare_part.cpp
     kompare_partfactory.cpp
     kompareconnectwidget.cpp
     komparesplitter.cpp
     komparelistview.cpp
     kompareprefdlg.cpp
     komparesaveoptionsbase.cpp
     komparesaveoptionswidget.cpp
     kompareview.cpp )

ecm_qt_declare_logging_category(komparepart_PART_SRCS
    HEADER komparepartdebug.h
    IDENTIFIER KOMPAREPART
    CATEGORY_NAME "kompare.part"
    DESCRIPTION "Kompare KParts plugin"
    EXPORT kompare
)

ki18n_wrap_ui(komparepart_PART_SRCS komparesaveoptionsbase.ui )

qt5_add_resources(komparepart_PART_SRCS
    kompare_part.qrc
)

add_library(komparepart MODULE ${komparepart_PART_SRCS})

target_link_libraries(komparepart
    komparedialogpages
    kompareinterface
    KompareDiff2
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::JobWidgets
    Qt5::PrintSupport
)

install(TARGETS komparepart  DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/parts)
kcoreaddons_desktop_to_json(komparepart ${CMAKE_CURRENT_BINARY_DIR}/komparepart.desktop)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/komparepart.desktop  DESTINATION  ${KDE_INSTALL_KSERVICES5DIR})
