/***************************************************************************
                                KompareNavTreePart.cpp  -  description
                                -------------------
        begin                   : Sun Mar 4 2001
        copyright               : (C) 2001-2003 by Otto Bruggeman
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
#include <kaboutdata.h>

#include "difference.h"
#include "diffmodel.h"
#include "komparemodellist.h"

#include "komparenavtreepart.h"

#define COL_SOURCE        0
#define COL_DESTINATION   1
#define COL_DIFFERENCE    2

KompareNavTreePart::KompareNavTreePart( QWidget* parent, const char* name )
	: KParts::ReadOnlyPart( parent, name ),
	m_splitter( 0 ),
	m_modelList( 0 ),
	m_srcDirTree( 0 ),
	m_destDirTree( 0 ),
	m_fileList( 0 ),
	m_changesList( 0 ),
	m_srcRootItem( 0 ),
	m_destRootItem( 0 ),
	m_info( 0 )
{
	m_splitter = new QSplitter( Qt::Horizontal );

	setWidget( m_splitter );

	m_srcDirTree = new KListView( m_splitter );
	m_srcDirTree->addColumn( i18n("Source Folder") );
	m_srcDirTree->setRootIsDecorated( false );
	m_srcDirTree->setSorting( 0, true );

	m_destDirTree = new KListView( m_splitter );
	m_destDirTree->addColumn( i18n("Destination Folder") );
	m_destDirTree->setRootIsDecorated( false );
	m_destDirTree->setSorting( 0, true );

	m_fileList = new KListView( m_splitter );
	m_fileList->addColumn( i18n("Source File") );
	m_fileList->addColumn( i18n("Destination File") );
	m_fileList->setAllColumnsShowFocus( true );
	m_fileList->setRootIsDecorated( false );
	m_fileList->setSorting( 0, true );

	m_changesList = new KListView( m_splitter );
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
}

KompareNavTreePart::~KompareNavTreePart()
{
}

void KompareNavTreePart::slotKompareInfo( struct Kompare::Info* info )
{
	m_info = info;
}

void KompareNavTreePart::slotModelsChanged( const QPtrList<Diff2::DiffModel>* modelList )
{
	kdDebug(8105) << "Models have changed... scanning the models... " << endl;

	if ( modelList )
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
	kdDebug(8105) << "BuildTreeInMemory called" << endl;

	if ( m_modelList->count() == 0 )
		return; // avoids a crash on clear()

	if ( m_info == 0 )
		return;

	QString srcBase;
	QString destBase;

	Diff2::DiffModel* model;
	model = m_modelList->getFirst();
	m_selectedModel = 0L;

	switch ( m_info->mode )
	{
	case Kompare::ShowingDiff:
		srcBase = model->sourcePath();
		destBase = model->destinationPath();
		break;
	case Kompare::ComparingFiles:
		srcBase  = model->sourcePath();
		destBase = model->destinationPath();
		break;
	case Kompare::ComparingDirs:
		srcBase = m_info->localSource;
		if ( !srcBase.endsWith( "/" ) )
			srcBase += "/";
		destBase = m_info->localDestination;
		if ( !destBase.endsWith( "/" ) )
			destBase += "/";
		break;
	case Kompare::BlendingFile:
	case Kompare::BlendingDir:
	default:
		kdDebug(8105) << "Oops needs to implement this..." << endl;
	}

//	kdDebug(8105) << "srcBase  = " << srcBase << endl;
//	kdDebug(8105) << "destBase = " << destBase << endl;

	m_srcRootItem  = new KDirLVI( m_srcDirTree, srcBase );
	m_destRootItem = new KDirLVI( m_destDirTree, destBase );

	QString srcPath;
	QString destPath;

	// Create the tree from the models
	QPtrListIterator<Diff2::DiffModel> it( *m_modelList );
	while ( ( model = it.current() ) != 0L )
	{
		srcPath  = model->sourcePath();
		destPath = model->destinationPath();

//		kdDebug(8105) << "srcPath  = " << srcPath  << endl;
//		kdDebug(8105) << "destPath = " << destPath << endl;
		m_srcRootItem->addModel( srcPath, model, &m_modelToSrcDirItemDict );
		m_destRootItem->addModel( destPath, model, &m_modelToDestDirItemDict );
		++it;
	}
//	m_srcDirTree->setSelected( m_srcDirTree->firstChild(), true );
}

void KompareNavTreePart::buildDirectoryTree()
{
// FIXME: afaict this can be deleted
//	kdDebug(8105) << "BuildDirTree called" << endl;
}

QString KompareNavTreePart::compareFromEndAndReturnSame(
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

void KompareNavTreePart::slotSetSelection( const Diff2::DiffModel* model, const Diff2::Difference* diff )
{
	kdDebug(8105) << "KompareNavTreePart::slotSetSelection model = " << model << ", diff = " << diff << endl;
	if ( model == m_selectedModel )
	{
		// model is the same, so no need to update that...
		if ( diff != m_selectedDifference )
		{
			m_selectedDifference = diff;
			setSelectedDifference( diff );
		}
		return;
	}

	// model is different so we need to find the right dirs, file and changeitems to select
	// if m_selectedModel == NULL then everything needs to be done as well
	if ( !m_selectedModel || model->sourcePath() != m_selectedModel->sourcePath() )
	{   // dirs are different, so we need to update the dirviews as well
		m_selectedModel = model;
		m_selectedDifference = diff;

		setSelectedDir( model );
		setSelectedFile( model );
		setSelectedDifference( diff );
		return;
	}

	if ( !m_selectedModel || model->sourceFile() != m_selectedModel->sourceFile() )
	{
		m_selectedModel = model;
		setSelectedFile( model );

		m_selectedDifference = diff;
		setSelectedDifference( diff );
	}
}

void KompareNavTreePart::setSelectedDir( const Diff2::DiffModel* model )
{
	KDirLVI* currentDir;
	currentDir = m_modelToSrcDirItemDict[ (void*)model ];
	kdDebug(8105) << "Manually setting selection in srcdirtree" << endl;
	m_srcDirTree->blockSignals( true );
	m_srcDirTree->setSelected( currentDir, true );
	m_srcDirTree->ensureItemVisible( currentDir );
	m_srcDirTree->blockSignals( false );

	currentDir = m_modelToDestDirItemDict[ (void*)model ];
	kdDebug(8105) << "Manually setting selection in destdirtree" << endl;
	m_destDirTree->blockSignals( true );
	m_destDirTree->setSelected( currentDir, true );
	m_destDirTree->ensureItemVisible( currentDir );
	m_destDirTree->blockSignals( false );

	m_fileList->blockSignals( true );
	currentDir->fillFileList( m_fileList, &m_modelToFileItemDict );
	m_fileList->blockSignals( false );
}

void KompareNavTreePart::setSelectedFile( const Diff2::DiffModel* model )
{
	KFileLVI* currentFile;
	currentFile = m_modelToFileItemDict[ (void*)model ];
	kdDebug(8105) << "Manually setting selection in filelist" << endl;
	m_fileList->blockSignals( true );
	m_fileList->setSelected( currentFile, true );
	m_fileList->ensureItemVisible( currentFile );
	m_fileList->blockSignals( false );

	m_changesList->blockSignals( true );
	currentFile->fillChangesList( m_changesList, &m_diffToChangeItemDict );
	m_changesList->blockSignals( false );
}

void KompareNavTreePart::setSelectedDifference( const Diff2::Difference* diff )
{
	KChangeLVI* currentDiff;
	currentDiff = m_diffToChangeItemDict[ (void*)diff ];
	kdDebug(8105) << "Manually setting selection in changeslist to " << currentDiff << endl;
	m_changesList->blockSignals( true );
	m_changesList->setSelected( currentDiff, true );
	m_changesList->ensureItemVisible( currentDiff );
	m_changesList->blockSignals( false );
}

void KompareNavTreePart::slotSetSelection( const Diff2::Difference* diff )
{
//	kdDebug(8105) << "Scotty i need more power !!" << endl;
	if ( m_selectedDifference != diff )
	{
//		kdDebug(8105) << "But sir, i am giving you all she's got" << endl;
		m_selectedDifference = diff;
		setSelectedDifference( diff );
	}
}

void KompareNavTreePart::slotSrcDirTreeSelectionChanged( QListViewItem* item )
{
	kdDebug(8105) << "Sent by the sourceDirectoryTree with item = " << item << endl;
	m_srcDirTree->ensureItemVisible( item );
	KDirLVI* dir = static_cast<KDirLVI*>(item);
	// order the dest tree view to set its selected item to the same as here
	QString path;
	// We start with an empty path and after the call path contains the full path
	path = dir->fullPath( path );
	KDirLVI* selItem = m_destRootItem->setSelected( path );
	m_destDirTree->blockSignals( true );
	m_destDirTree->setSelected( selItem, true );
	m_destDirTree->ensureItemVisible( selItem );
	m_destDirTree->blockSignals( false );
	// fill the changes list
	dir->fillFileList( m_fileList, &m_modelToFileItemDict );
}

void KompareNavTreePart::slotDestDirTreeSelectionChanged( QListViewItem* item )
{
	kdDebug(8105) << "Sent by the destinationDirectoryTree with item = " << item << endl;
	m_destDirTree->ensureItemVisible( item );
	KDirLVI* dir = static_cast<KDirLVI*>(item);
	// order the src tree view to set its selected item to the same as here
	QString path;
	// We start with an empty path and after the call path contains the full path
	path = dir->fullPath( path );
	KDirLVI* selItem = m_srcRootItem->setSelected( path );
	m_srcDirTree->blockSignals( true );
	m_srcDirTree->setSelected( selItem, true );
	m_srcDirTree->ensureItemVisible( selItem );
	m_srcDirTree->blockSignals( false );
	// fill the changes list
	dir->fillFileList( m_fileList, &m_modelToFileItemDict );
}

void KompareNavTreePart::slotFileListSelectionChanged( QListViewItem* item )
{
	kdDebug(8105) << "Sent by the fileList with item = " << item << endl;

	KFileLVI* file = static_cast<KFileLVI*>(item);
	m_selectedModel = file->model();
	m_changesList->blockSignals( true );
	file->fillChangesList( m_changesList, &m_diffToChangeItemDict );
	m_changesList->blockSignals( false );

	m_selectedDifference = (static_cast<KChangeLVI*>(m_changesList->selectedItem()))->difference();

	emit selectionChanged( m_selectedModel, m_selectedDifference );
}

void KompareNavTreePart::slotChangesListSelectionChanged( QListViewItem* item )
{
	kdDebug(8105) << "Sent by the changesList" << endl;

	KChangeLVI* change = static_cast<KChangeLVI*>(item);
	m_selectedDifference = change->difference();

	emit selectionChanged( m_selectedDifference );
}

void KompareNavTreePart::slotApplyDifference( bool /*apply*/ )
{
	// this applies to the currently selected difference
	// setItemText( m_differenceToItemDict[(void*)d], d );
}

