/***************************************************************************
                                kompare_part.cpp
                                ----------------
        begin                   : Sun Mar 4 2001
        Copyright 2001-2005,2009 Otto Bruggeman <bruggie@gmail.com>
        Copyright 2001-2003 John Firebaugh <jfirebaugh@kde.org>
        Copyright 2004      Jeff Snyder    <jeff@caffeinated.me.uk>
        Copyright 2007-2011 Kevin Kofler   <kevin.kofler@chello.at>
        Copyright 2012     Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>
****************************************************************************/

/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***************************************************************************/

#include "kompare_part.h"

#include <QDialog>
#include <QLayout>
#include <QWidget>
#include <QMenu>
#include <QPainter>
#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QTemporaryDir>
#include <QTemporaryFile>

#include <KAboutData>
#include <KActionCollection>
#include <KJobWidgets>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>
#include <KStandardAction>
#include <KStandardShortcut>
#include <KStandardGuiItem>
#include <KXMLGUIFactory>

#include <KIO/CopyJob>
#include <KIO/StatJob>
#include <KIO/FileCopyJob>
#include <KIO/MkdirJob>

#include <libkomparediff2/diffmodel.h>
#include <libkomparediff2/diffsettings.h>

#include <komparepartdebug.h>
#include "komparelistview.h"
#include "kompareconnectwidget.h"
#include "viewsettings.h"
#include "kompareprefdlg.h"
#include "komparesaveoptionswidget.h"
#include "komparesplitter.h"
#include "kompareview.h"

using namespace Diff2;

ViewSettings* KomparePart::m_viewSettings = nullptr;
DiffSettings* KomparePart::m_diffSettings = nullptr;

KomparePart::KomparePart(QWidget* parentWidget, QObject* parent, const KAboutData& aboutData, Modus modus) :
    KParts::ReadWritePart(parent),
    m_info()
{
    setComponentData(aboutData);

    // set our XML-UI resource file
    setXMLFile(QStringLiteral("komparepartui.rc"));

    if (!m_viewSettings) {
        m_viewSettings = new ViewSettings(nullptr);
    }
    if (!m_diffSettings) {
        m_diffSettings = new DiffSettings(nullptr);
    }

    readProperties(KSharedConfig::openConfig().data());

    m_view = new KompareView(m_viewSettings, parentWidget);
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_view, &QWidget::customContextMenuRequested,
            this, &KomparePart::onContextMenuRequested);
    setWidget(m_view);
    m_splitter = m_view->splitter();

    // This creates the "Model creator" and connects the signals and slots
    m_modelList = new Diff2::KompareModelList(m_diffSettings, m_splitter, this, "komparemodellist" , (modus == ReadWriteModus));

    const auto modelListActions = m_modelList->actionCollection()->actions();
    for (QAction* action : modelListActions) {
        actionCollection()->addAction(action->objectName(), action);
    }
    connect(m_modelList, &KompareModelList::status,
            this, &KomparePart::slotSetStatus);
    connect(m_modelList, &KompareModelList::setStatusBarModelInfo,
            this, &KomparePart::setStatusBarModelInfo);
    connect(m_modelList, &KompareModelList::error,
            this, &KomparePart::slotShowError);
    connect(m_modelList, &KompareModelList::applyAllDifferences,
            this, &KomparePart::updateActions);
    connect(m_modelList, static_cast<void(KompareModelList::*)(bool)>(&KompareModelList::applyDifference),
            this, &KomparePart::updateActions);
    connect(m_modelList, &KompareModelList::applyAllDifferences,
            this, &KomparePart::appliedChanged);
    connect(m_modelList, static_cast<void(KompareModelList::*)(bool)>(&KompareModelList::applyDifference),
            this, &KomparePart::appliedChanged);
    connect(m_modelList, &KompareModelList::updateActions, this, &KomparePart::updateActions);

    // This is the stuff to connect the "interface" of the kompare part to the model inside
    connect(m_modelList, &KompareModelList::modelsChanged,
            this, &KomparePart::modelsChanged);

    typedef void(KompareModelList::*void_KompareModelList_argModelDiff)(const DiffModel*, const Difference*);
    typedef void(KompareModelList::*void_KompareModelList_argDiffBool)(const Difference*, bool);
    typedef void(KompareSplitter::*void_KompareSplitter_argModelDiff)(const DiffModel*, const Difference*);
    typedef void(KompareSplitter::*void_KompareSplitter_argDiffBool)(const Difference*, bool);
    typedef void(KomparePart::*void_KomparePart_argModelDiff)(const DiffModel*, const Difference*);
    typedef void(KomparePart::*void_KomparePart_argDiffBool)(const Difference*, bool);
    connect(m_modelList, static_cast<void_KompareModelList_argModelDiff>(&KompareModelList::setSelection),
            this, static_cast<void_KomparePart_argModelDiff>(&KomparePart::setSelection));
    connect(this, static_cast<void_KomparePart_argModelDiff>(&KomparePart::selectionChanged),
            m_modelList, static_cast<void_KompareModelList_argModelDiff>(&KompareModelList::slotSelectionChanged));

    connect(m_modelList, static_cast<void(KompareModelList::*)(const Difference*)>(&KompareModelList::setSelection),
            this, static_cast<void(KomparePart::*)(const Difference*)>(&KomparePart::setSelection));
    connect(this, static_cast<void(KomparePart::*)(const Difference*)>(&KomparePart::selectionChanged),
            m_modelList, static_cast<void(KompareModelList::*)(const Difference*)>(&KompareModelList::slotSelectionChanged));

    connect(m_modelList, static_cast<void(KompareModelList::*)(bool)>(&KompareModelList::applyDifference),
            this, static_cast<void(KomparePart::*)(bool)>(&KomparePart::applyDifference));
    connect(m_modelList, &KompareModelList::applyAllDifferences,
            this, &KomparePart::applyAllDifferences);
    connect(m_modelList, static_cast<void_KompareModelList_argDiffBool>(&KompareModelList::applyDifference),
            this, static_cast<void_KomparePart_argDiffBool>(&KomparePart::applyDifference));
    connect(m_modelList, &KompareModelList::diffString,
            this, &KomparePart::diffString);

    connect(this, &KomparePart::kompareInfo, m_modelList, &KompareModelList::slotKompareInfo);

    // Here we connect the splitter to the modellist
    connect(m_modelList, static_cast<void_KompareModelList_argModelDiff>(&KompareModelList::setSelection),
            m_splitter,  static_cast<void_KompareSplitter_argModelDiff>(&KompareSplitter::slotSetSelection));
