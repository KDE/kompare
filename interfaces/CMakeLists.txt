



########### next target ###############

set(kompareinterface_LIB_SRCS kompareinterface.cpp )


kde4_add_library(kompareinterface SHARED ${kompareinterface_LIB_SRCS})

target_link_libraries(kompareinterface  ${KDE4_KDECORE_LIBS} )

set_target_properties(kompareinterface PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kompareinterface ${INSTALL_TARGETS_DEFAULT_ARGS} )
install(FILES kompareinterface.h DESTINATION ${INCLUDE_INSTALL_DIR}/kompare COMPONENT Devel )


