add_executable(kompare)

target_sources(kompare PRIVATE
    main.cpp
    kompare_shell.cpp
    kompareurldialog.cpp

    kompare_shell.qrc
)

ecm_qt_declare_logging_category(kompare
    HEADER kompareshelldebug.h
    IDENTIFIER KOMPARESHELL
    CATEGORY_NAME "kompare.shell"
    DESCRIPTION "Kompare App Shell"
    EXPORT kompare
)

ecm_add_app_icon(kompare ICONS
    ../icons/16-apps-kompare.png
    ../icons/22-apps-kompare.png
    ../icons/32-apps-kompare.png
    ../icons/48-apps-kompare.png
    ../icons/128-apps-kompare.png
)

target_link_libraries(kompare
    kompareinterface
    komparedialogpages
    KompareDiff2
    KF5::TextEditor
    KF5::WidgetsAddons
    Qt::Widgets
)

install(TARGETS kompare  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(PROGRAMS org.kde.kompare.desktop  DESTINATION  ${KDE_INSTALL_APPDIR})
install(FILES org.kde.kompare.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
