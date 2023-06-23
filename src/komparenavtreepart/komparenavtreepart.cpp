/*
    SPDX-FileCopyrightText: 2001-2005, 2009 Otto Bruggeman <bruggie@gmail.com>
    SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2007-2011 Kevin Kofler <kevin.kofler@chello.at>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "komparenavtreepart.h"

#include <QDebug>
#include <QTreeWidgetItemIterator>

#include <KLocalizedString>
#include <KPluginMetaData>
#include <KPluginFactory>

#include <KompareDiff2/Difference>
#include <KompareDiff2/DiffModel>
#include <KompareDiff2/DiffModelList>
#include <KompareDiff2/KompareModelList>

#include <komparenavviewdebug.h>

#define COL_SOURCE        0
#define COL_DESTINATION   1
#define COL_DIFFERENCE    2

using namespace Diff2;

KompareNavTreePart::KompareNavTreePart(QWidget* parentWidget, QObject* parent,
                                       const KPluginMetaData& metaData, const QVariantList&)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    : KParts::ReadOnlyPart(parent, metaData),
#else
    : KParts::ReadOnlyPart(parent),
#endif
      m_splitter(nullptr),
      m_modelList(nullptr),
      m_srcDirTree(nullptr),
      m_destDirTree(nullptr),
      m_fileList(nullptr),
      m_changesList(nullptr),
      m_srcRootItem(nullptr),
      m_destRootItem(nullptr),
      m_selectedModel(nullptr),
      m_selectedDifference(nullptr),
      m_source(),
      m_destination(),
      m_info(nullptr)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    setMetaData(metaData);
#endif

    m_splitter = new QSplitter(Qt::Horizontal, parentWidget);

    setWidget(m_splitter);

    m_srcDirTree = new QTreeWidget(m_splitter);
    m_srcDirTree->setHeaderLabel(i18nc("@title:column", "Source Folder"));
    m_srcDirTree->setRootIsDecorated(false);
    m_srcDirTree->setSortingEnabled(true);
    m_srcDirTree->sortByColumn(0, Qt::AscendingOrder);

    m_destDirTree = new QTreeWidget(m_splitter);
    m_destDirTree->setHeaderLabel(i18nc("@title:column", "Destination Folder"));
    m_destDirTree->setRootIsDecorated(false);
    m_destDirTree->setSortingEnabled(true);
    m_destDirTree->sortByColumn(0, Qt::AscendingOrder);

    m_fileList = new QTreeWidget(m_splitter);
    m_fileList->setHeaderLabels(QStringList {
        i18nc("@title:column", "Source File"),
        i18nc("@title:column", "Destination File")
    } );
    m_fileList->setAllColumnsShowFocus(true);
    m_fileList->setRootIsDecorated(false);
    m_fileList->setSortingEnabled(true);
    m_fileList->sortByColumn(0, Qt::AscendingOrder);

    m_changesList = new QTreeWidget(m_splitter);
    m_changesList->setHeaderLabels(QStringList {
        i18nc("@title:column", "Source Line"),
        i18nc("@title:column", "Destination Line"),
        i18nc("@title:column", "Difference"),
    } );
    m_changesList->setAllColumnsShowFocus(true);
    m_changesList->setRootIsDecorated(false);
    m_changesList->setSortingEnabled(true);
    m_changesList->sortByColumn(0, Qt::AscendingOrder);

    connect(m_srcDirTree, &QTreeWidget::currentItemChanged,
            this, &KompareNavTreePart::slotSrcDirTreeSelectionChanged);
    connect(m_destDirTree, &QTreeWidget::currentItemChanged,
            this, &KompareNavTreePart::slotDestDirTreeSelectionChanged);
    connect(m_fileList, &QTreeWidget::currentItemChanged,
            this, &KompareNavTreePart::slotFileListSelectionChanged);
    connect(m_changesList, &QTreeWidget::currentItemChanged,
            this, &KompareNavTreePart::slotChangesListSelectionChanged);
}

KompareNavTreePart::~KompareNavTreePart()
{
    m_modelList = nullptr;
    m_selectedModel = nullptr;
    m_selectedDifference = nullptr;
}

void KompareNavTreePart::slotKompareInfo(struct Kompare::Info* info)
{
    m_info = info;
}

void KompareNavTreePart::slotModelsChanged(const DiffModelList* modelList)
{
    qCDebug(KOMPARENAVVIEW) << "Models (" << modelList << ") have changed... scanning the models... " ;

    if (modelList)
    {
        m_modelList = modelList;
        m_srcDirTree->clear();
        m_destDirTree->clear();
        m_fileList->clear();
        m_changesList->clear();
        buildTreeInMemory();
    }
    else
    {
        m_modelList = modelList;
        m_srcDirTree->clear();
        m_destDirTree->clear();
        m_fileList->clear();
        m_changesList->clear();
    }
}

void KompareNavTreePart::buildTreeInMemory()
{
    qCDebug(KOMPARENAVVIEW) << "BuildTreeInMemory called" ;

    if (m_modelList->count() == 0)
    {
        qCDebug(KOMPARENAVVIEW) << "No models... weird shit..." ;
        return; // avoids a crash on clear()
    }

    if (!m_info)
    {
        qCDebug(KOMPARENAVVIEW) << "No Info... weird shit..." ;
        return;
    }

    QString srcBase;
    QString destBase;

    DiffModel* model;
    model = m_modelList->first();
    m_selectedModel = nullptr;

    switch (m_info->mode)
    {
    case Kompare::ShowingDiff:
        // BUG: 107489 No common root because it is a multi directory relative path diff
        // We need to detect this and create a different rootitem / or so or should we always add this?
        // Trouble we run into is that the directories do not start with a /
        // so we have an unknown top root dir
        // Thinking some more about it i guess it is best to use "" as base and simply show some string
        // like Unknown filesystem path as root text but only in the case of dirs starting without a /
        srcBase = model->sourcePath();
        destBase = model->destinationPath();
        // FIXME: these tests will not work on windows, we need something else
        if (srcBase[0] != QLatin1Char('/'))
            srcBase.clear();
        if (destBase[0] != QLatin1Char('/'))
            destBase.clear();
        break;
    case Kompare::ComparingFiles:
        srcBase  = model->sourcePath();
        destBase = model->destinationPath();
        break;
    case Kompare::ComparingDirs:
        srcBase = m_info->localSource;
        if (!srcBase.endsWith(QLatin1Char('/')))
            srcBase += QLatin1Char('/');
        destBase = m_info->localDestination;
        if (!destBase.endsWith(QLatin1Char('/')))
            destBase += QLatin1Char('/');
        break;
    case Kompare::BlendingFile:
    case Kompare::BlendingDir:
    default:
        qCDebug(KOMPARENAVVIEW) << "Oops needs to implement this..." ;
    }

//     qCDebug(KOMPARENAVVIEW) << "srcBase  = " << srcBase ;
//     qCDebug(KOMPARENAVVIEW) << "destBase = " << destBase ;

    m_srcRootItem  = new KDirLVI(m_srcDirTree, srcBase);
    m_destRootItem = new KDirLVI(m_destDirTree, destBase);

    QString srcPath;
    QString destPath;

    // Create the tree from the models
    DiffModelListConstIterator modelIt = m_modelList->begin();
    DiffModelListConstIterator mEnd    = m_modelList->end();

    for (; modelIt != mEnd; ++modelIt)
    {
        model = *modelIt;
        srcPath  = model->sourcePath();
        destPath = model->destinationPath();

        qCDebug(KOMPARENAVVIEW) << "srcPath  = " << srcPath  ;
        qCDebug(KOMPARENAVVIEW) << "destPath = " << destPath ;
        m_srcRootItem->addModel(srcPath, model, &m_modelToSrcDirItemDict);
        m_destRootItem->addModel(destPath, model, &m_modelToDestDirItemDict);
    }
//     m_srcDirTree->setSelected( m_srcDirTree->firstChild(), true );
}

void KompareNavTreePart::buildDirectoryTree()
{
// FIXME: afaict this can be deleted
//     qCDebug(KOMPARENAVVIEW) << "BuildDirTree called" ;
}

QString KompareNavTreePart::compareFromEndAndReturnSame(
    const QString& string1,
    const QString& string2)
{
    QString result;

    int srcLen = string1.length();
    int destLen = string2.length();

    while (srcLen != 0 && destLen != 0)
    {
        if (string1[--srcLen] == string2[--destLen])
            result.prepend(string1[srcLen]);
        else
            break;
    }

    if (srcLen != 0 && destLen != 0 && result.startsWith(QLatin1Char('/')))
        result = result.remove(0, 1);   // strip leading /, we need it later

    return result;
}

void KompareNavTreePart::slotSetSelection(const DiffModel* model, const Difference* diff)
{
    qCDebug(KOMPARENAVVIEW) << "KompareNavTreePart::slotSetSelection model = " << model << ", diff = " << diff ;
    if (model == m_selectedModel)
    {
        // model is the same, so no need to update that...
        if (diff != m_selectedDifference)
        {
            m_selectedDifference = diff;
            setSelectedDifference(diff);
        }
        return;
    }

    // model is different so we need to find the right dirs, file and changeitems to select
    // if m_selectedModel == NULL then everything needs to be done as well
    if (!m_selectedModel || model->sourcePath() != m_selectedModel->sourcePath())
    {   // dirs are different, so we need to update the dirviews as well
        m_selectedModel = model;
        m_selectedDifference = diff;

        setSelectedDir(model);
        setSelectedFile(model);
        setSelectedDifference(diff);
        return;
    }

    if (!m_selectedModel || model->sourceFile() != m_selectedModel->sourceFile())
    {
        m_selectedModel = model;
        setSelectedFile(model);

        m_selectedDifference = diff;
        setSelectedDifference(diff);
    }
}

void KompareNavTreePart::setSelectedDir(const DiffModel* model)
{
    KDirLVI* currentDir;
    currentDir = m_modelToSrcDirItemDict[ model ];
    qCDebug(KOMPARENAVVIEW) << "Manually setting selection in srcdirtree with currentDir = " << currentDir ;
    m_srcDirTree->blockSignals(true);
    m_srcDirTree->setCurrentItem(currentDir);
    m_srcDirTree->scrollToItem(currentDir);
    m_srcDirTree->blockSignals(false);

    currentDir = m_modelToDestDirItemDict[ model ];
    qCDebug(KOMPARENAVVIEW) << "Manually setting selection in destdirtree with currentDir = " << currentDir ;
    m_destDirTree->blockSignals(true);
    m_destDirTree->setCurrentItem(currentDir);
    m_destDirTree->scrollToItem(currentDir);
    m_destDirTree->blockSignals(false);

    m_fileList->blockSignals(true);
    currentDir->fillFileList(m_fileList, &m_modelToFileItemDict);
    m_fileList->blockSignals(false);
}

void KompareNavTreePart::setSelectedFile(const DiffModel* model)
{
    KFileLVI* currentFile;
    currentFile = m_modelToFileItemDict[ model ];
    qCDebug(KOMPARENAVVIEW) << "Manually setting selection in filelist" ;
    m_fileList->blockSignals(true);
    m_fileList->setCurrentItem(currentFile);
    m_fileList->scrollToItem(currentFile);
    m_fileList->blockSignals(false);

    m_changesList->blockSignals(true);
    currentFile->fillChangesList(m_changesList, &m_diffToChangeItemDict);
    m_changesList->blockSignals(false);
}

void KompareNavTreePart::setSelectedDifference(const Difference* diff)
{
    KChangeLVI* currentDiff;
    currentDiff = m_diffToChangeItemDict[ diff ];
    qCDebug(KOMPARENAVVIEW) << "Manually setting selection in changeslist to " << currentDiff ;
    m_changesList->blockSignals(true);
    m_changesList->setCurrentItem(currentDiff);
    m_changesList->scrollToItem(currentDiff);
    m_changesList->blockSignals(false);
}

void KompareNavTreePart::slotSetSelection(const Difference* diff)
{
//     qCDebug(KOMPARENAVVIEW) << "Scotty i need more power !!" ;
    if (m_selectedDifference != diff)
    {
//         qCDebug(KOMPARENAVVIEW) << "But sir, i am giving you all she's got" ;
        m_selectedDifference = diff;
        setSelectedDifference(diff);
    }
}

void KompareNavTreePart::slotSrcDirTreeSelectionChanged(QTreeWidgetItem* item)
{
    if (!item)
        return;

    qCDebug(KOMPARENAVVIEW) << "Sent by the sourceDirectoryTree with item = " << item ;
    m_srcDirTree->scrollToItem(item);
    KDirLVI* dir = static_cast<KDirLVI*>(item);
    // order the dest tree view to set its selected item to the same as here
    QString path;
    // We start with an empty path and after the call path contains the full path
    path = dir->fullPath(path);
    KDirLVI* selItem = m_destRootItem->setSelected(path);
    m_destDirTree->blockSignals(true);
    m_destDirTree->setCurrentItem(selItem);
    m_destDirTree->scrollToItem(selItem);
    m_destDirTree->blockSignals(false);
    // fill the changes list
    dir->fillFileList(m_fileList, &m_modelToFileItemDict);
}

void KompareNavTreePart::slotDestDirTreeSelectionChanged(QTreeWidgetItem* item)
{
    if (!item)
        return;

    qCDebug(KOMPARENAVVIEW) << "Sent by the destinationDirectoryTree with item = " << item ;
    m_destDirTree->scrollToItem(item);
    KDirLVI* dir = static_cast<KDirLVI*>(item);
    // order the src tree view to set its selected item to the same as here
    QString path;
    // We start with an empty path and after the call path contains the full path
    path = dir->fullPath(path);
    KDirLVI* selItem = m_srcRootItem->setSelected(path);
    m_srcDirTree->blockSignals(true);
    m_srcDirTree->setCurrentItem(selItem);
    m_srcDirTree->scrollToItem(selItem);
    m_srcDirTree->blockSignals(false);
    // fill the changes list
    dir->fillFileList(m_fileList, &m_modelToFileItemDict);
}

void KompareNavTreePart::slotFileListSelectionChanged(QTreeWidgetItem* item)
{
    if (!item)
        return;

    qCDebug(KOMPARENAVVIEW) << "Sent by the fileList with item = " << item ;

    KFileLVI* file = static_cast<KFileLVI*>(item);
    m_selectedModel = file->model();
    m_changesList->blockSignals(true);
    file->fillChangesList(m_changesList, &m_diffToChangeItemDict);
    m_changesList->blockSignals(false);

    if (m_changesList->currentItem())
    {
        // FIXME: This is ugly...
        m_selectedDifference = (static_cast<KChangeLVI*>(m_changesList->currentItem()))->difference();
    }

    Q_EMIT selectionChanged(m_selectedModel, m_selectedDifference);
}

void KompareNavTreePart::slotChangesListSelectionChanged(QTreeWidgetItem* item)
{
    if (!item)
        return;

    qCDebug(KOMPARENAVVIEW) << "Sent by the changesList" ;

    KChangeLVI* change = static_cast<KChangeLVI*>(item);
    m_selectedDifference = change->difference();

    Q_EMIT selectionChanged(m_selectedDifference);
}

void KompareNavTreePart::slotApplyDifference(bool /*apply*/)
{
    KChangeLVI* clvi = m_diffToChangeItemDict[m_selectedDifference];
    if (clvi)
        clvi->setDifferenceText();
}

