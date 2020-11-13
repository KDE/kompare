add_definitions(-DTRANSLATION_DOMAIN=\"kompare\")

set(komparenavtreepart_PART_SRCS komparenavtreepart.cpp )

ecm_qt_declare_logging_category(komparenavtreepart_PART_SRCS
    HEADER komparenavviewdebug.h
    IDENTIFIER KOMPARENAVVIEW
    CATEGORY_NAME "kompare.navview"
    DESCRIPTION "Kompare Navigation TreeView"
    EXPORT kompare
)

add_library(komparenavtreepart MODULE ${komparenavtreepart_PART_SRCS})
kcoreaddons_desktop_to_json(komparenavtreepart komparenavtreepart.desktop)

target_link_libraries(komparenavtreepart
    Qt5::Widgets
    KF5::Parts
    KF5::IconThemes
    KompareDiff2
)

install(TARGETS komparenavtreepart  DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/parts)
install(FILES komparenavtreepart.desktop  DESTINATION  ${KDE_INSTALL_KSERVICES5DIR})