void KompareNavTreePart::slotApplyAllDifferences( bool /*apply*/ )
{
	// this applies to the currently selected model
	// setItemText( m_differenceToItemDict[(void*)d], d );
}

void KompareNavTreePart::slotApplyDifference( const Diff2::Difference* /*diff*/, bool /*apply*/ )
{
	// this applies to the currently selected difference
	// setItemText( m_differenceToItemDict[(void*)d], d );
}

KChangeLVI::KChangeLVI( KListView* parent, Diff2::Difference* diff ) : KListViewItem( parent )
{
	m_difference = diff;

	setText( 0, i18n( "%1" ).arg( diff->sourceLineNumber() ) );
	setText( 1, i18n( "%1" ).arg( diff->destinationLineNumber() ) );

	QString text = QString::null;
	switch( diff->type() ) {
	case Diff2::Difference::Change:
		// Shouldn't this simply be diff->sourceLineCount() ?
		// because you change the _number of lines_ lines in source, not in dest
		text = i18n( "Changed %n line", "Changed %n lines",
		             QMAX( diff->sourceLineCount(),
		             diff->destinationLineCount() ) );
		break;
	case Diff2::Difference::Insert:
		text = i18n( "Inserted %n line", "Inserted %n lines",
		             diff->destinationLineCount() );
		break;
	case Diff2::Difference::Delete:
		text = i18n( "Deleted %n line", "Deleted %n lines",
		             diff->sourceLineCount() );
		break;
	default:
		kdDebug(8105) << "Unknown or Unchanged enum value when checking for diff->type() in KChangeLVI's constructor" << endl;
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

KFileLVI::KFileLVI( KListView* parent, Diff2::DiffModel* model ) : KListViewItem( parent )
{
	m_model = model;

	setText( 0, model->sourceFile() );
	setText( 1, model->destinationFile() );
	setPixmap( 0, SmallIcon( "txt" ) );
	setPixmap( 1, SmallIcon( "txt" ) );
	setSelectable( true );
}

void KFileLVI::fillChangesList( KListView* changesList, QPtrDict<KChangeLVI>* diffToChangeItemDict )
{
	changesList->clear();

	QPtrListIterator<Diff2::Difference> it( m_model->differences() );
	Diff2::Difference* diff;
	while ( ( diff = it.current() ) != 0L )
	{
		KChangeLVI* change = new KChangeLVI( changesList, diff );
		diffToChangeItemDict->insert( diff, change );
		++it;
	}

	changesList->setSelected( changesList->firstChild(), true );
}

KFileLVI::~KFileLVI()
{
}

KDirLVI::KDirLVI( KListView* parent, QString& dir ) : KListViewItem( parent )
{
//	kdDebug(8105) << "KDirLVI (KListView) constructor called with dir = " << dir << endl;
	m_rootItem = true;
	m_dirName = dir;
	setPixmap( 0, SmallIcon( "folder" ) );
	setOpen( true );
	setSelectable( true );
	if ( m_dirName.isEmpty() )
		setText( 0, i18n( "Unknown" ) );
	else
		setText( 0, m_dirName );
}

KDirLVI::KDirLVI( KDirLVI* parent, QString& dir ) : KListViewItem( parent )
{
//	kdDebug(8105) << "KDirLVI (KDirLVI) constructor called with dir = " << dir << endl;
	m_rootItem = false;
	m_dirName = dir;
	setPixmap( 0, SmallIcon( "folder" ) );
	setOpen( true );
	setSelectable( true );
	setText( 0, m_dirName );
}

// addModel always removes it own path from the beginning
void KDirLVI::addModel( QString& path, Diff2::DiffModel* model, QPtrDict<KDirLVI>* modelToDirItemDict )
{
//	kdDebug(8105) << "KDirLVI::addModel called with path = " << path << " from KDirLVI with m_dirName = " << m_dirName << endl;

	if ( !m_dirName.isEmpty() )
		path = path.remove( 0, m_dirName.length() );

//	kdDebug(8105) << "Path after removal of own dir (\"" << m_dirName << "\") = " << path << endl;

	if ( path.isEmpty() ) {
		m_modelList.append( model );
		modelToDirItemDict->insert( model, this );
		return;
	}

	KDirLVI* child;

	QString dir = path.mid( 0, path.find( "/", 0 ) + 1 );
	child = findChild( dir );
	if ( !child )
	{
		// does not exist yet so make it
//		kdDebug(8105) << "KDirLVI::addModel creating new KDirLVI because not found" << endl;
		child = new KDirLVI( this, dir );
	}

	child->addModel( path, model, modelToDirItemDict );
}

KDirLVI* KDirLVI::findChild( QString dir )
{
//	kdDebug(8105) << "KDirLVI::findChild called with dir = " << dir << endl;
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

void KDirLVI::fillFileList( KListView* fileList, QPtrDict<KFileLVI>* modelToFileItemDict )
{
	fileList->clear();

	QPtrListIterator<Diff2::DiffModel> it( m_modelList );
	Diff2::DiffModel* model;
	while ( ( model = it.current() ) != 0L )
	{
		KFileLVI* file = new KFileLVI( fileList, model );
		modelToFileItemDict->insert( model, file );
		++it;
	}

	fileList->setSelected( fileList->firstChild(), true );
}

QString KDirLVI::fullPath( QString& path )
{
//	if ( !path.isEmpty() )
//		kdDebug(8105) << "KDirLVI::fullPath called with path = " << path << endl;
//	else
//		kdDebug(8105) << "KDirLVI::fullPath called with empty path..." << endl;

	if ( m_rootItem ) // dont bother adding rootItem's dir...
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
//	kdDebug(8105) << "KDirLVI::setSelected called with dir = " << dir << endl;

	// root item's dirName is never taken into account... remember that
	if ( !m_rootItem )
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
		if ( dir.startsWith( child->dirName() ) )
			return child->setSelected( dir );
	} while ( ( child = static_cast<KDirLVI*>(child->nextSibling()) ) != 0L );

	return 0L;
}

KDirLVI::~KDirLVI()
{
}

// part stuff
KInstance*  KompareNavTreePartFactory::s_instance = 0L;
KAboutData* KompareNavTreePartFactory::s_about = 0L;

KompareNavTreePartFactory::KompareNavTreePartFactory()
    : KParts::Factory()
{
}

KompareNavTreePartFactory::~KompareNavTreePartFactory()
{
	delete s_instance;
	delete s_about;

	s_instance = 0L;
}

KParts::Part* KompareNavTreePartFactory::createPartObject( QWidget* parentWidget, const char* widgetName,
                                                  QObject* /*parent*/, const char* /*name*/,
                                                  const char* /*classname*/, const QStringList & /*args*/ )
{
	// Create an instance of our Part
	KompareNavTreePart* obj = new KompareNavTreePart( parentWidget, widgetName );

	KGlobal::locale()->insertCatalogue("kompare");

	return obj;
}

KInstance* KompareNavTreePartFactory::instance()
{
	if( !s_instance )
	{
		s_about = new KAboutData("komparenavtreepart", I18N_NOOP("KompareNavTreePart"), "1.1");
		s_about->addAuthor("John Firebaugh", "Author", "jfirebaugh@kde.org");
		s_about->addAuthor("Otto Bruggeman", "Author", "otto.bruggeman@home.nl" );
		s_instance = new KInstance(s_about);
	}
	return s_instance;
}

extern "C"
{
	void* init_libkomparenavtreepart()
	{
		return new KompareNavTreePartFactory;
	}
}

#include "komparenavtreepart.moc"