//     connect(m_splitter,  SIGNAL(selectionChanged(const Diff2::Difference*,const Diff2::Difference*)),
//             m_modelList, SLOT(slotSelectionChanged(const Diff2::Difference*,const Diff2::Difference*)));
    connect(m_modelList, static_cast<void(KompareModelList::*)(const Difference*)>(&KompareModelList::setSelection),
            m_splitter,  static_cast<void(KompareSplitter::*)(const Difference*)>(&KompareSplitter::slotSetSelection));
    connect(m_splitter,  static_cast<void(KompareSplitter::*)(const Difference*)>(&KompareSplitter::selectionChanged),
            m_modelList, static_cast<void(KompareModelList::*)(const Difference*)>(&KompareModelList::slotSelectionChanged));

    connect(m_modelList, static_cast<void(KompareModelList::*)(bool)>(&KompareModelList::applyDifference),
            m_splitter, static_cast<void(KompareSplitter::*)(bool)>(&KompareSplitter::slotApplyDifference));
    connect(m_modelList, &KompareModelList::applyAllDifferences,
            m_splitter, &KompareSplitter::slotApplyAllDifferences);
    connect(m_modelList, static_cast<void_KompareModelList_argDiffBool>(&KompareModelList::applyDifference),
            m_splitter, static_cast<void_KompareSplitter_argDiffBool>(&KompareSplitter::slotApplyDifference));
    connect(this, &KomparePart::configChanged, m_splitter, &KompareSplitter::configChanged);

    setupActions(modus);

    // we are read-write by default -> uhm what if we are opened by lets say konq in RO mode ?
    // Then we should not be doing this...
    setReadWrite((modus == ReadWriteModus));

    // we are not modified since we haven't done anything yet
    setModified(false);
}

KomparePart::~KomparePart()
{
    // This is the only place allowed to call cleanUpTemporaryFiles
    // because before there might still be a use for them (when swapping)
    cleanUpTemporaryFiles();
}

