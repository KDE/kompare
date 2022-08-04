ecm_setup_version(${Kompare_VERSION}
    VARIABLE_PREFIX KOMPARE
    VERSION_HEADER kompare_version.h
)

add_subdirectory(icons)
add_subdirectory(servicemenus)

add_subdirectory(interfaces)
add_subdirectory(dialogpages)
add_subdirectory(komparenavtreepart)
add_subdirectory(komparepart)
add_subdirectory(app)


ecm_qt_install_logging_categories(
    EXPORT kompare
    FILE kompare.categories
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)

install(FILES
    komparenavigationpart.desktop
    kompareviewpart.desktop
    DESTINATION  ${KDE_INSTALL_KSERVICETYPESDIR}
)
