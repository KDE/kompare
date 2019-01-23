cmake_minimum_required(VERSION 3.0)

project(kompare)

set(ECM_MIN_VERSION "5.35.0")
find_package(ECM ${ECM_MIN_VERSION} NO_MODULE REQUIRED)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMInstallIcons)
include(GenerateExportHeader)
include(ECMAddAppIcon)
include(ECMQtDeclareLoggingCategory)

set(QT_MIN_VERSION "5.5.0")
find_package(Qt5 ${QT_MIN_VERSION} REQUIRED COMPONENTS
    Core
    PrintSupport
    Widgets
)

set(KF5_MIN_VERSION "5.28.0")
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
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

find_package(LibKompareDiff2 5.2 CONFIG)
set_package_properties(LibKompareDiff2 PROPERTIES
    TYPE REQUIRED
)


set(KOMPARE_LIB_VERSION 5)
set(KOMPARE_LIB_SOVERSION 5)

add_definitions(
    -DQT_DEPRECATED_WARNINGS
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050500
    -DQT_NO_SIGNALS_SLOTS_KEYWORDS
    -DQT_STRICT_ITERATORS
    -DQT_USE_QSTRINGBUILDER
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
#     -DQT_NO_URL_CAST_FROM_STRING
)

add_subdirectory( doc )
add_subdirectory( interfaces )
add_subdirectory( libdialogpages )
add_subdirectory( komparenavtreepart )
add_subdirectory( komparepart )
add_subdirectory( pics )
add_subdirectory( servicemenus )

set(kompare_SRCS
    main.cpp
    kompare_shell.cpp
    kompareurldialog.cpp
)
ecm_qt_declare_logging_category(kompare_SRCS
    HEADER kompareshelldebug.h
    IDENTIFIER KOMPARESHELL
    CATEGORY_NAME "kompareshell"
)

ecm_add_app_icon(kompare_SRCS ICONS
    pics/16-apps-kompare.png
    pics/22-apps-kompare.png
    pics/32-apps-kompare.png
    pics/48-apps-kompare.png
    pics/128-apps-kompare.png
)

add_executable(kompare ${kompare_SRCS})

target_link_libraries(kompare
    kompareinterface
    komparedialogpages
    KompareDiff2
    KF5::TextEditor
    KF5::WidgetsAddons
    Qt5::Widgets
)

install(TARGETS kompare  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(PROGRAMS org.kde.kompare.desktop  DESTINATION  ${KDE_INSTALL_APPDIR})
install(FILES org.kde.kompare.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES kompareui.rc  DESTINATION  ${KDE_INSTALL_KXMLGUI5DIR}/kompare)
install(FILES komparenavigationpart.desktop kompareviewpart.desktop  DESTINATION  ${KDE_INSTALL_KSERVICETYPES5DIR})

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