void KomparePart::setupActions(Modus modus)
{
    // create our actions

    if (modus == ReadWriteModus) {
        m_saveAll = actionCollection()->addAction(QStringLiteral("file_save_all"), this, &KomparePart::saveAll);
        m_saveAll->setIcon(QIcon::fromTheme(QStringLiteral("document-save-all")));
        m_saveAll->setText(i18nc("@action", "Save &All"));
        m_saveDiff = actionCollection()->addAction(QStringLiteral("file_save_diff"), this, &KomparePart::saveDiff);
        m_saveDiff->setText(i18nc("@action", "Save &Diff..."));
        m_swap = actionCollection()->addAction(QStringLiteral("file_swap"), this, &KomparePart::slotSwap);
        m_swap->setText(i18nc("@action", "Swap Source with Destination"));
    } else {
        m_saveAll = nullptr;
        m_saveDiff = nullptr;
        m_swap = nullptr;
    }
    m_diffStats = actionCollection()->addAction(QStringLiteral("file_diffstats"), this, &KomparePart::slotShowDiffstats);
    m_diffStats->setText(i18nc("@action", "Show Statistics"));
    m_diffRefresh = actionCollection()->addAction(QStringLiteral("file_refreshdiff"), this, &KomparePart::slotRefreshDiff);
    m_diffRefresh->setIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));
    m_diffRefresh->setText(i18nc("@action", "Refresh Diff"));
    actionCollection()->setDefaultShortcuts(m_diffRefresh, KStandardShortcut::reload());

    m_print        = KStandardAction::print(this, &KomparePart::slotFilePrint, actionCollection());
    m_printPreview = KStandardAction::printPreview(this, &KomparePart::slotFilePrintPreview, actionCollection());
    KStandardAction::preferences(this, &KomparePart::optionsPreferences, actionCollection());
}

void KomparePart::updateActions()
{
    if (m_saveAll) m_saveAll->setEnabled(m_modelList->hasUnsavedChanges());
    if (m_saveDiff) m_saveDiff->setEnabled(m_modelList->mode() == Kompare::ComparingFiles || m_modelList->mode() == Kompare::ComparingDirs);
    if (m_swap) m_swap->setEnabled(m_modelList->mode() == Kompare::ComparingFiles || m_modelList->mode() == Kompare::ComparingDirs);
    m_diffRefresh->setEnabled(m_modelList->mode() == Kompare::ComparingFiles || m_modelList->mode() == Kompare::ComparingDirs);
    m_diffStats->setEnabled(m_modelList->modelCount() > 0);
    m_print->setEnabled(m_modelList->modelCount() > 0);          // If modellist has models then we have something to print, it's that simple.
    m_printPreview->setEnabled(m_modelList);
}

void KomparePart::setEncoding(const QString& encoding)
{
    qCDebug(KOMPAREPART) << "Encoding: " << encoding;
    m_modelList->setEncoding(encoding);
}

bool KomparePart::openDiff(const QUrl& url)
{
    qCDebug(KOMPAREPART) << "Url = " << url.url();

    m_info.mode = Kompare::ShowingDiff;
    m_info.source = url;
    bool result = false;
    fetchURL(url, true);

    emit kompareInfo(&m_info);

    if (!m_info.localSource.isEmpty())
    {
        qCDebug(KOMPAREPART) << "Download succeeded ";
        result = m_modelList->openDiff(m_info.localSource);
        updateActions();
        updateCaption();
        updateStatus();
    }
    else
    {
        qCDebug(KOMPAREPART) << "Download failed !";
    }

    return result;
}

bool KomparePart::openDiff(const QString& diffOutput)
{
    bool value = false;

    m_info.mode = Kompare::ShowingDiff;

    emit kompareInfo(&m_info);

    if (m_modelList->parseAndOpenDiff(diffOutput) == 0)
    {
        value = true;
        updateActions();
        updateCaption();
        updateStatus();
    }

    return value;
}

bool KomparePart::openDiff3(const QUrl& diff3Url)
{
    // FIXME: Implement this !!!
    qCDebug(KOMPAREPART) << "Not implemented yet. Filename is: " << diff3Url;
    return false;
}