void KompareNavTreePart::slotApplyAllDifferences(bool /*apply*/)
{
    QHash<const Diff2::Difference*, KChangeLVI*>::ConstIterator it = m_diffToChangeItemDict.constBegin();
    QHash<const Diff2::Difference*, KChangeLVI*>::ConstIterator end = m_diffToChangeItemDict.constEnd();

    qCDebug(KOMPARENAVVIEW) << "m_diffToChangeItemDict.count() = " << m_diffToChangeItemDict.count() ;

    for (; it != end ; ++it)
    {
        it.value()->setDifferenceText();
    }
}

void KompareNavTreePart::slotApplyDifference(const Difference* diff, bool /*apply*/)
{
    // this applies to the currently selected difference
    KChangeLVI* clvi = m_diffToChangeItemDict[diff];
    if (clvi)
        clvi->setDifferenceText();
}

void KChangeLVI::setDifferenceText()
{
    QString text;
    switch (m_difference->type()) {
    case Difference::Change:
        // Shouldn't this simply be diff->sourceLineCount() ?
        // because you change the _number of lines_ lines in source, not in dest
        if (m_difference->applied())
            text = i18np("Applied: Changes made to %1 line undone", "Applied: Changes made to %1 lines undone",
                         m_difference->sourceLineCount());
        else
            text = i18np("Changed %1 line", "Changed %1 lines",
                         m_difference->sourceLineCount());
        break;
    case Difference::Insert:
        if (m_difference->applied())
            text = i18np("Applied: Insertion of %1 line undone", "Applied: Insertion of %1 lines undone",
                         m_difference->destinationLineCount());
        else
            text = i18np("Inserted %1 line", "Inserted %1 lines",
                         m_difference->destinationLineCount());
        break;
    case Difference::Delete:
        if (m_difference->applied())
            text = i18np("Applied: Deletion of %1 line undone", "Applied: Deletion of %1 lines undone",
                         m_difference->sourceLineCount());
        else
            text = i18np("Deleted %1 line", "Deleted %1 lines",
                         m_difference->sourceLineCount());
        break;
    default:
        qCDebug(KOMPARENAVVIEW) << "Unknown or Unchanged enum value when checking for diff->type() in KChangeLVI's constructor" ;
        text.clear();
    }

    setText(2, text);
}

