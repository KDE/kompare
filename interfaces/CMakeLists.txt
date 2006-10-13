



########### next target ###############

set(kompareinterface_LIB_SRCS kompareinterface.cpp )

kde4_automoc(kompareinterface ${kompareinterface_LIB_SRCS})

kde4_add_library(kompareinterface SHARED ${kompareinterface_LIB_SRCS})

target_link_libraries(kompareinterface  ${KDE4_KDECORE_LIBS} )

set_target_properties(kompareinterface PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kompareinterface  DESTINATION ${LIB_INSTALL_DIR} )