bool KomparePart::openDiff3(const QString& diff3Output)
{
    // FIXME: Implement this !!!
    qCDebug(KOMPAREPART) << "Not implemented yet. diff3 output is: ";
    qCDebug(KOMPAREPART) << diff3Output;
    return false;
}

bool KomparePart::fetchURL(const QUrl& url, bool addToSource)
{
    // Default value if there is an error is "", we rely on it!
    QString tempFileName;
    // Only in case of error do we set result to false, don't forget!!
    bool result = true;
    QTemporaryDir* tmpDir = nullptr;

    if (!url.isLocalFile())
    {
        KIO::StatJob* statJob = KIO::stat(url);
        KJobWidgets::setWindow(statJob, widget());
        if (statJob->exec())
        {
            KIO::UDSEntry node;
            node = statJob->statResult();
            if (!node.isDir())
            {
                tmpDir = new QTemporaryDir(QDir::tempPath() + QLatin1String("/kompare"));
                tmpDir->setAutoRemove(true);
                tempFileName = tmpDir->path() + QLatin1Char('/') + url.fileName();
                KIO::FileCopyJob* copyJob = KIO::file_copy(url, QUrl::fromLocalFile(tempFileName));
                KJobWidgets::setWindow(copyJob, widget());
                if (! copyJob->exec())
                {
                    qDebug() << "download error " << copyJob->errorString();
                    slotShowError(i18n("<qt>The URL <b>%1</b> cannot be downloaded.</qt>", url.toDisplayString()));
                    tempFileName.clear(); // Not sure if download has already touched this tempFileName when there is an error
                    result = false;
                }
            }
            else
            {
                tmpDir = new QTemporaryDir(QDir::tempPath() + QLatin1String("/kompare"));
                tmpDir->setAutoRemove(true);   // Yes this is the default but just to make sure
                KIO::CopyJob* copyJob = KIO::copy(url, QUrl::fromLocalFile(tmpDir->path()));
                KJobWidgets::setWindow(copyJob, widget());
                if (! copyJob->exec())
                {
                    slotShowError(i18n("<qt>The URL <b>%1</b> cannot be downloaded.</qt>", url.toDisplayString()));
                    delete tmpDir;
                    tmpDir = nullptr;
                    result = false;
                }
                else
                {
                    tempFileName = tmpDir->path();
                    qCDebug(KOMPAREPART) << "tempFileName = " << tempFileName;
                    // If a directory is copied into QTemporaryDir then the directory in
                    // here is what I need to add to tempFileName
                    QDir dir(tempFileName);
                    QStringList entries = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
                    if (entries.size() == 1)   // More than 1 entry in here means big problems!!!
                    {
                        if (!tempFileName.endsWith(QLatin1Char('/')))
                            tempFileName += QLatin1Char('/');
                        tempFileName += entries.at(0);
                        tempFileName += QLatin1Char('/');
                    }
                    else
                    {
                        qCDebug(KOMPAREPART) << "Yikes, nothing downloaded?";
                        delete tmpDir;
                        tmpDir = nullptr;
                        tempFileName.clear();
                        result = false;
                    }
                }
            }
        }
    }
    else
    {
        // is Local already, check if exists
        if (QFile::exists(url.toLocalFile())) {
            tempFileName = url.toLocalFile();
        } else {
            slotShowError(i18n("<qt>The URL <b>%1</b> does not exist on your system.</qt>", url.toDisplayString()));
            result = false;
        }
    }

    if (addToSource)
    {
        m_info.localSource = tempFileName;
        m_info.sourceQTempDir = tmpDir;
    }
    else
    {
        m_info.localDestination = tempFileName;
        m_info.destinationQTempDir = tmpDir;
    }

    return result;
}

void KomparePart::cleanUpTemporaryFiles()
{
    qCDebug(KOMPAREPART) << "Cleaning temporary files.";
    if (!m_info.localSource.isEmpty())
    {
        if (m_info.sourceQTempDir)
        {
            delete m_info.sourceQTempDir;
            m_info.sourceQTempDir = nullptr;
        }
        m_info.localSource.clear();
    }
    if (!m_info.localDestination.isEmpty())
    {
        if (!m_info.destinationQTempDir)
        {
            delete m_info.destinationQTempDir;
            m_info.destinationQTempDir = nullptr;
        }
        m_info.localDestination.clear();
    }
}

