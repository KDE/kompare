/***************************************************************************
                                komparenavtreepart.h  -  description
                                -------------------
        begin                   : Mon Feb 26 2002
        copyright               : (C) 2001-2004 Otto Bruggeman
                                  (C) 2001-2003 John Firebaugh
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

#ifndef KOMPARENAVTREEPART_H
#define KOMPARENAVTREEPART_H

#include <qptrdict.h>
#include <qptrlist.h>
#include <qsplitter.h>
#include <qlistview.h>

#include <kparts/factory.h>
#include <kparts/part.h>

#include "kompare.h"
#include "diffmodellist.h"

class KompareModelList;
class KomparePart;
class KListView;

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
	KompareNavTreePart( QWidget* parent = 0L, const char* name = 0L );
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
	void slotSrcDirTreeSelectionChanged ( QListViewItem* item );
	void slotDestDirTreeSelectionChanged( QListViewItem* item );
	void slotFileListSelectionChanged   ( QListViewItem* item );
	void slotChangesListSelectionChanged( QListViewItem* item );

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

	KListViewItem* findDirInDirTree( const KListViewItem* parent, const QString& dir );

//	KListViewItem* firstItem();
//	KListViewItem* lastItem();

private:
	QSplitter*                         m_splitter;
	const Diff2::DiffModelList*        m_modelList;

	QPtrDict<KChangeLVI>               m_diffToChangeItemDict;
	QPtrDict<KFileLVI>                 m_modelToFileItemDict;
	QPtrDict<KDirLVI>                  m_modelToSrcDirItemDict;
	QPtrDict<KDirLVI>                  m_modelToDestDirItemDict;

	KListView*                         m_srcDirTree;
	KListView*                         m_destDirTree;
	KListView*                         m_fileList;
	KListView*                         m_changesList;

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

class KChangeLVI : public KListViewItem
{
public:
	KChangeLVI( KListView* parent, Diff2::Difference* diff );
	~KChangeLVI();
public:
	Diff2::Difference* difference() { return m_difference; };
	virtual int compare( QListViewItem* item, int column, bool ascending ) const;

	void setDifferenceText();
private:
	Diff2::Difference* m_difference;
};

class KFileLVI : public KListViewItem
{
public:
	KFileLVI( KListView* parent, Diff2::DiffModel* model );
	~KFileLVI();
public:
	Diff2::DiffModel* model() { return m_model; };
	void fillChangesList( KListView* changesList, QPtrDict<KChangeLVI>* diffToChangeItemDict );
private:
	Diff2::DiffModel* m_model;
};

class KDirLVI : public KListViewItem
{
public:
	KDirLVI( KDirLVI* parent, QString& dir );
	KDirLVI( KListView* parent, QString& dir );
	~KDirLVI();
public:
	void addModel( QString& dir, Diff2::DiffModel* model, QPtrDict<KDirLVI>* modelToDirItemDict );
	QString& dirName() { return m_dirName; };
	QString fullPath( QString& path );
	KDirLVI* setSelected( QString dir );
	void fillFileList( KListView* fileList, QPtrDict<KFileLVI>* modelToFileItemDict );
	bool isRootItem() { return m_rootItem; };
private:
	KDirLVI* findChild( QString dir );
private:
	Diff2::DiffModelList m_modelList;
	QString m_dirName;
	bool m_rootItem;
};

// part stuff
class KInstance;
class KAboutData;

class KompareNavTreePartFactory : public KParts::Factory
{
	Q_OBJECT
public:
	KompareNavTreePartFactory();
	virtual ~KompareNavTreePartFactory();
	virtual KParts::Part* createPartObject( QWidget *parentWidget, const char *widgetName,
	                                        QObject *parent, const char *name,
	                                        const char *classname, const QStringList &args );
	static KInstance* instance();

private:
	static KInstance* s_instance;
	static KAboutData* s_about;
};

#endif
