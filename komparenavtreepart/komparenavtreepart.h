/***************************************************************************
                                komparenavtreepart.h
                                --------------------
        begin                   : Mon Feb 26 2002
        Copyright 2001-2004 Otto Bruggeman <otto.bruggeman@home.nl>
        Copyright 2001-2003 John Firebaugh <jfirebaugh@kde.org>
        Copyright 2007-2011 Kevin Kofler   <kevin.kofler@chello.at>
****************************************************************************/

/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***************************************************************************/

#ifndef KOMPARENAVTREEPART_H
#define KOMPARENAVTREEPART_H

#include <QtCore/QHash>

#include <QtGui/QSplitter>
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>

#include <kparts/part.h>

#include "kompare.h"
#include "diffmodellist.h"

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
	explicit KompareNavTreePart( QWidget* parentWidget, QObject* parent, const QVariantList& args );
	virtual ~KompareNavTreePart();

public:
	virtual bool openFile() { return false; };

public slots:
	void slotSetSelection( const Diff2::DiffModel* model, const Diff2::Difference* diff );
	void slotSetSelection( const Diff2::Difference* diff );
	void slotModelsChanged( const Diff2::DiffModelList* modelList );
	void slotKompareInfo( Kompare::Info* info );

signals:
	void selectionChanged( const Diff2::DiffModel* model, const Diff2::Difference* diff );
	void selectionChanged( const Diff2::Difference* diff );

private slots:
	void slotSrcDirTreeSelectionChanged ( QTreeWidgetItem* item );
	void slotDestDirTreeSelectionChanged( QTreeWidgetItem* item );
	void slotFileListSelectionChanged   ( QTreeWidgetItem* item );
	void slotChangesListSelectionChanged( QTreeWidgetItem* item );

	void slotApplyDifference( bool apply );
	void slotApplyAllDifferences( bool apply );
	void slotApplyDifference( const Diff2::Difference* diff, bool apply );

	void buildTreeInMemory();

private:
	void setSelectedDir( const Diff2::DiffModel* model );
	void setSelectedFile( const Diff2::DiffModel* model );
	void setSelectedDifference( const Diff2::Difference* diff );

	void buildDirectoryTree();

	QString compareFromEndAndReturnSame( const QString& string1, const QString& string2 );
	void addDirToTreeView( enum Kompare::Target, const QString& filename );

	QTreeWidgetItem* findDirInDirTree( const QTreeWidgetItem* parent, const QString& dir );

private:
	QSplitter*                         m_splitter;
	const Diff2::DiffModelList*        m_modelList;

	QHash<const Diff2::Difference*, KChangeLVI*>	m_diffToChangeItemDict;
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
	KChangeLVI( QTreeWidget* parent, Diff2::Difference* diff );
	~KChangeLVI();
public:
	Diff2::Difference* difference() { return m_difference; };
	virtual bool operator<( const QTreeWidgetItem& item ) const;

	void setDifferenceText();
private:
	Diff2::Difference* m_difference;
};

class KFileLVI : public QTreeWidgetItem
{
public:
	KFileLVI( QTreeWidget* parent, Diff2::DiffModel* model );
	~KFileLVI();
public:
	Diff2::DiffModel* model() { return m_model; };
	void fillChangesList( QTreeWidget* changesList, QHash<const Diff2::Difference*, KChangeLVI*>* diffToChangeItemDict );
private:
	bool hasExtension(const QString& extensions, const QString& fileName);
	const QString getIcon(const QString& fileName);
private:
	Diff2::DiffModel* m_model;
};

class KDirLVI : public QTreeWidgetItem
{
public:
	KDirLVI( KDirLVI* parent, QString& dir );
	KDirLVI( QTreeWidget* parent, QString& dir );
	~KDirLVI();
public:
	void addModel( QString& dir, Diff2::DiffModel* model, QHash<const Diff2::DiffModel*, KDirLVI*>* modelToDirItemDict );
	QString& dirName() { return m_dirName; };
	QString fullPath( QString& path );

	KDirLVI* setSelected( QString dir );
	void setSelected( bool selected ) { QTreeWidgetItem::setSelected( selected ); }

	void fillFileList( QTreeWidget* fileList, QHash<const Diff2::DiffModel*, KFileLVI*>* modelToFileItemDict );
	bool isRootItem() { return m_rootItem; };

private:
	KDirLVI* findChild( QString dir );
private:
	Diff2::DiffModelList m_modelList;
	QString m_dirName;
	bool m_rootItem;
};

#endif