void KomparePart::compare(const QUrl& source, const QUrl& destination)
{
    // FIXME: This is silly, i can use NetAccess::stat to figure out what it is and not
    // wait until i am in the modellist to determine the mode we're supposed to be in.
    // That should make the code more readable
    // I should store the QTemporaryDir(s)/File(s) in the Info struct as well and delete it at the right time
    m_info.source = source;
    m_info.destination = destination;

    // FIXME: (Not urgent) But turn this into an enum, for now i cant find a nice name for the enum that has Source and Destination as values
    // For now we do not do error checking, user has already been notified and if the localString is empty then we do not diff
    fetchURL(source, true);
    fetchURL(destination, false);

    emit kompareInfo(&m_info);

    compareAndUpdateAll();
}

void KomparePart::compareFileString(const QUrl& sourceFile, const QString& destination)
{
    //Set the modeto specify that the source is a file, and the destination is a string
    m_info.mode = Kompare::ComparingFileString;

    m_info.source = sourceFile;
    m_info.localDestination = destination;

    fetchURL(sourceFile, true);

    emit kompareInfo(&m_info);

    compareAndUpdateAll();
}

void KomparePart::compareStringFile(const QString& source, const QUrl& destinationFile)
{
    //Set the modeto specify that the source is a file, and the destination is a string
    m_info.mode = Kompare::ComparingStringFile;

    m_info.localSource = source;
    m_info.destination = destinationFile;

    fetchURL(destinationFile, false);

    emit kompareInfo(&m_info);

    compareAndUpdateAll();
}

void KomparePart::compareFiles(const QUrl& sourceFile, const QUrl& destinationFile)
{
    m_info.mode = Kompare::ComparingFiles;

    m_info.source = sourceFile;
    m_info.destination = destinationFile;

    // FIXME: (Not urgent) But turn this into an enum, for now i cant find a nice name for the enum that has Source and Destination as values
    // For now we do not do error checking, user has already been notified and if the localString is empty then we do not diff
    fetchURL(sourceFile, true);
    fetchURL(destinationFile, false);

    emit kompareInfo(&m_info);

    compareAndUpdateAll();
}

void KomparePart::compareDirs(const QUrl& sourceDirectory, const QUrl& destinationDirectory)
{
    m_info.mode = Kompare::ComparingDirs;

    m_info.source = sourceDirectory;
    m_info.destination = destinationDirectory;

    fetchURL(sourceDirectory, true);
    fetchURL(destinationDirectory, false);

    emit kompareInfo(&m_info);

    compareAndUpdateAll();
}

void KomparePart::compare3Files(const QUrl& /*originalFile*/, const QUrl& /*changedFile1*/, const QUrl& /*changedFile2*/)
{
    // FIXME: actually implement this some day :)
    updateActions();
    updateCaption();
    updateStatus();
}

void KomparePart::openFileAndDiff(const QUrl& file, const QUrl& diffFile)
{
    m_info.source = file;
    m_info.destination = diffFile;

    fetchURL(file, true);
    fetchURL(diffFile, false);
    m_info.mode = Kompare::BlendingFile;

    emit kompareInfo(&m_info);

    compareAndUpdateAll();
}

void KomparePart::openDirAndDiff(const QUrl& dir,  const QUrl& diffFile)
{
    m_info.source = dir;
    m_info.destination = diffFile;

    fetchURL(dir, true);
    fetchURL(diffFile, false);
    m_info.mode = Kompare::BlendingDir;

    emit kompareInfo(&m_info);

    if (!m_info.localSource.isEmpty() && !m_info.localDestination.isEmpty())
    {
        m_modelList->openDirAndDiff();
        //Must this be in here? couldn't we use compareAndUpdateAll as well?
        updateActions();
        updateCaption();
        updateStatus();
    }
}

bool KomparePart::openFile()
{
    // This is called from openURL
    // This is a little inefficient but i will do it anyway
    openDiff(url());
    return true;
}

bool KomparePart::saveAll()
{
    bool result = m_modelList->saveAll();
    updateActions();
    updateCaption();
    updateStatus();
    return result;
}

