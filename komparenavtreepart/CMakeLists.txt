
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../komparepart)


########### next target ###############

set(komparenavtreepart_PART_SRCS komparenavtreepart.cpp )


add_library(komparenavtreepart MODULE ${komparenavtreepart_PART_SRCS})



target_link_libraries(komparenavtreepart
    Qt5::Widgets
    KF5::Parts
    KF5::IconThemes
    ${LIBKOMPAREDIFF2_LIBRARIES}
)

install(TARGETS komparenavtreepart  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES komparenavtreepart.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

