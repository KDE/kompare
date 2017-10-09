add_definitions(-DTRANSLATION_DOMAIN=\"kompare\")

set(kompareinterface_LIB_SRCS kompareinterface.cpp )

add_library(kompareinterface SHARED ${kompareinterface_LIB_SRCS})

generate_export_header(kompareinterface BASE_NAME KOMPAREINTERFACE)

target_link_libraries(kompareinterface
    PUBLIC
        Qt5::Core
)

set_target_properties(kompareinterface PROPERTIES VERSION ${KOMPARE_LIB_VERSION} SOVERSION ${KOMPARE_LIB_SOVERSION} )
install(TARGETS kompareinterface ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES kompareinterface.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kompare COMPONENT Devel)