void KomparePart::saveDiff()
{
    QDialog dlg(widget());
    dlg.setObjectName(QStringLiteral("save_options"));
    dlg.setModal(true);
    dlg.setWindowTitle(i18nc("@title:window", "Diff Options"));
    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dlg);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    KompareSaveOptionsWidget* w = new KompareSaveOptionsWidget(
        m_info.localSource,
        m_info.localDestination,
        m_diffSettings, &dlg);
    QVBoxLayout* layout = new QVBoxLayout(&dlg);
    layout->addWidget(w);
    layout->addWidget(buttons);
    dlg.setLayout(layout);

    if (dlg.exec()) {
        w->saveOptions();
        KSharedConfig::Ptr config = KSharedConfig::openConfig();
        saveProperties(config.data());
        config->sync();

        while (1)
        {
            QUrl url = QFileDialog::getSaveFileUrl(widget(), i18nc("@title:window", "Save .diff"),
                                                   m_info.destination,
                                                   i18n("Patch Files (*.diff *.dif *.patch)"));
            if (url.isLocalFile() && QFile::exists(url.toLocalFile())) {
                int result = KMessageBox::warningYesNoCancel(widget(), i18n("The file exists or is write-protected; do you want to overwrite it?"), i18nc("@window:title", "File Exists"), KStandardGuiItem::overwrite(), KGuiItem(i18nc("@action:button", "Do Not Overwrite")));
                if (result == KMessageBox::Cancel)
                {
                    break;
                }
                else if (result == KMessageBox::No)
                {
                    continue;
                }
                else
                {
                    qCDebug(KOMPAREPART) << "URL = " << url.toDisplayString();
                    qCDebug(KOMPAREPART) << "Directory = " << w->directory();
                    qCDebug(KOMPAREPART) << "DiffSettings = " << m_diffSettings;

                    m_modelList->saveDiff(url.url(), w->directory(), m_diffSettings);
                    break;
                }
            }
            else
            {
                qCDebug(KOMPAREPART) << "URL = " << url.toDisplayString();
                qCDebug(KOMPAREPART) << "Directory = " << w->directory();
                qCDebug(KOMPAREPART) << "DiffSettings = " << m_diffSettings;

                m_modelList->saveDiff(url.url(), w->directory(), m_diffSettings);
                break;
            }
        }
    }
}

void KomparePart::slotFilePrint()
{
    QPrinter printer;
    printer.setOrientation(QPrinter::Landscape);
    QPrintDialog* dlg = new QPrintDialog(&printer, nullptr);

    if (dlg->exec() == QDialog::Accepted)
    {
        // do some printing in qprinter
        slotPaintRequested(&printer);
    }

    delete dlg;
}

void KomparePart::slotFilePrintPreview()
{
    QPrinter printer;
    printer.setOrientation(QPrinter::Landscape);
    QPrintPreviewDialog dlg(&printer);

    connect(&dlg, &QPrintPreviewDialog::paintRequested, this, &KomparePart::slotPaintRequested);

    dlg.exec();
}

void KomparePart::slotPaintRequested(QPrinter* printer)
{
    qCDebug(KOMPAREPART) << "Now paint something...";
    QPainter p;
    p.begin(printer);

    QSize widgetWidth = m_view->size();
    qCDebug(KOMPAREPART) << "printer.width()     = " << printer->width();
    qCDebug(KOMPAREPART) << "widgetWidth.width() = " << widgetWidth.width();
    qreal factor = ((qreal)printer->width()) / ((qreal)widgetWidth.width());

    qCDebug(KOMPAREPART) << "factor              = " << factor;

    p.scale(factor, factor);
    m_view->render(&p);

    p.end();
    qCDebug(KOMPAREPART) << "Done painting something...";
}

void KomparePart::slotSetStatus(enum Kompare::Status status)
{
    updateActions();

    switch (status) {
    case Kompare::RunningDiff:
        emit setStatusBarText(i18nc("@info:status", "Running diff..."));
        break;
    case Kompare::Parsing:
        emit setStatusBarText(i18nc("@info:status", "Parsing diff output..."));
        break;
    case Kompare::FinishedParsing:
        updateStatus();
        break;
    case Kompare::FinishedWritingDiff:
        updateStatus();
        emit diffURLChanged();
        break;
    default:
        break;
    }
}

void KomparePart::onContextMenuRequested(const QPoint& pos)
{
    QMenu* popup = static_cast<QMenu*>(factory()->container(QStringLiteral("mainPopUp"), this));
    if (popup)
        popup->exec(m_view->mapToGlobal(pos));
}