KChangeLVI::KChangeLVI(QTreeWidget* parent, Difference* diff) : QTreeWidgetItem(parent)
{
    m_difference = diff;

    setText(0, QString::number(diff->sourceLineNumber()));
    setText(1, QString::number(diff->destinationLineNumber()));

    setDifferenceText();
}

bool KChangeLVI::operator<(const QTreeWidgetItem& item) const
{
    int column = treeWidget()->sortColumn();
    QString text1 = text(column);
    QString text2 = item.text(column);

    // Compare the numbers.
    if (column < 2 && text1.length() != text2.length())
        return text1.length() < text2.length();
    return text1 < text2;
}

KChangeLVI::~KChangeLVI()
{
}

KFileLVI::KFileLVI(QTreeWidget* parent, DiffModel* model) : QTreeWidgetItem(parent)
{
    m_model = model;

    QString src = model->sourceFile();
    QString dst = model->destinationFile();

    setText(0, src);
    setText(1, dst);
    setIcon(0, QIcon::fromTheme(getIcon(src)));
    setIcon(1, QIcon::fromTheme(getIcon(dst)));
}

bool KFileLVI::hasExtension(const QString& extensions, const QString& fileName)
{
    const QStringList extList = extensions.split(QLatin1Char(' '));
    for (const QString& ext : extList) {
        if (fileName.endsWith(ext, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

const QString KFileLVI::getIcon(const QString& fileName)
{
    // C++, C
    if (hasExtension(QStringLiteral(".h .hpp"), fileName)) {
        return QStringLiteral("text-x-c++hdr");
    }
    if (hasExtension(QStringLiteral(".cpp"), fileName)) {
        return QStringLiteral("text-x-c++src");
    }
    if (hasExtension(QStringLiteral(".c"), fileName)) {
        return QStringLiteral("text-x-csrc");
    }
    // Python
    if (hasExtension(QStringLiteral(".py .pyw"), fileName)) {
        return QStringLiteral("text-x-python");
    }
    // C#
    if (hasExtension(QStringLiteral(".cs"), fileName)) {
        return QStringLiteral("text-x-csharp");
    }
    // Objective-C
    if (hasExtension(QStringLiteral(".m"), fileName)) {
        return QStringLiteral("text-x-objcsrc");
    }
    // Java
    if (hasExtension(QStringLiteral(".java"), fileName)) {
        return QStringLiteral("text-x-java");
    }
    // Script
    if (hasExtension(QStringLiteral(".sh"), fileName)) {
        return QStringLiteral("text-x-script");
    }
    // Makefile
    if (hasExtension(QStringLiteral(".cmake Makefile"), fileName)) {
        return QStringLiteral("text-x-makefile");
    }
    // Ada
    if (hasExtension(QStringLiteral(".ada .ads .adb"), fileName)) {
        return QStringLiteral("text-x-adasrc");
    }
    // Pascal
    if (hasExtension(QStringLiteral(".pas"), fileName)) {
        return QStringLiteral("text-x-pascal");
    }
    // Patch
    if (hasExtension(QStringLiteral(".diff"), fileName)) {
        return QStringLiteral("text-x-patch");
    }
    // Tcl
    if (hasExtension(QStringLiteral(".tcl"), fileName)) {
        return QStringLiteral("text-x-tcl");
    }
    // Text
    if (hasExtension(QStringLiteral(".txt"), fileName)) {
        return QStringLiteral("text-plain");
    }
    // Xml
    if (hasExtension(QStringLiteral(".xml"), fileName)) {
        return QStringLiteral("text-xml");
    }
    // unknown or no file extension
    return QStringLiteral("text-plain");
}

void KFileLVI::fillChangesList(QTreeWidget* changesList, QHash<const Diff2::Difference*, KChangeLVI*>* diffToChangeItemDict)
{
    changesList->clear();
    diffToChangeItemDict->clear();

    DifferenceListConstIterator diffIt = m_model->differences()->constBegin();
    DifferenceListConstIterator dEnd   = m_model->differences()->constEnd();

    for (; diffIt != dEnd; ++diffIt)
    {
        KChangeLVI* change = new KChangeLVI(changesList, *diffIt);
        diffToChangeItemDict->insert(*diffIt, change);
    }

    changesList->setCurrentItem(changesList->topLevelItem(0));
}

KFileLVI::~KFileLVI()
{
}

KDirLVI::KDirLVI(QTreeWidget* parent, const QString& dir) : QTreeWidgetItem(parent)
{
//     qCDebug(KOMPARENAVVIEW) << "KDirLVI (QTreeWidget) constructor called with dir = " << dir ;
    m_rootItem = true;
    m_dirName = dir;
    setIcon(0, QIcon::fromTheme(QStringLiteral("folder")));
    setExpanded(true);
    if (m_dirName.isEmpty())
        setText(0, i18nc("@item directory name not known", "Unknown"));
    else
        setText(0, m_dirName);
}

KDirLVI::KDirLVI(KDirLVI* parent, const QString& dir) : QTreeWidgetItem(parent)
{
//     qCDebug(KOMPARENAVVIEW) << "KDirLVI (KDirLVI) constructor called with dir = " << dir ;
    m_rootItem = false;
    m_dirName = dir;
    setIcon(0, QIcon::fromTheme(QStringLiteral("folder")));
    setExpanded(true);
    setText(0, m_dirName);
}

// addModel always removes it own path from the beginning
void KDirLVI::addModel(QString& path, DiffModel* model, QHash<const Diff2::DiffModel*, KDirLVI*>* modelToDirItemDict)
{
//     qCDebug(KOMPARENAVVIEW) << "KDirLVI::addModel called with path = " << path << " from KDirLVI with m_dirName = " << m_dirName ;

    if (!m_dirName.isEmpty())
    {
        if (path.indexOf(m_dirName) > -1)
            path = path.remove(path.indexOf(m_dirName), m_dirName.length());
    }

//     qCDebug(KOMPARENAVVIEW) << "Path after removal of own dir (\"" << m_dirName << "\") = " << path ;

    if (path.isEmpty()) {
        m_modelList.append(model);
        modelToDirItemDict->insert(model, this);
        return;
    }

    KDirLVI* child;

    QString dir = path.mid(0, path.indexOf(QLatin1Char('/'), 0) + 1);
    child = findChild(dir);
    if (!child)
    {
        // does not exist yet so make it
//         qCDebug(KOMPARENAVVIEW) << "KDirLVI::addModel creating new KDirLVI because not found" ;
        child = new KDirLVI(this, dir);
    }

    child->addModel(path, model, modelToDirItemDict);
}

KDirLVI* KDirLVI::findChild(const QString& dir)
{
//     qCDebug(KOMPARENAVVIEW) << "KDirLVI::findChild called with dir = " << dir ;
    KDirLVI* child;
    if ((child = static_cast<KDirLVI*>(this->child(0))) != nullptr)
    {   // has children, check if dir already exists, if so addModel
        QTreeWidgetItemIterator it(child);
        while (*it) {
            child = static_cast<KDirLVI*>(*it);

            if (dir == child->dirName())
                return child;
            ++it;
        }
    }

    return nullptr;
}

void KDirLVI::fillFileList(QTreeWidget* fileList, QHash<const Diff2::DiffModel*, KFileLVI*>* modelToFileItemDict)
{
    fileList->clear();

    DiffModelListIterator modelIt = m_modelList.begin();
    DiffModelListIterator mEnd    = m_modelList.end();
    for (; modelIt != mEnd; ++modelIt)
    {
        KFileLVI* file = new KFileLVI(fileList, *modelIt);
        modelToFileItemDict->insert(*modelIt, file);
    }

    fileList->setCurrentItem(fileList->topLevelItem(0));
}

QString KDirLVI::fullPath(QString& path)
{
//     if (!path.isEmpty())
//         qCDebug(KOMPARENAVVIEW) << "KDirLVI::fullPath called with path = " << path ;
//     else
//         qCDebug(KOMPARENAVVIEW) << "KDirLVI::fullPath called with empty path..." ;

    if (m_rootItem)   // don't bother adding rootItem's dir...
        return path;

    path = path.prepend(m_dirName);

    KDirLVI* lviParent = dynamic_cast<KDirLVI*>(parent());
    if (lviParent)
    {
        path = lviParent->fullPath(path);
    }

    return path;
}

KDirLVI* KDirLVI::setSelected(const QString& _dir)
{
    QString dir(_dir);
//     qCDebug(KOMPARENAVVIEW) << "KDirLVI::setSelected called with dir = " << dir ;

    // root item's dirName is never taken into account... remember that
    if (!m_rootItem)
    {
        dir = dir.remove(0, m_dirName.length());
    }

    if (dir.isEmpty())
    {
        return this;
    }
    KDirLVI* child = static_cast<KDirLVI*>(this->child(0));
    if (!child)
        return nullptr;

    QTreeWidgetItemIterator it(child);
    while (*it) {
        child = static_cast<KDirLVI*>(*it);

        if (dir.startsWith(child->dirName()))
            return child->setSelected(dir);
        ++it;
    }

    return nullptr;
}

KDirLVI::~KDirLVI()
{
    m_modelList.clear();
}

K_PLUGIN_FACTORY_WITH_JSON(KompareNavTreePartFactory, "komparenavtreepart.json",
                           registerPlugin<KompareNavTreePart>();)

#include "komparenavtreepart.moc"
#include "moc_komparenavtreepart.cpp"
