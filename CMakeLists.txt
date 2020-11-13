cmake_minimum_required(VERSION 3.0)

# Release Service version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "21")
set (RELEASE_SERVICE_VERSION_MINOR "03")
set (RELEASE_SERVICE_VERSION_MICRO "70")
set (RELEASE_SERVICE_COMPACT_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}${RELEASE_SERVICE_VERSION_MINOR}${RELEASE_SERVICE_VERSION_MICRO}")

# Bump KOMPARE_BASE_VERSION once new features are added
set(KOMPARE_BASE_VERSION "4.1")
set(KOMPARE_VERSION "${KOMPARE_BASE_VERSION}.${RELEASE_SERVICE_COMPACT_VERSION}")

project(Kompare VERSION ${KOMPARE_VERSION})

set(QT_MIN_VERSION "5.12.0")
set(KF_MIN_VERSION "5.68.0")

find_package(ECM ${KF5_MIN_VERSION} NO_MODULE REQUIRED)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMInstallIcons)
include(GenerateExportHeader)
include(ECMAddAppIcon)
include(ECMQtDeclareLoggingCategory)
include(ECMSetupVersion)

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED COMPONENTS
    Core
    PrintSupport
    Widgets
)

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
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050F00
    -DQT_DEPRECATED_WARNINGS_SINCE=0x060000
    -DQT_NO_KEYWORDS
    -DQT_STRICT_ITERATORS
    -DQT_USE_QSTRINGBUILDER
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
#     -DQT_NO_URL_CAST_FROM_STRING
    -DQT_NO_FOREACH
    -DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT
    -DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x054700
    -DKF_DEPRECATED_WARNINGS_SINCE=0x060000
)

ecm_setup_version(${Kompare_VERSION}
    VARIABLE_PREFIX KOMPARE
    VERSION_HEADER kompare_version.h
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
    CATEGORY_NAME "kompare.shell"
    DESCRIPTION "Kompare App Shell"
    EXPORT kompare
)

qt5_add_resources(kompare_SRCS
    kompare_shell.qrc
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
install(FILES komparenavigationpart.desktop kompareviewpart.desktop  DESTINATION  ${KDE_INSTALL_KSERVICETYPES5DIR})

ecm_qt_install_logging_categories(
    EXPORT kompare
    FILE kompare.categories
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)

ki18n_install(po)
kdoctools_install(po)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