void KomparePart::updateCaption()
{
    QString source = m_info.source.toDisplayString();
    QString destination = m_info.destination.toDisplayString();

    QString text;

    switch (m_info.mode)
    {
    case Kompare::ComparingFiles :
    case Kompare::ComparingDirs :
    case Kompare::BlendingFile :
    case Kompare::BlendingDir :
        text = source + QLatin1String(" -- ") + destination; // no need to translate this " -- "
        break;
    case Kompare::ShowingDiff :
        text = source;
        break;
    default:
        break;
    }

    emit setWindowCaption(text);
}

void KomparePart::updateStatus()
{
    QString source = m_info.source.toDisplayString();
    QString destination = m_info.destination.toDisplayString();

    QString text;

    switch (m_info.mode)
    {
    case Kompare::ComparingFiles :
        text = i18nc("@info:status", "Comparing file %1 with file %2" ,
                    source,
                    destination);
        break;
    case Kompare::ComparingDirs :
        text = i18nc("@info:status", "Comparing files in %1 with files in %2" ,
                    source,
                    destination);
        break;
    case Kompare::ShowingDiff :
        text = i18nc("@info:status", "Viewing diff output from %1", source);
        break;
    case Kompare::BlendingFile :
        text = i18nc("@info:status", "Blending diff output from %1 into file %2" ,
                    source,
                    destination);
        break;
    case Kompare::BlendingDir :
        text = i18nc("@info:status", "Blending diff output from %1 into folder %2" ,
                    m_info.source.toDisplayString(),
                    m_info.destination.toDisplayString());
        break;
    default:
        break;
    }

    emit setStatusBarText(text);
}

void KomparePart::compareAndUpdateAll()
{
    if (!m_info.localSource.isEmpty() && !m_info.localDestination.isEmpty())
    {
        switch (m_info.mode)
        {
        default:
        case Kompare::UnknownMode:
            m_modelList->compare();
            break;

        case Kompare::ComparingStringFile:
        case Kompare::ComparingFileString:
        case Kompare::ComparingFiles:
        case Kompare::ComparingDirs:
            m_modelList->compare(m_info.mode);
            break;

        case Kompare::BlendingFile:
            m_modelList->openFileAndDiff();
            break;
        }
        updateCaption();
        updateStatus();
    }
    updateActions();
}

void KomparePart::slotShowError(const QString& error)
{
    KMessageBox::error(widget(), error);
}

void KomparePart::slotSwap()
{
    if (m_modelList->hasUnsavedChanges())
    {
        int query = KMessageBox::warningYesNoCancel
                    (
                        widget(),
                        i18n("You have made changes to the destination file(s).\n"
                             "Would you like to save them?"),
                        i18nc("@title:window", "Save Changes?"),
                        KStandardGuiItem::save(),
                        KStandardGuiItem::discard()
                    );

        if (query == KMessageBox::Yes)
            m_modelList->saveAll();

        if (query == KMessageBox::Cancel)
            return; // Abort prematurely so no swapping
    }

    // Swap the info in the Kompare::Info struct
    m_info.swapSourceWithDestination();

    // Update window caption and statusbar text
    updateCaption();
    updateStatus();

    m_modelList->swap();
}

void KomparePart::slotRefreshDiff()
{
    if (m_modelList->hasUnsavedChanges())
    {
        int query = KMessageBox::warningYesNoCancel
                    (
                        widget(),
                        i18n("You have made changes to the destination file(s).\n"
                             "Would you like to save them?"),
                        i18nc("@title:window", "Save Changes?"),
                        KStandardGuiItem::save(),
                        KStandardGuiItem::discard()
                    );

        if (query == KMessageBox::Cancel)
            return; // Abort prematurely so no refreshing

        if (query == KMessageBox::Yes)
            m_modelList->saveAll();
    }

    // For this to work properly you have to refetch the files from their (remote) locations
    cleanUpTemporaryFiles();
    fetchURL(m_info.source, true);
    fetchURL(m_info.destination, false);
    m_modelList->refresh();
}

