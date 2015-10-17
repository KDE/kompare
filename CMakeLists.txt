cmake_minimum_required(VERSION 2.8.12)
project(kompare)

# search packages used by KDE
include(FeatureSummary)
find_package(Qt5 REQUIRED COMPONENTS Core PrintSupport Widgets)

find_package(ECM 0.0.9 NO_MODULE REQUIRED)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(ECMInstallIcons)
include(GenerateExportHeader)

find_package(KF5 REQUIRED COMPONENTS
    CoreAddons
    Codecs
    DocTools
    IconThemes
    JobWidgets
    Config
    Parts
    TextEditor
    WidgetsAddons
)

find_package(LibKompareDiff2 REQUIRED)
include_directories (${LIBKOMPAREDIFF2_INCLUDE_DIR})
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

set(KOMPARE_LIB_VERSION 5)
set(KOMPARE_LIB_SOVERSION 5)

add_subdirectory( doc )
add_subdirectory( interfaces )
add_subdirectory( libdialogpages )
add_subdirectory( komparenavtreepart )
add_subdirectory( komparepart )
add_subdirectory( pics )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/komparepart
                     ${CMAKE_CURRENT_SOURCE_DIR}/libdialogpages
                     ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
                     ${CMAKE_CURRENT_SOURCE_DIR}/komparenavtreepart
                     ${CMAKE_CURRENT_SOURCE_DIR}/komparepart)


########### next target ###############

set(kompare_SRCS
main.cpp
kompare_shell.cpp
kompareurldialog.cpp
)

# kde4_add_app_icon(kompare_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/pics/hi*-app-kompare.png")

add_executable(kompare ${kompare_SRCS})

target_link_libraries(kompare
    kompareinterface
    ${LIBKOMPAREDIFF2_LIBRARIES}
    komparedialogpages
    KF5::TextEditor
    KF5::WidgetsAddons
    Qt5::Widgets
)

install(TARGETS kompare  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( PROGRAMS org.kde.kompare.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES kompareui.rc  DESTINATION  ${KXMLGUI_INSTALL_DIR}/kompare )
install( FILES komparenavigationpart.desktop kompareviewpart.desktop  DESTINATION  ${SERVICETYPES_INSTALL_DIR} )

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

