/***************************************************************************
                                komparenavtreepart.h  -  description
                                -------------------
        begin                   : Mon Feb 26 2002
        copyright               : (C) 2002 by Otto Bruggeman
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

#ifndef KOMPARENAVTREEPART_H
#define KOMPARENAVTREEPART_H

#include <qptrdict.h>
#include <qptrlist.h>
#include <qsplitter.h>
#include <qlistview.h>

#include <kparts/factory.h>
#include <kparts/part.h>

class KompareModelList;
class KomparePart;
class KListView;
class DiffModel;
class Difference;

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
	void slotSetSelection( const DiffModel* model, const Difference* diff );
	void slotSetSelection( const Difference* diff );
	void slotModelsChanged( const QPtrList<DiffModel>* modelList );

signals:
	void selectionChanged( const DiffModel* model, const Difference* diff );
	void selectionChanged( const Difference* diff );

private slots:
	void slotSrcDirTreeSelectionChanged ( QListViewItem* item );
	void slotDestDirTreeSelectionChanged( QListViewItem* item );
	void slotFileListSelectionChanged   ( QListViewItem* item );
	void slotChangesListSelectionChanged( QListViewItem* item );

	void slotApplyDifference( bool apply );
	void slotApplyAllDifferences( bool apply );

	void buildTreeInMemory();

private:
	void setSelectedDir( const DiffModel* model );
	void setSelectedFile( const DiffModel* model );
	void setSelectedDifference( const Difference* diff );

	void buildDirectoryTree();

	QString compareFromEndAndReturnSame( const QString& string1, const QString& string2 );
	void addDirToTreeView( enum Kompare::Target, const QString& filename );

	KListViewItem* findDirInDirTree( const KListViewItem* parent, const QString& dir );

//	KListViewItem* firstItem();
//	KListViewItem* lastItem();

private:
	QSplitter*                  m_splitter;
	const QPtrList<DiffModel>*  m_modelList;

	QPtrDict<KChangeLVI>        m_diffToChangeItemDict;
	QPtrDict<KFileLVI>          m_modelToFileItemDict;
	QPtrDict<KDirLVI>           m_modelToSrcDirItemDict;
	QPtrDict<KDirLVI>           m_modelToDestDirItemDict;

	KListView*                  m_srcDirTree;
	KListView*                  m_destDirTree;
	KListView*                  m_fileList;
	KListView*                  m_changesList;

	KDirLVI*                    m_srcRootItem;
	KDirLVI*                    m_destRootItem;

	const DiffModel*            m_selectedModel;
	const Difference*           m_selectedDifference;
};

// These 3 classes are need to store the models into a tree so it is easier
// to extract the info we need for the navigation widgets

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

class KFileLVI : public KListViewItem, private Kompare
{
public:
	KFileLVI( KListView* parent, DiffModel* model );
	~KFileLVI();
public:
	DiffModel* model() { return m_model; };
	void fillChangesList( KListView* changesList, QPtrDict<KChangeLVI>* diffToChangeItemDict );
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
	void addModel( QString& dir, DiffModel* model, QPtrDict<KDirLVI>* modelToDirItemDict );
	QString& dirName() { return m_dirName; };
	QString fullPath( QString& path );
	KDirLVI* setSelected( QString dir );
	void fillFileList( KListView* fileList, QPtrDict<KFileLVI>* modelToFileItemDict );
	bool isRootItem() { return m_rootItem; };
private:
	KDirLVI* findChild( QString dir );
private:
	QPtrList<DiffModel> m_modelList;
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
