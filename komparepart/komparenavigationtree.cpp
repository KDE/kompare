/***************************************************************************
                                KompareNavigationTree.cpp  -  description
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

#include <qptrlist.h>

#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmimetype.h>
#include <klistview.h>

#include "difference.h"
#include "diffmodel.h"
#include "komparemodellist.h"

#include "komparenavigationtree.h"

#define COL_SOURCE        0
#define COL_DESTINATION   1
#define COL_DIFFERENCE    2

KompareNavigationTree::KompareNavigationTree( KompareModelList* models, QWidget* parent, const char* name ) :
	QSplitter( Qt::Horizontal, parent, name ),
	m_models( models ),
	m_srcDirTree( 0 ),
	m_destDirTree( 0 ),
	m_fileList( 0 ),
	m_changesList( 0 ),
	m_srcRootItem( 0 ),
	m_destRootItem( 0 )
{
	m_srcDirTree = new KListView( this );
	m_srcDirTree->addColumn( i18n("Source Directory") );
	m_srcDirTree->setRootIsDecorated( false );
	m_srcDirTree->setSorting( 0, true );

	m_destDirTree = new KListView( this );
	m_destDirTree->addColumn( i18n("Destination Directory") );
	m_destDirTree->setRootIsDecorated( false );
	m_destDirTree->setSorting( 0, true );

	m_fileList = new KListView( this );
	m_fileList->addColumn( i18n("Source File") );
	m_fileList->addColumn( i18n("Destination File") );
	m_fileList->setAllColumnsShowFocus( true );
	m_fileList->setRootIsDecorated( false );
	m_fileList->setSorting( 0, true );

	m_changesList = new KListView( this );
	m_changesList->addColumn( i18n("Source Line") );
	m_changesList->addColumn( i18n("Destination Line") );
	m_changesList->addColumn( i18n("Difference") );
	m_changesList->setAllColumnsShowFocus( true );
	m_changesList->setRootIsDecorated( false );
	m_changesList->setSorting( 0, true );

	connect( m_srcDirTree, SIGNAL(selectionChanged( QListViewItem* )),
	         this, SLOT(slotSrcDirTreeSelectionChanged( QListViewItem* )) );
	connect( m_destDirTree, SIGNAL(selectionChanged( QListViewItem* )),
	         this, SLOT(slotDestDirTreeSelectionChanged( QListViewItem* )) );
	connect( m_fileList, SIGNAL(selectionChanged( QListViewItem* )),
	         this, SLOT(slotFileListSelectionChanged( QListViewItem* )) );
	connect( m_changesList, SIGNAL(selectionChanged( QListViewItem* )),
	         this, SLOT(slotChangesListSelectionChanged( QListViewItem* )) );

	connect( models, SIGNAL(modelsChanged()),
	         this, SLOT(buildTreeInMemory()) );
}

KompareNavigationTree::~KompareNavigationTree()
{
}

void KompareNavigationTree::buildTreeInMemory()
{
	kdDebug() << "BuildTreeInMemory called" << endl;

	if ( m_models->modelCount() == 0 )
		return; // avoids a crash on clear()

	DiffModel* model;
	model = m_models->modelAt( 0 );

	QString srcPath  = model->srcPath();
	QString destPath = model->destPath();
	kdDebug() << "srcPath     = " << srcPath << endl;
	kdDebug() << "destPath    = " << destPath << endl;
	
	// result is the same piece minus a leading / -> we need all dirs to end with a /
	QString result = compareFromEndAndReturnSame( srcPath, destPath );
	kdDebug() << "result      = " << result << endl;

	QString srcBaseURL = m_models->sourceBaseURL().path() + srcPath.replace( srcPath.length() - result.length(), result.length(), "" );
	QString destBaseURL = m_models->destinationBaseURL().path() + destPath.replace( destPath.length() - result.length(), result.length(), "" );

	kdDebug() << "srcBaseURL  = " << srcBaseURL << endl;
	kdDebug() << "destBaseURL = " << destBaseURL <<endl;

	m_srcRootItem  = new KDirLVI( m_srcDirTree,  srcBaseURL  );
	m_destRootItem = new KDirLVI( m_destDirTree, destBaseURL );

	// Create the tree from the models
	QPtrListIterator<DiffModel> it( m_models->models() );
	while ( ( model = it.current() ) != 0L )
	{
		srcPath  = model->srcPath();
		destPath = model->destPath();

		kdDebug() << "srcPath  = " << srcPath  << endl;
		kdDebug() << "destPath = " << destPath << endl;
		m_srcRootItem->addModel( srcPath, model );
		m_destRootItem->addModel( destPath, model );
		++it;
	}
	m_srcDirTree->setSelected( m_srcDirTree->firstChild(), true );
}

void KompareNavigationTree::buildDirectoryTree()
{
	kdDebug() << "BuildDirTree called" << endl;
}

QString KompareNavigationTree::compareFromEndAndReturnSame(
    const QString& string1,
    const QString& string2 )
{
	QString result;

	int srcLen = string1.length();
	int destLen = string2.length();

	while ( srcLen != 0 && destLen != 0 )
	{
		if ( string1[--srcLen] == string2[--destLen] )
			result.prepend( string1[srcLen] );
		else
			break;
	}

	if ( srcLen != 0 && destLen != 0 && result.startsWith( "/" ) )
		result = result.remove( 0, 1 ); // strip leading /, we need it later

	return result;
}

void KompareNavigationTree::slotSetSelection( int model, int diff )
{
	if ( model >= 0 && diff >= 0 ) {
		KListViewItem* current = m_differenceToItemDict[ m_models->modelAt( model )->differenceAt( diff ) ];
		m_srcDirTree->setSelected( current, true );
	} else {
		m_srcDirTree->clearSelection();
	}
}

void KompareNavigationTree::slotSrcDirTreeSelectionChanged( QListViewItem* item )
{
	kdDebug() << "Sent by the sourceDirectoryTree with item = " << item << endl;
	KDirLVI* dir = static_cast<KDirLVI*>(item);
	// order the dest tree view to set its selected item to the same as here
	QString path;
	path = dir->fullPath( path );
	KDirLVI* selItem = m_destRootItem->setSelected( path );
	m_destDirTree->setSelected( selItem, true );
	// fill the changes list
	dir->fillFileList( m_fileList );
}

void KompareNavigationTree::slotDestDirTreeSelectionChanged( QListViewItem* item )
{
	kdDebug() << "Sent by the destinationDirectoryTree with item = " << item << endl;
	KDirLVI* dir = static_cast<KDirLVI*>(item);
	// order the src tree view to set its selected item to the same as here
	QString path;
	path = dir->fullPath( path );
	KDirLVI* selItem = m_srcRootItem->setSelected( path );
	m_srcDirTree->setSelected( selItem, true );
	// fill the changes list
	dir->fillFileList( m_fileList );
}

void KompareNavigationTree::slotFileListSelectionChanged( QListViewItem* item )
{
	kdDebug() << "Sent by the fileList with item = " << item << endl;

	KFileLVI* file = static_cast<KFileLVI*>(item);
	file->fillChangesList( m_changesList );
	int model = m_models->findModel( file->model() );

	emit selectionChanged( model, 0 );
}

void KompareNavigationTree::slotChangesListSelectionChanged( QListViewItem* item )
{
	kdDebug() << "Sent by the changesList" << endl;

	KChangeLVI* change = static_cast<KChangeLVI*>(item);
	KFileLVI* file = static_cast<KFileLVI*>(m_fileList->selectedItem());
	int model = m_models->findModel( file->model() );
	int diff = file->model()->findDifference( change->difference() );

	emit selectionChanged( model, diff );
}

void KompareNavigationTree::slotAppliedChanged( const Difference* d )
{
	//setItemText( m_differenceToItemDict[(void*)d], d );
}

KChangeLVI::KChangeLVI( KListView* parent, Difference* diff ) : KListViewItem( parent )
{
	m_difference = diff;

	setText( 0, i18n( "%1" ).arg( diff->sourceLineNumber() ) );
	setText( 1, i18n( "%1" ).arg( diff->destinationLineNumber() ) );

	QString text = "";
	switch( diff->type() ) {
	case Difference::Change:
		text = i18n( "Changed line", "Changed lines",
		             QMAX( diff->sourceLineCount(),
		             diff->destinationLineCount() ) );
			break;
	case Difference::Insert:
		text = i18n( "Inserted line", "Inserted lines",
		             diff->destinationLineCount() );
			break;
	case Difference::Delete:
	default:
		text = i18n( "Deleted line", "Deleted lines",
		             diff->sourceLineCount() );
			break;
	}

	if( diff->applied() ) {
			text = i18n( "Applied: %1" ).arg( text );
	}

	setText( 2, text );
}

int KChangeLVI::compare( QListViewItem* item, int column, bool ascending ) const
{
	if ( ascending )
	{
		if ( this->text(column).length() < item->text(column).length() )
			return -1;
		if ( this->text(column).length() > item->text(column).length() )
			return 1;
	}
	else
	{
		if ( this->text(column).length() > item->text(column).length() )
			return -1;
		if ( this->text(column).length() < item->text(column).length() )
			return 1;
	}

	return key( column, ascending ).compare( item->key( column, ascending ) );
}

KChangeLVI::~KChangeLVI()
{
}

KFileLVI::KFileLVI( KListView* parent, DiffModel* model ) : KListViewItem( parent )
{
	m_model = model;

	setText( 0, model->sourceFile() );
	setText( 1, model->destinationFile() );
	setPixmap( 0, SmallIcon( "file" ) );
	setPixmap( 1, SmallIcon( "file" ) );
	setSelectable( true );
}

void KFileLVI::fillChangesList( KListView* changesList )
{
	changesList->clear();
	
	QPtrListIterator<Difference> it( m_model->differences() );
	Difference* diff;
	while ( ( diff = it.current() ) != 0L )
	{
		new KChangeLVI( changesList, diff );
		++it;
	}
	
	changesList->setSelected( changesList->firstChild(), true );
}

KFileLVI::~KFileLVI()
{
}

KDirLVI::KDirLVI( KListView* parent, QString& dir ) : KListViewItem( parent )
{
	kdDebug() << "KDirLVI (KListView) constructor called with dir = " << dir << endl;
	m_bRootItem = true;
	m_dirName = dir;
	setPixmap( 0, SmallIcon( "folder" ) );
	setOpen( true );
	setSelectable( true );
	setText( 0, m_dirName );
}

KDirLVI::KDirLVI( KDirLVI* parent, QString& dir ) : KListViewItem( parent )
{
	kdDebug() << "KDirLVI (KDirLVI) constructor called with dir = " << dir << endl;
	m_bRootItem = false;
	m_dirName = dir;
	setPixmap( 0, SmallIcon( "folder" ) );
	setOpen( true );
	setSelectable( true );
	setText( 0, m_dirName );
}

// addModel always removes it own path from the beginning
void KDirLVI::addModel( QString& path, DiffModel* model )
{
	kdDebug() << "KDirLVI::addModel called with path = " << path << " from KDirLVI with m_dirName = " << m_dirName << endl;

	path = path.remove( 0, m_dirName.length() );
	kdDebug() << "Path after removal of own dir = " << path << endl;

	if ( path.isEmpty() ) {
		m_modelList.append( model );
		return;
	}

	KDirLVI* child;

	QString dir = path.mid( 0, path.find( "/", 0 ) + 1 );
	child = findChild( dir );
	if ( !child )
	{
		// does not exist yet so make it
		kdDebug() << "KDirLVI::addModel creating new KDirLVI because not found" << endl;
		child = new KDirLVI( this, dir );
	}

	child->addModel( path, model );
}

KDirLVI* KDirLVI::findChild( QString dir )
{
	kdDebug() << "KDirLVI::findChild called with dir = " << dir << endl;
	KDirLVI* child;
	if ( ( child = static_cast<KDirLVI*>(this->firstChild()) ) != 0L )
	{ // has children, check if dir already exists, if so addModel
		do {
			if ( dir == child->dirName() )
				return child;
		} while ( ( child = static_cast<KDirLVI*>(child->nextSibling()) ) != 0L );
	}

	return 0L;
}

void KDirLVI::fillFileList( KListView* fileList )
{
	fileList->clear();

	QPtrListIterator<DiffModel> it( m_modelList );
	DiffModel* model;
	while ( ( model = it.current() ) != 0L )
	{
		new KFileLVI( fileList, model );
		++it;
	}
	
	fileList->setSelected( fileList->firstChild(), true );
}

QString KDirLVI::fullPath( QString& path )
{
	if ( !path.isEmpty() )
		kdDebug() << "KDirLVI::fullPath called with path = " << path << endl;
	else
		kdDebug() << "KDirLVI::fullPath called with empty path..." << endl;

	if ( m_bRootItem ) // dont bother adding rootItem's dir...
		return path;

	path = path.prepend( m_dirName );

	KDirLVI* lviParent = dynamic_cast<KDirLVI*>( parent() );
	if ( lviParent )
	{
		path = lviParent->fullPath( path );
	}

	return path;
}

KDirLVI* KDirLVI::setSelected( QString dir )
{
	kdDebug() << "KDirLVI::setSelected called with dir = " << dir << endl;

	// root item's dirName is never taken into account... remember that
	if ( !m_bRootItem )
	{
		dir = dir.remove( 0, m_dirName.length() );
	}

	if ( dir.isEmpty() )
	{
		return this;
	}

	KDirLVI* child = static_cast<KDirLVI*>(firstChild());
	if ( !child )
		return 0L;

	do {
		if ( child->dirName().startsWith( dir ) )
			return child->setSelected( dir );
	} while ( ( child = static_cast<KDirLVI*>(child->nextSibling()) ) != 0L );

	return 0L;
}

KDirLVI::~KDirLVI()
{
}

#include "komparenavigationtree.moc"
