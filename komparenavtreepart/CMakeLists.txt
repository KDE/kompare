
include_directories( ${CMAKE_SOURCE_DIR}/kompare/libdiff2 ${CMAKE_SOURCE_DIR}/kompare/komparepart  )


########### next target ###############

set(komparenavtreepart_PART_SRCS komparenavtreepart.cpp )


kde4_add_plugin(komparenavtreepart WITH_PREFIX ${komparenavtreepart_PART_SRCS})



target_link_libraries(komparenavtreepart  ${KDE4_KPARTS_LIBS} komparediff2 ${KDE4_KDE3SUPPORT_LIBS} )

install(TARGETS komparenavtreepart  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES komparenavtreepart.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

