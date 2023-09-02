/*
    SPDX-FileCopyrightText: 2001-2004 Otto Bruggeman <otto.bruggeman@home.nl>
    SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2007-2011 Kevin Kofler <kevin.kofler@chello.at>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPARENAVTREEPART_H
#define KOMPARENAVTREEPART_H

// Qt
#include <QHash>
#include <QSplitter>
#include <QTreeWidget>
#include <QTreeWidgetItem>
// KF
#include <KParts/ReadOnlyPart>
// KompareDiff2
#include <KompareDiff2/Kompare>
#include <KompareDiff2/DiffModelList>

namespace KompareDiff2 {
class DiffModel;
class Difference;
}

class KDirLVI;
class KFileLVI;
class KChangeLVI;

class KompareNavTreePart : public KParts::ReadOnlyPart
{
    Q_OBJECT

public:
    explicit KompareNavTreePart(QWidget* parentWidget, QObject* parent,
                                const KPluginMetaData& metaData, const QVariantList& args);
    ~KompareNavTreePart() override;

public:
    bool openFile() override { return false; };

public Q_SLOTS:
    void slotSetSelection(const KompareDiff2::DiffModel* model, const KompareDiff2::Difference* diff);
    void slotSetSelection(const KompareDiff2::Difference* diff);
    void slotModelsChanged(const KompareDiff2::DiffModelList* modelList);
    void slotKompareInfo(Kompare::Info* info);

Q_SIGNALS:
    void selectionChanged(const KompareDiff2::DiffModel* model, const KompareDiff2::Difference* diff);
    void selectionChanged(const KompareDiff2::Difference* diff);

private Q_SLOTS:
    void slotSrcDirTreeSelectionChanged(QTreeWidgetItem* item);
    void slotDestDirTreeSelectionChanged(QTreeWidgetItem* item);
    void slotFileListSelectionChanged(QTreeWidgetItem* item);
    void slotChangesListSelectionChanged(QTreeWidgetItem* item);

    void slotApplyDifference(bool apply);
    void slotApplyAllDifferences(bool apply);
    void slotApplyDifference(const KompareDiff2::Difference* diff, bool apply);

    void buildTreeInMemory();

private:
    void setSelectedDir(const KompareDiff2::DiffModel* model);
    void setSelectedFile(const KompareDiff2::DiffModel* model);
    void setSelectedDifference(const KompareDiff2::Difference* diff);

    void buildDirectoryTree();

    QString compareFromEndAndReturnSame(const QString& string1, const QString& string2);

private:
    QSplitter*                         m_splitter;
    const KompareDiff2::DiffModelList*        m_modelList;

    QHash<const KompareDiff2::Difference*, KChangeLVI*>    m_diffToChangeItemDict;
    QHash<const KompareDiff2::DiffModel*, KFileLVI*>       m_modelToFileItemDict;
    QHash<const KompareDiff2::DiffModel*, KDirLVI*>        m_modelToSrcDirItemDict;
    QHash<const KompareDiff2::DiffModel*, KDirLVI*>        m_modelToDestDirItemDict;

    QTreeWidget*                       m_srcDirTree;
    QTreeWidget*                       m_destDirTree;
    QTreeWidget*                       m_fileList;
    QTreeWidget*                       m_changesList;

    KDirLVI*                           m_srcRootItem;
    KDirLVI*                           m_destRootItem;

    const KompareDiff2::DiffModel*            m_selectedModel;
    const KompareDiff2::Difference*           m_selectedDifference;

    QString                            m_source;
    QString                            m_destination;

    struct Kompare::Info*              m_info;
};

// These 3 classes are need to store the models into a tree so it is easier
// to extract the info we need for the navigation widgets

class KChangeLVI : public QTreeWidgetItem
{
public:
    KChangeLVI(QTreeWidget* parent, KompareDiff2::Difference* diff);
    ~KChangeLVI() override;
public:
    KompareDiff2::Difference* difference() { return m_difference; };
    bool operator<(const QTreeWidgetItem& item) const override;

    void setDifferenceText();
private:
    KompareDiff2::Difference* m_difference;
};

class KFileLVI : public QTreeWidgetItem
{
public:
    KFileLVI(QTreeWidget* parent, KompareDiff2::DiffModel* model);
    ~KFileLVI() override;
public:
    KompareDiff2::DiffModel* model() { return m_model; };
    void fillChangesList(QTreeWidget* changesList, QHash<const KompareDiff2::Difference*, KChangeLVI*>* diffToChangeItemDict);
private:
    bool hasExtension(const QString& extensions, const QString& fileName);
    const QString getIcon(const QString& fileName);
private:
    KompareDiff2::DiffModel* m_model;
};

class KDirLVI : public QTreeWidgetItem
{
public:
    KDirLVI(KDirLVI* parent, const QString& dir);
    KDirLVI(QTreeWidget* parent, const QString& dir);
    ~KDirLVI() override;
public:
    void addModel(QString& dir, KompareDiff2::DiffModel* model, QHash<const KompareDiff2::DiffModel*, KDirLVI*>* modelToDirItemDict);
    QString& dirName() { return m_dirName; };
    QString fullPath(QString& path);

    KDirLVI* setSelected(const QString& dir);
    void setSelected(bool selected) { QTreeWidgetItem::setSelected(selected); }

    void fillFileList(QTreeWidget* fileList, QHash<const KompareDiff2::DiffModel*, KFileLVI*>* modelToFileItemDict);
    bool isRootItem() { return m_rootItem; };

private:
    KDirLVI* findChild(const QString& dir);
private:
    KompareDiff2::DiffModelList m_modelList;
    QString m_dirName;
    bool m_rootItem;
};

#endif