void KomparePart::slotShowDiffstats()
{
    // Fetch all the args needed for komparestatsmessagebox
    // oldfile, newfile, diffformat, noofhunks, noofdiffs

    QString oldFile;
    QString newFile;
    QString diffFormat;
    int filesInDiff;
    int noOfHunks;
    int noOfDiffs;

    oldFile = m_modelList->selectedModel() ? m_modelList->selectedModel()->sourceFile()  : QString();
    newFile = m_modelList->selectedModel() ? m_modelList->selectedModel()->destinationFile() : QString();

    if (m_modelList->selectedModel())
    {
        switch (m_info.format) {
        case Kompare::Unified :
            diffFormat = i18nc("@item diff format", "Unified");
            break;
        case Kompare::Context :
            diffFormat = i18nc("@item diff format", "Context");
            break;
        case Kompare::RCS :
            diffFormat = i18nc("@item diff format", "RCS");
            break;
        case Kompare::Ed :
            diffFormat = i18nc("@item diff format", "Ed");
            break;
        case Kompare::Normal :
            diffFormat = i18nc("@item diff format", "Normal");
            break;
        case Kompare::UnknownFormat :
        default:
            diffFormat = i18nc("@item diff format", "Unknown");
            break;
        }
    }
    else
    {
        diffFormat.clear();
    }

    filesInDiff = m_modelList->modelCount();

    noOfHunks = m_modelList->selectedModel() ? m_modelList->selectedModel()->hunkCount() : 0;
    noOfDiffs = m_modelList->selectedModel() ? m_modelList->selectedModel()->differenceCount() : 0;

    if (m_modelList->modelCount() == 0) {   // no diff loaded yet
        KMessageBox::information(nullptr, i18n(
            "No diff file, or no 2 files have been diffed. "
            "Therefore no stats are available."),
            i18nc("@title:window", "Diff Statistics"), QString(), nullptr);
    }
    else if (m_modelList->modelCount() == 1) {   // 1 file in diff, or 2 files compared
        KMessageBox::information(nullptr, i18n(
            "Statistics:\n"
            "\n"
            "Old file: %1\n"
            "New file: %2\n"
            "\n"
            "Format: %3\n"
            "Number of hunks: %4\n"
            "Number of differences: %5",
            oldFile, newFile, diffFormat,
            noOfHunks, noOfDiffs),
            i18nc("@title:window", "Diff Statistics"), QString(), nullptr);
    } else { // more than 1 file in diff, or 2 directories compared
        KMessageBox::information(nullptr, ki18n(
            "Statistics:\n"
            "\n"
            "Number of files in diff file: %1\n"
            "Format: %2\n"
            "\n"
            "Current old file: %3\n"
            "Current new file: %4\n"
            "\n"
            "Number of hunks: %5\n"
            "Number of differences: %6")
            .subs(filesInDiff).subs(diffFormat).subs(oldFile)
            .subs(newFile).subs(noOfHunks).subs(noOfDiffs)
            .toString(),
            i18nc("@title:window", "Diff Statistics"), QString(), nullptr);
    }
}

bool KomparePart::queryClose()
{
    if (!m_modelList->hasUnsavedChanges()) return true;

    int query = KMessageBox::warningYesNoCancel
                (
                    widget(),
                    i18n("You have made changes to the destination file(s).\n"
                         "Would you like to save them?"),
                    i18nc("@title:window", "Save Changes?"),
                    KStandardGuiItem::save(),
                    KStandardGuiItem::discard()
                );

    if (query == KMessageBox::Cancel)
        return false;

    if (query == KMessageBox::Yes)
        return m_modelList->saveAll();

    return true;
}

void KomparePart::setReadWrite(bool readWrite)
{
    m_modelList->setReadWrite(readWrite);
    KParts::ReadWritePart::setReadWrite(readWrite);
}

int KomparePart::readProperties(KConfig* config)
{
    m_viewSettings->loadSettings(config);
    m_diffSettings->loadSettings(config);
    emit configChanged();
    return 0;
}

int KomparePart::saveProperties(KConfig* config)
{
    m_viewSettings->saveSettings(config);
    m_diffSettings->saveSettings(config);
    return 0;
}

void KomparePart::optionsPreferences()
{
    // show preferences
    KomparePrefDlg pref(m_viewSettings, m_diffSettings);

    connect(&pref, &KomparePrefDlg::configChanged, this, &KomparePart::configChanged);

    if (pref.exec())
        emit configChanged();
}
