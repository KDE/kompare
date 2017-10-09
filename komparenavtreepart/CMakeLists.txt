add_definitions(-DTRANSLATION_DOMAIN=\"kompare\")

set(komparenavtreepart_PART_SRCS komparenavtreepart.cpp )

add_library(komparenavtreepart MODULE ${komparenavtreepart_PART_SRCS})

target_link_libraries(komparenavtreepart
    Qt5::Widgets
    KF5::Parts
    KF5::IconThemes
    KompareDiff2
)

install(TARGETS komparenavtreepart  DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES komparenavtreepart.desktop  DESTINATION  ${KDE_INSTALL_KSERVICES5DIR})

