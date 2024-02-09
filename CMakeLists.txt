cmake_minimum_required(VERSION 3.16)

# Release Service version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "23")
set (RELEASE_SERVICE_VERSION_MINOR "08")
set (RELEASE_SERVICE_VERSION_MICRO "5")
set (RELEASE_SERVICE_COMPACT_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}${RELEASE_SERVICE_VERSION_MINOR}${RELEASE_SERVICE_VERSION_MICRO}")

# Bump KOMPARE_BASE_VERSION once new features are added
set(KOMPARE_BASE_VERSION "4.1")
set(KOMPARE_VERSION "${KOMPARE_BASE_VERSION}.${RELEASE_SERVICE_COMPACT_VERSION}")

project(Kompare VERSION ${KOMPARE_VERSION})

set(QT_MIN_VERSION "5.15.2")
set(KF_MIN_VERSION "5.100.0")

find_package(ECM ${KF_MIN_VERSION} NO_MODULE REQUIRED)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMInstallIcons)
include(ECMAddAppIcon)
include(ECMQtDeclareLoggingCategory)
include(ECMSetupVersion)
include(ECMDeprecationSettings)
include(GenerateExportHeader)
include(FeatureSummary)


if (QT_MAJOR_VERSION STREQUAL "6")
    set(QT_MIN_VERSION "6.5.0")
    set(KF_MIN_VERSION "5.240.0")
    set(KF_MAJOR_VERSION "6")
else()
    set(KF_MAJOR_VERSION "5")
endif()

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} REQUIRED COMPONENTS
    Core
    PrintSupport
    Widgets
)

find_package(KF${KF_MAJOR_VERSION} ${KF_MIN_VERSION} REQUIRED COMPONENTS
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

find_package(LibKompareDiff2 5.4 CONFIG)
set_package_properties(LibKompareDiff2 PROPERTIES
    TYPE REQUIRED
)


set(KOMPARE_LIB_VERSION 5)
set(KOMPARE_LIB_SOVERSION 5)

ecm_set_disabled_deprecation_versions(
    QT 5.15.2
    KF 5.100
    KOMPAREDIFF2 5.4
)

# added by KDE_COMPILERSETTINGS_LEVEL 5.85.0
remove_definitions(
    -DQT_NO_URL_CAST_FROM_STRING
)

add_subdirectory(doc)
add_subdirectory(src)

ki18n_install(po)
kdoctools_install(po)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
