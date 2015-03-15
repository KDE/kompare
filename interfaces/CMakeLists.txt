



########### next target ###############

set(kompareinterface_LIB_SRCS kompareinterface.cpp )


add_library(kompareinterface SHARED ${kompareinterface_LIB_SRCS})

generate_export_header(kompareinterface BASE_NAME KOMPAREINTERFACE)

target_link_libraries(kompareinterface
    Qt5::Core
)

set_target_properties(kompareinterface PROPERTIES VERSION ${KOMPARE_LIB_VERSION} SOVERSION ${KOMPARE_LIB_SOVERSION} )
install(TARGETS kompareinterface ${INSTALL_TARGETS_DEFAULT_ARGS} )
install(FILES kompareinterface.h DESTINATION ${INCLUDE_INSTALL_DIR}/kompare COMPONENT Devel )


