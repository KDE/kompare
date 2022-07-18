/*
    SPDX-FileCopyrightText: 2001-2004, 2009 Otto Bruggeman <bruggie@gmail.com>
    SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2007-2011 Kevin Kofler <kevin.kofler@chello.at>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kompare_shell.h"

#include <QTextStream>
#include <QDockWidget>
#include <QEventLoopLocker>
#include <QFileDialog>
#include <QMimeDatabase>
#include <QPushButton>
#include <QStatusBar>

#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <KEditToolBar>
#include <KFile>
#include <KShortcutsDialog>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSqueezedTextLabel>
#include <KStandardAction>
#include <KServiceTypeTrader>
#include <KSharedConfig>
#include <KToggleAction>
#include <KActionCollection>
#include <KConfigGroup>

#include "kompareinterface.h"
#include "kompareurldialog.h"

#define ID_N_OF_N_DIFFERENCES      0
#define ID_N_OF_N_FILES            1
#define ID_GENERAL                 2

KompareShell::KompareShell()
    : KParts::MainWindow(),
      m_textViewPart(nullptr),
      m_textViewWidget(nullptr),
      m_eventLoopLocker(new QEventLoopLocker())
{
    resize(800, 480);

    // set the shell's ui resource file
    setXMLFile(QStringLiteral("kompareui.rc"));

    // then, setup our actions
    setupActions();
    setupStatusBar();

    const auto viewPartLoadResult = KPluginFactory::instantiatePlugin<KParts::ReadWritePart>(KPluginMetaData(QStringLiteral("kf5/parts/komparepart")), this);

    if (viewPartLoadResult)
    {
        m_viewPart = viewPartLoadResult.plugin;
        setCentralWidget(m_viewPart->widget());
        // and integrate the part's GUI with the shell's
        createGUI(m_viewPart);
    }
    else
    {
        // if we couldn't load our Part, we exit since the Shell by
        // itself can't do anything useful
        KMessageBox::error(this, i18n("Could not load our KompareViewPart: %1", viewPartLoadResult.errorString));
        exit(2);
    }

    m_navTreeDock = new QDockWidget(i18nc("@title:window", "Navigation"), this);
    m_navTreeDock->setObjectName(QStringLiteral("Navigation"));

    // This part is implemented in KompareNavTreePart

    const auto navPartLoadResult = KPluginFactory::instantiatePlugin<KParts::ReadOnlyPart>(KPluginMetaData(QStringLiteral("kf5/parts/komparenavtreepart")), m_navTreeDock);

    if (navPartLoadResult)
    {
        m_navTreePart = navPartLoadResult.plugin;
        m_navTreeDock->setWidget(m_navTreePart->widget());
        addDockWidget(Qt::TopDockWidgetArea, m_navTreeDock);
//          m_navTreeDock->manualDock( m_mainViewDock, KDockWidget::DockTop, 20 );
    }
    else
    {
        // if we couldn't load our Part, we exit since the Shell by
        // itself can't do anything useful
        KMessageBox::error(this, i18n("Could not load our KompareNavigationPart: %1", navPartLoadResult.errorString));
        exit(4);
    }

    // Hook up the inter part communication
    connect(m_viewPart, SIGNAL(modelsChanged(const Diff2::DiffModelList*)),
            m_navTreePart, SLOT(slotModelsChanged(const Diff2::DiffModelList*)));

    connect(m_viewPart, SIGNAL(kompareInfo(Kompare::Info*)),
            m_navTreePart, SLOT(slotKompareInfo(Kompare::Info*)));

    connect(m_navTreePart, SIGNAL(selectionChanged(const Diff2::DiffModel*,const Diff2::Difference*)),
            m_viewPart, SIGNAL(selectionChanged(const Diff2::DiffModel*,const Diff2::Difference*)));
    connect(m_viewPart, SIGNAL(setSelection(const Diff2::DiffModel*,const Diff2::Difference*)),
            m_navTreePart, SLOT(slotSetSelection(const Diff2::DiffModel*,const Diff2::Difference*)));

    connect(m_navTreePart, SIGNAL(selectionChanged(const Diff2::Difference*)),
            m_viewPart, SIGNAL(selectionChanged(const Diff2::Difference*)));
    connect(m_viewPart, SIGNAL(setSelection(const Diff2::Difference*)),
            m_navTreePart, SLOT(slotSetSelection(const Diff2::Difference*)));

    // This is the interpart interface, it is signal and slot based so no "real" nterface here
    // All you have to do is connect the parts from your application.
    // These just point to the method with the same name in the KompareModelList or get called
    // from the method with the same name in KompareModelList.

    // There is currently no applying possible from the navtreepart to the viewpart
    connect(m_viewPart, SIGNAL(applyDifference(bool)),
            m_navTreePart, SLOT(slotApplyDifference(bool)));
    connect(m_viewPart, SIGNAL(applyAllDifferences(bool)),
            m_navTreePart, SLOT(slotApplyAllDifferences(bool)));
    connect(m_viewPart, SIGNAL(applyDifference(const Diff2::Difference*,bool)),
            m_navTreePart, SLOT(slotApplyDifference(const Diff2::Difference*,bool)));

    // Hook up the KomparePart -> KompareShell communication
    connect(m_viewPart, SIGNAL(setStatusBarModelInfo(int,int,int,int,int)),
            this, SLOT(slotUpdateStatusBar(int,int,int,int,int)));
    connect(m_viewPart, SIGNAL(setStatusBarText(QString)),
            this, SLOT(slotSetStatusBarText(QString)));

    connect(m_viewPart, SIGNAL(diffString(QString)),
            this, SLOT(slotSetDiffString(QString)));

    // Read basic main-view settings, and set to autosave
    setAutoSaveSettings(QStringLiteral("General Options"));
}

KompareShell::~KompareShell()
{
    delete m_eventLoopLocker;
    m_eventLoopLocker = nullptr;
}

bool KompareShell::queryClose()
{
    bool rv = m_viewPart->queryClose();
    if (rv)
    {
        close();
    }
    return rv;
}

void KompareShell::openDiff(const QUrl& url)
{
    qCDebug(KOMPARESHELL) << "Url = " << url.toDisplayString();
    m_diffURL = url;
    viewPart()->openDiff(url);
}

void KompareShell::openStdin()
{
    qCDebug(KOMPARESHELL) << "Using stdin to read the diff" ;
    QFile file;
    file.open(stdin, QIODevice::ReadOnly);
    QTextStream stream(&file);

    QString diff = stream.readAll();

    file.close();

    viewPart()->openDiff(diff);

}

void KompareShell::compare(const QUrl& source, const QUrl& destination)
{
    m_sourceURL = source;
    m_destinationURL = destination;

    viewPart()->compare(source, destination);
}

void KompareShell::blend(const QUrl& url1, const QUrl& diff)
{
    m_sourceURL = url1;
    m_destinationURL = diff;

    viewPart()->openDirAndDiff(url1, diff);
}

void KompareShell::setupActions()
{
    QAction* a;
    a = KStandardAction::open(this, &KompareShell::slotFileOpen, actionCollection());
    a->setText(i18nc("@action", "&Open Diff..."));
    a = actionCollection()->addAction(QStringLiteral("file_compare_files"), this, &KompareShell::slotFileCompareFiles);
    a->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));
    a->setText(i18nc("@action", "&Compare Files..."));
    actionCollection()->setDefaultShortcut(a, QKeySequence(Qt::CTRL + Qt::Key_C));
    a = actionCollection()->addAction(QStringLiteral("file_blend_url"), this, &KompareShell::slotFileBlendURLAndDiff);
    a->setText(i18nc("@action", "&Blend URL with Diff..."));
    actionCollection()->setDefaultShortcut(a, QKeySequence(Qt::CTRL + Qt::Key_B));
    KStandardAction::quit(this, &KompareShell::slotFileClose, actionCollection());

    createStandardStatusBarAction();
    setStandardToolBarMenuEnabled(true);
    m_showTextView = new KToggleAction(i18nc("@action", "Show T&ext View"), this);
// needs a KGuiItem, also the doc says explicitly not to do this
//     m_showTextView->setCheckedState(i18n("Hide T&ext View"));
    actionCollection()->addAction(QStringLiteral("options_show_text_view"), m_showTextView);
    connect(m_showTextView, &KToggleAction::triggered, this, &KompareShell::slotShowTextView);

    KStandardAction::keyBindings(this, &KompareShell::optionsConfigureKeys, actionCollection());
    KStandardAction::configureToolbars(this, &KompareShell::optionsConfigureToolbars, actionCollection());
}

void KompareShell::setupStatusBar()
{
    // Made these entries permanent so they will appear on the right side
    m_differencesLabel = new QLabel(i18n(" 0 of 0 differences "));
    m_filesLabel = new QLabel(i18n(" 0 of 0 files "));
    statusBar()->insertPermanentWidget(ID_N_OF_N_DIFFERENCES, m_differencesLabel, 0);
    statusBar()->insertPermanentWidget(ID_N_OF_N_FILES, m_filesLabel, 0);

    m_generalLabel = new KSqueezedTextLabel(QString(), nullptr);
    statusBar()->addWidget(m_generalLabel, 1);
    m_generalLabel->setAlignment(Qt::AlignLeft);
}

void KompareShell::slotUpdateStatusBar(int modelIndex, int differenceIndex, int modelCount, int differenceCount, int appliedCount)
{
    qCDebug(KOMPARESHELL) << "KompareShell::updateStatusBar()" ;

    QString fileStr;
    QString diffStr;

    if (modelIndex >= 0)
        fileStr = i18np(" %2 of %1 file ", " %2 of %1 files ", modelCount, modelIndex + 1);
    else
        fileStr = i18np(" %1 file ", " %1 files ", modelCount);

    if (differenceIndex >= 0)
        diffStr = i18np(" %2 of %1 difference, %3 applied ", " %2 of %1 differences, %3 applied ", differenceCount ,
                        differenceIndex + 1, appliedCount);
    else
        diffStr = i18np(" %1 difference ", " %1 differences ", differenceCount);

    m_filesLabel->setText(fileStr);
    m_differencesLabel->setText(diffStr);
}

void KompareShell::slotSetStatusBarText(const QString& text)
{
    m_generalLabel->setText(text);
}

void KompareShell::saveProperties(KConfigGroup& config)
{
    // The 'config' object points to the session managed
    // config file.  Anything you write here will be available
    // later when this app is restored
    if (m_mode == Kompare::ComparingFiles)
    {
        config.writeEntry("Mode", "ComparingFiles");
        config.writePathEntry("SourceUrl", m_sourceURL.url());
        config.writePathEntry("DestinationUrl", m_destinationURL.url());
    }
    else if (m_mode == Kompare::ShowingDiff)
    {
        config.writeEntry("Mode", "ShowingDiff");
        config.writePathEntry("DiffUrl", m_diffURL.url());
    }

    viewPart()->saveProperties(config.config());
}

void KompareShell::readProperties(const KConfigGroup& config)
{
    // The 'config' object points to the session managed
    // config file. This function is automatically called whenever
    // the app is being restored. Read in here whatever you wrote
    // in 'saveProperties'

    QString mode = config.readEntry("Mode", "ComparingFiles");
    if (mode == QLatin1String("ComparingFiles"))
    {
        m_mode  = Kompare::ComparingFiles;
        m_sourceURL  = QUrl::fromLocalFile(config.readPathEntry("SourceUrl", QString()));
        m_destinationURL = QUrl::fromLocalFile(config.readPathEntry("DestinationFile", QString()));

        viewPart()->readProperties(const_cast<KConfig*>(config.config()));

        viewPart()->compareFiles(m_sourceURL, m_destinationURL);
    }
    else if (mode == QLatin1String("ShowingDiff"))
    {
        m_mode = Kompare::ShowingDiff;
        m_diffURL = QUrl::fromLocalFile(config.readPathEntry("DiffUrl", QString()));

        viewPart()->readProperties(const_cast<KConfig*>(config.config()));

        m_viewPart->openUrl(m_diffURL);
    }
    else
    {   // just in case something weird has happened, don't restore the diff then
        // Bruggie: or when some idiot like me changes the possible values for mode
        // IOW, a nice candidate for a kconf_update thingy :)
        viewPart()->readProperties(const_cast<KConfig*>(config.config()));
    }
}

void KompareShell::slotFileOpen()
{
    // FIXME: use different filedialog which gets encoding
    QUrl url = QFileDialog::getOpenFileUrl(this, QString(), QUrl(), QMimeDatabase().mimeTypeForName(QStringLiteral("text/x-patch")).filterString());
    if (!url.isEmpty()) {
        KompareShell* shell = new KompareShell();
        shell->show();
        shell->openDiff(url);
    }
}

void KompareShell::slotFileBlendURLAndDiff()
{
    KompareURLDialog dialog(this);

    dialog.setWindowTitle(i18nc("@title:window", "Blend File/Folder with diff Output"));
    dialog.setFirstGroupBoxTitle(i18nc("@title:group", "File/Folder"));
    dialog.setSecondGroupBoxTitle(i18nc("@title:group", "Diff Output"));

    QPushButton* okButton = dialog.button(QDialogButtonBox::Ok);
    okButton->setText(i18nc("@action:button", "Blend"));
    okButton->setToolTip(i18nc("@info:tooltip", "Blend this file or folder with the diff output"));
    okButton->setWhatsThis(i18nc("@infor:whatsthis", "If you have entered a file or folder name and a file that contains diff output in the fields in this dialog then this button will be enabled and pressing it will open kompare's main view where the output of the entered file or files from the folder are mixed with the diff output so you can then apply the difference(s) to a file or to the files. "));

    dialog.setGroup(QStringLiteral("Recent Blend Files"));

    dialog.setFirstURLRequesterMode(KFile::File | KFile::Directory | KFile::ExistingOnly);
    // diff output can not be a directory
    dialog.setSecondURLRequesterMode(KFile::File | KFile::ExistingOnly);
    if (dialog.exec() == QDialog::Accepted)
    {
        m_sourceURL = dialog.getFirstURL();
        m_destinationURL = dialog.getSecondURL();
        // Leak???
        KompareShell* shell = new KompareShell();
        shell->show();
        shell->viewPart()->setEncoding(dialog.encoding());
        shell->blend(m_sourceURL, m_destinationURL);
    }
}

void KompareShell::slotFileCompareFiles()
{
    KompareURLDialog dialog(this);

    dialog.setWindowTitle(i18nc("@title:window", "Compare Files or Folders"));
    dialog.setFirstGroupBoxTitle(i18nc("@title:group", "Source"));
    dialog.setSecondGroupBoxTitle(i18nc("@title:group", "Destination"));

    QPushButton* okButton = dialog.button(QDialogButtonBox::Ok);
    okButton->setText(i18nc("@action:button", "Compare"));
    okButton->setToolTip(i18nc("@info:tooltip", "Compare these files or folders"));
    okButton->setWhatsThis(i18nc("@info:whatsthis", "If you have entered 2 filenames or 2 folders in the fields in this dialog then this button will be enabled and pressing it will start a comparison of the entered files or folders. "));

    dialog.setGroup(QStringLiteral("Recent Compare Files"));

    dialog.setFirstURLRequesterMode(KFile::File | KFile::Directory | KFile::ExistingOnly);
    dialog.setSecondURLRequesterMode(KFile::File | KFile::Directory | KFile::ExistingOnly);

    if (dialog.exec() == QDialog::Accepted)
    {
        m_sourceURL = dialog.getFirstURL();
        m_destinationURL = dialog.getSecondURL();
        KompareShell* shell = new KompareShell();
        shell->show();
        qCDebug(KOMPARESHELL) << "The encoding is: " << dialog.encoding() ;
        shell->viewPart()->setEncoding(dialog.encoding());
        shell->compare(m_sourceURL, m_destinationURL);
    }
}

void KompareShell::slotFileClose()
{
    if (m_viewPart->queryClose())
    {
        close();
    }
}

void KompareShell::slotShowTextView()
{
    if (!m_textViewWidget)
    {
        QString error;

        // FIXME: proper error checking
        m_textViewWidget = new QDockWidget(i18nc("@title:window", "Text View"), this);
        m_textViewWidget->setObjectName(QStringLiteral("Text View"));
//         m_textViewWidget = createDockWidget(i18n("Text View"), SmallIcon("text-x-generic"));
        m_textViewPart = KServiceTypeTrader::createInstanceFromQuery<KTextEditor::Document>(
                             QStringLiteral("KTextEditor/Document"),
                             this, this, QString(), QVariantList(), &error);
        if (m_textViewPart)
        {
            m_textView = qobject_cast<KTextEditor::View*>(m_textViewPart->createView(this));
            m_textViewWidget->setWidget(static_cast<QWidget*>(m_textView));
            m_textViewPart->setHighlightingMode(QStringLiteral("Diff"));
            m_textViewPart->setText(m_diffString);
        }
        m_textViewWidget->show();
        connect(m_textViewWidget, &QDockWidget::visibilityChanged,
                this, &KompareShell::slotVisibilityChanged);
    }
    else if (m_textViewWidget->isVisible())
        m_textViewWidget->hide();
    else
        m_textViewWidget->show();

    addDockWidget(Qt::BottomDockWidgetArea, m_textViewWidget);
//     m_textViewWidget->manualDock(m_mainViewDock, KDockWidget:: DockCenter);
}

void KompareShell::slotVisibilityChanged(bool visible)
{
    m_showTextView->setChecked(visible);
}

void KompareShell::slotSetDiffString(const QString& diffString)
{
    if (m_textViewPart)
        m_textViewPart->setText(diffString);
    m_diffString = diffString;
}

void KompareShell::optionsConfigureKeys()
{
    KShortcutsDialog dlg(KShortcutsEditor::AllActions, KShortcutsEditor::LetterShortcutsAllowed, this);

    dlg.addCollection(actionCollection());
    if (m_viewPart)
        dlg.addCollection(m_viewPart->actionCollection());

    dlg.configure();
}

void KompareShell::optionsConfigureToolbars()
{
    KConfigGroup group(KSharedConfig::openConfig(), autoSaveGroup());
    saveMainWindowSettings(group);
    // use the standard toolbar editor
    KEditToolBar dlg(factory());
    connect(&dlg, &KEditToolBar::newToolBarConfig, this, &KompareShell::newToolbarConfig);
    dlg.exec();
}

void KompareShell::newToolbarConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), autoSaveGroup());
    applyMainWindowSettings(group);
}

KompareInterface* KompareShell::viewPart() const
{
    return qobject_cast<KompareInterface*>(m_viewPart);
}
