/***************************************************************************
                                komparenavigationtree.h  -  description
                                -------------------
        begin                   : Sun Mar 4 2001
        copyright               : (C) 2001 by Otto Bruggeman
                                  and John Firebaugh
        email                   : otto.bruggeman@home.nl
                                  jfirebaugh@kde.org
****************************************************************************/

/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***************************************************************************/

#ifndef KOMPARENAVIGATIONTREE_H
#define KOMPARENAVIGATIONTREE_H

#include <qptrdict.h>
#include <qsplitter.h>
#include <qlistview.h>


class KompareModelList;
class KomparePart;
class KListView;

class KDirLVI;

class KompareNavigationTree : public QSplitter
{
	Q_OBJECT

public:
	KompareNavigationTree( KompareModelList* models, QWidget* parent = 0L, const char* name = 0L );
	virtual ~KompareNavigationTree();

public slots:
	void slotSetSelection( int model, int diff );

signals:
	void selectionChanged( int model, int diff );

private slots:
	void slotSrcDirTreeSelectionChanged( QListViewItem* item );
	void slotDestDirTreeSelectionChanged( QListViewItem* item );
	void slotFileListSelectionChanged( QListViewItem* item );
	void slotChangesListSelectionChanged( QListViewItem* item );

	void slotAppliedChanged( const Difference* d );

private slots:
	void buildTreeInMemory();

private:
	void buildDirectoryTree();
//	void buildFileList( const QString& dir );
//	void buildChangesList( int change );

	QString compareFromEndAndReturnSame( const QString& string1, const QString& string2 );
	void addDirToTreeView( enum Kompare::Target, const QString& filename );

	KListViewItem* findDirInDirTree( const KListViewItem* parent, const QString& dir );

//	KListViewItem* firstItem();
//	KListViewItem* lastItem();

private:
	KompareModelList*        m_models;

	QPtrDict<KListViewItem>  m_differenceToItemDict;

	KListView*               m_srcDirTree;
	KListView*               m_destDirTree;
	KListView*               m_fileList;
	KListView*               m_changesList;

	KDirLVI*                 m_srcRootItem;
	KDirLVI*                 m_destRootItem;
};

// These 4 classes are need to store the models into a tree so it is easier
// to extract the info we need for the navigation widgets
class Difference;

class KChangeLVI : public KListViewItem, private Kompare
{
public:
	KChangeLVI( KListView* parent, Difference* diff );
	~KChangeLVI();
public:
	Difference* difference() { return m_difference; };
	virtual int compare( QListViewItem* item, int column, bool ascending ) const;
private:
	Difference* m_difference;
};

class DiffModel;

class KFileLVI : public KListViewItem, private Kompare
{
public:
	KFileLVI( KListView* parent, DiffModel* model );
	~KFileLVI();
public:
	DiffModel* model() { return m_model; };
	void fillChangesList( KListView* changesList );
private:
	DiffModel* m_model;
};

class KDirLVI : public KListViewItem, private Kompare
{
public:
	KDirLVI( KDirLVI* parent, QString& dir );
	KDirLVI( KListView* parent, QString& dir );
	~KDirLVI();
public:
	void addModel( QString& dir, DiffModel* model );
	QString& dirName() { return m_dirName; };
	QString fullPath( QString& path );
	KDirLVI* setSelected( QString dir );
	void fillFileList( KListView* fileList );
	bool isRootItem() { return m_bRootItem; };
private:
	KDirLVI* findChild( QString dir );
private:
	QPtrList<DiffModel> m_modelList;
	QString m_dirName;
	bool m_bRootItem;
};

#endif
