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
#include <kparts_version.h>
#include <KParts/ReadOnlyPart>
// libkomparediff2
#include <libkomparediff2/kompare.h>
#include <libkomparediff2/diffmodellist.h>

namespace Diff2 {
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
    void slotSetSelection(const Diff2::DiffModel* model, const Diff2::Difference* diff);
    void slotSetSelection(const Diff2::Difference* diff);
    void slotModelsChanged(const Diff2::DiffModelList* modelList);
    void slotKompareInfo(Kompare::Info* info);

Q_SIGNALS:
    void selectionChanged(const Diff2::DiffModel* model, const Diff2::Difference* diff);
    void selectionChanged(const Diff2::Difference* diff);

private Q_SLOTS:
    void slotSrcDirTreeSelectionChanged(QTreeWidgetItem* item);
    void slotDestDirTreeSelectionChanged(QTreeWidgetItem* item);
    void slotFileListSelectionChanged(QTreeWidgetItem* item);
    void slotChangesListSelectionChanged(QTreeWidgetItem* item);

    void slotApplyDifference(bool apply);
    void slotApplyAllDifferences(bool apply);
    void slotApplyDifference(const Diff2::Difference* diff, bool apply);

    void buildTreeInMemory();

private:
    void setSelectedDir(const Diff2::DiffModel* model);
    void setSelectedFile(const Diff2::DiffModel* model);
    void setSelectedDifference(const Diff2::Difference* diff);

    void buildDirectoryTree();

    QString compareFromEndAndReturnSame(const QString& string1, const QString& string2);
    void addDirToTreeView(enum Kompare::Target, const QString& filename);

    QTreeWidgetItem* findDirInDirTree(const QTreeWidgetItem* parent, const QString& dir);

private:
    QSplitter*                         m_splitter;
    const Diff2::DiffModelList*        m_modelList;

    QHash<const Diff2::Difference*, KChangeLVI*>    m_diffToChangeItemDict;
    QHash<const Diff2::DiffModel*, KFileLVI*>       m_modelToFileItemDict;
    QHash<const Diff2::DiffModel*, KDirLVI*>        m_modelToSrcDirItemDict;
    QHash<const Diff2::DiffModel*, KDirLVI*>        m_modelToDestDirItemDict;

    QTreeWidget*                       m_srcDirTree;
    QTreeWidget*                       m_destDirTree;
    QTreeWidget*                       m_fileList;
    QTreeWidget*                       m_changesList;

    KDirLVI*                           m_srcRootItem;
    KDirLVI*                           m_destRootItem;

    const Diff2::DiffModel*            m_selectedModel;
    const Diff2::Difference*           m_selectedDifference;

    QString                            m_source;
    QString                            m_destination;

    struct Kompare::Info*              m_info;
};

// These 3 classes are need to store the models into a tree so it is easier
// to extract the info we need for the navigation widgets

class KChangeLVI : public QTreeWidgetItem
{
public:
    KChangeLVI(QTreeWidget* parent, Diff2::Difference* diff);
    ~KChangeLVI() override;
public:
    Diff2::Difference* difference() { return m_difference; };
    bool operator<(const QTreeWidgetItem& item) const override;

    void setDifferenceText();
private:
    Diff2::Difference* m_difference;
};

class KFileLVI : public QTreeWidgetItem
{
public:
    KFileLVI(QTreeWidget* parent, Diff2::DiffModel* model);
    ~KFileLVI() override;
public:
    Diff2::DiffModel* model() { return m_model; };
    void fillChangesList(QTreeWidget* changesList, QHash<const Diff2::Difference*, KChangeLVI*>* diffToChangeItemDict);
private:
    bool hasExtension(const QString& extensions, const QString& fileName);
    const QString getIcon(const QString& fileName);
private:
    Diff2::DiffModel* m_model;
};

class KDirLVI : public QTreeWidgetItem
{
public:
    KDirLVI(KDirLVI* parent, const QString& dir);
    KDirLVI(QTreeWidget* parent, const QString& dir);
    ~KDirLVI() override;
public:
    void addModel(QString& dir, Diff2::DiffModel* model, QHash<const Diff2::DiffModel*, KDirLVI*>* modelToDirItemDict);
    QString& dirName() { return m_dirName; };
    QString fullPath(QString& path);

    KDirLVI* setSelected(const QString& dir);
    void setSelected(bool selected) { QTreeWidgetItem::setSelected(selected); }

    void fillFileList(QTreeWidget* fileList, QHash<const Diff2::DiffModel*, KFileLVI*>* modelToFileItemDict);
    bool isRootItem() { return m_rootItem; };

private:
    KDirLVI* findChild(const QString& dir);
private:
    Diff2::DiffModelList m_modelList;
    QString m_dirName;
    bool m_rootItem;
};

#endif
