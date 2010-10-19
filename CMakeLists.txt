project(kompare)

# TODO port away from QT3_SUPPORT
add_definitions(-DQT3_SUPPORT)

add_subdirectory( interfaces )
add_subdirectory( libdiff2 )
add_subdirectory( libdialogpages )
add_subdirectory( komparenavtreepart )
add_subdirectory( komparepart )
add_subdirectory( pics )

include_directories(  ${CMAKE_CURRENT_SOURCE_DIR}/komparepart ${CMAKE_CURRENT_SOURCE_DIR}/libdiff2 ${CMAKE_CURRENT_SOURCE_DIR}/libdialogpages ${CMAKE_CURRENT_SOURCE_DIR}/interfaces ${CMAKE_CURRENT_SOURCE_DIR}/komparenavtreepart ${CMAKE_CURRENT_SOURCE_DIR}/komparepart)


########### next target ###############

set(kompare_SRCS
main.cpp
kompare_shell.cpp
kompareurldialog.cpp
)

kde4_add_app_icon(kompare_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/pics/hi*-app-kompare.png")

kde4_add_executable(kompare ${kompare_SRCS})

target_link_libraries(kompare  kompareinterface komparediff2 komparedialogpages ${KDE4_KTEXTEDITOR_LIBS} ${KDE4_KDE3SUPPORT_LIBS} )

install(TARGETS kompare  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( PROGRAMS kompare.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES kompareui.rc  DESTINATION  ${DATA_INSTALL_DIR}/kompare )
install( FILES komparenavigationpart.desktop kompareviewpart.desktop  DESTINATION  ${SERVICETYPES_INSTALL_DIR} )

