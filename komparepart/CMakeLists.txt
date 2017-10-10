add_definitions(-DTRANSLATION_DOMAIN=\"kompare\")

set( komparepart_PART_SRCS
     kompare_part.cpp
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
    CATEGORY_NAME "komparepart"
)

ki18n_wrap_ui(komparepart_PART_SRCS komparesaveoptionsbase.ui )

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

install(TARGETS komparepart  DESTINATION ${KDE_INSTALL_PLUGINDIR})

install(FILES komparepart.desktop  DESTINATION  ${KDE_INSTALL_KSERVICES5DIR})
install(FILES komparepartui.rc  DESTINATION  ${KDE_INSTALL_KXMLGUI5DIR}/komparepart)
