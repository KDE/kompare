
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../libdiff2 ${CMAKE_CURRENT_SOURCE_DIR}/../komparepart  )


########### next target ###############

set(komparenavtreepart_PART_SRCS komparenavtreepart.cpp )


kde4_add_plugin(komparenavtreepart ${komparenavtreepart_PART_SRCS})



target_link_libraries(komparenavtreepart  ${KDE4_KPARTS_LIBS} komparediff2 )

install(TARGETS komparenavtreepart  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES komparenavtreepart.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

