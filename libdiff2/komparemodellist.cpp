/***************************************************************************
                          komparemodellist.cpp  -  description
                             -------------------
    begin                : Tue Jun 26 2001
    copyright            : (C) 2001-2004 Otto Bruggeman
                           (C) 2001-2003 John Firebaugh
    email                : jfirebaugh@kde.org
                           otto.bruggeman@home.nl
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qfile.h>
#include <qdir.h>
#include <qregexp.h>

#include <kaction.h>
#include <kdebug.h>
#include <kdirwatch.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmimemagic.h>
#include <ktempfile.h>

#include "difference.h"
#include "diffhunk.h"
#include "diffmodel.h"
#include "kompareprocess.h"
#include "komparemodellist.h"
#include "parser.h"

#include "kompare_part.h"

using namespace Diff2;

KompareModelList::KompareModelList( DiffSettings* diffSettings, struct Kompare::Info* info, QObject* parent, const char* name )
	: QObject( parent, name ),
	m_diffProcess( 0 ),
	m_diffSettings( diffSettings ),
	m_models( 0 ),
	m_selectedModel( 0 ),
	m_selectedDifference( 0 ),
	m_noOfModified( 0 ),
	m_modelIndex( 0 ),
	m_info( info )
{
	m_applyDifference    = new KAction( i18n("&Apply Difference"), "1rightarrow", Qt::Key_Space,
	                                 this, SLOT(slotActionApplyDifference()),
	                                 (( KomparePart* )parent)->actionCollection(), "difference_apply" );
	m_unApplyDifference  = new KAction( i18n("Un&apply Difference"), "1leftarrow", Qt::Key_BackSpace,
	                                 this, SLOT(slotActionUnApplyDifference()),
	                                 (( KomparePart* )parent)->actionCollection(), "difference_unapply" );
	m_applyAll           = new KAction( i18n("App&ly All"), "2rightarrow", Qt::CTRL + Qt::Key_A,
	                                 this, SLOT(slotActionApplyAllDifferences()),
	                                 (( KomparePart* )parent)->actionCollection(), "difference_applyall" );
	m_unapplyAll         = new KAction( i18n("&Unapply All"), "2leftarrow", Qt::CTRL + Qt::Key_U,
	                                 this, SLOT(slotActionUnapplyAllDifferences()),
	                                 (( KomparePart* )parent)->actionCollection(), "difference_unapplyall" );
	m_previousFile       = new KAction( i18n("P&revious File"), "2uparrow", Qt::CTRL + Qt::Key_PageUp,
	                                 this, SLOT(slotPreviousModel()),
	                                 (( KomparePart* )parent)->actionCollection(), "difference_previousfile" );
	m_nextFile           = new KAction( i18n("N&ext File"), "2downarrow", Qt::CTRL + Qt::Key_PageDown,
	                                 this, SLOT(slotNextModel()),
	                                 (( KomparePart* )parent)->actionCollection(), "difference_nextfile" );
	m_previousDifference = new KAction( i18n("&Previous Difference"), "1uparrow", Qt::CTRL + Qt::Key_Up,
	                                 this, SLOT(slotPreviousDifference()),
	                                 (( KomparePart* )parent)->actionCollection(), "difference_previous" );
	m_previousDifference->setEnabled( false );
	m_nextDifference  = new KAction( i18n("&Next Difference"), "1downarrow", Qt::CTRL + Qt::Key_Down,
	                                 this, SLOT(slotNextDifference()),
	                                 (( KomparePart* )parent)->actionCollection(), "difference_next" );
	m_nextDifference->setEnabled( false );

	updateModelListActions();
}

KompareModelList::~KompareModelList()
{
}

bool KompareModelList::isDirectory( const QString& url )
{
	QFileInfo fi( url );
	if ( fi.isDir() )
		return true;
	else
		return false;
}

bool KompareModelList::isDiff( const QString& mimeType )
{
	if ( mimeType == "text/x-diff" )
		return true;
	else
		return false;
}

bool KompareModelList::compare( const QString& source, const QString& destination )
{
	bool result = false;

	if ( isDirectory( source ) && isDirectory( destination ) )
	{
		m_info->mode = Kompare::ComparingDirs;
		result = compareDirs( source, destination );
	}
	else if ( !isDirectory( source ) && !isDirectory( destination ) )
	{
		QString sourceMimeType = ( KMimeMagic::self()->findFileType( source ) )->mimeType();
		QString destinationMimeType = ( KMimeMagic::self()->findFileType( destination ) )->mimeType();

		// Not checking if it is a text file/something diff can even compare, we'll let diff handle that
		if ( !isDiff( sourceMimeType ) && isDiff( destinationMimeType ) )
		{
			m_info->mode = Kompare::BlendingFile;
			result = openFileAndDiff( source, destination );
		}
		else if ( isDiff( sourceMimeType ) && !isDiff( destinationMimeType ) )
		{
			m_info->mode = Kompare::BlendingFile;
			result = openFileAndDiff( destination, source );
		}
		else
		{
			m_info->mode = Kompare::ComparingFiles;
			result = compareFiles( source, destination );
		}
	}
	else if ( isDirectory( source ) && !isDirectory( destination ) )
	{
		m_info->mode = Kompare::BlendingDir;
		result = openDirAndDiff( source, destination );
	}
	else
	{
		m_info->mode = Kompare::BlendingDir;
		result = openDirAndDiff( destination, source );
	}

	return result;
}

bool KompareModelList::compareFiles( const QString& source, const QString& destination )
{
	m_source = source;
	m_destination = destination;

	clear(); // Destroy the old models...

	m_fileWatch = new KDirWatch( this, "filewatch" );
	m_fileWatch->addFile( m_source );
	m_fileWatch->addFile( m_destination );

	connect( m_fileWatch, SIGNAL( dirty( const QString& ) ), this, SLOT( slotFileChanged( const QString& ) ) );
	connect( m_fileWatch, SIGNAL( created( const QString& ) ), this, SLOT( slotFileChanged( const QString& ) ) );
	connect( m_fileWatch, SIGNAL( deleted( const QString& ) ), this, SLOT( slotFileChanged( const QString& ) ) );

	m_fileWatch->startScan();
	m_diffProcess = new KompareProcess( m_diffSettings, Kompare::Custom, m_source, m_destination );

	connect( m_diffProcess, SIGNAL(diffHasFinished( bool )),
	         this, SLOT(slotDiffProcessFinished( bool )) );

	emit status( Kompare::RunningDiff );
	m_diffProcess->start();

	return true;
}

bool KompareModelList::compareDirs( const QString& source, const QString& destination )
{
	m_source = source;
	m_destination = destination;

	clear(); // Destroy the old models...

	m_dirWatch = new KDirWatch( this, "dirwatch" );
	// Watch files in the dirs and watch the dirs recursively
	m_dirWatch->addDir( m_source, true, true );
	m_dirWatch->addDir( m_destination, true, true );

	connect( m_dirWatch, SIGNAL( dirty  ( const QString& ) ), this, SLOT( slotDirectoryChanged( const QString& ) ) );
	connect( m_dirWatch, SIGNAL( created( const QString& ) ), this, SLOT( slotDirectoryChanged( const QString& ) ) );
	connect( m_dirWatch, SIGNAL( deleted( const QString& ) ), this, SLOT( slotDirectoryChanged( const QString& ) ) );

	m_dirWatch->startScan();
	m_diffProcess = new KompareProcess( m_diffSettings, Kompare::Custom, m_source, m_destination );

	connect( m_diffProcess, SIGNAL(diffHasFinished( bool )),
	         this, SLOT(slotDiffProcessFinished( bool )) );

	emit status( Kompare::RunningDiff );
	m_diffProcess->start();

	return true;
}

bool KompareModelList::openFileAndDiff( const QString& file, const QString& diff )
{
	clear();

	if ( parseDiffOutput( readFile( diff ) ) != 0 )
	{
		emit error( i18n( "No models or no differences, this file: %1, is not a valid diff file" ).arg(  diff ) );
		return false;
	}

	// Do our thing :)
	if ( !blendOriginalIntoModelList( file ) )
	{
		kdDebug(8101) << "Oops cant blend original file into modellist : " << file << endl;
		emit( i18n( "There were problems applying the diff (%1) to the file (%2)." ).arg( diff ).arg( file ) );
		return false;
	}

	updateModelListActions();
	show();

	return true;
}

bool KompareModelList::openDirAndDiff( const QString& dir, const QString& diff )
{
	clear();

	if ( parseDiffOutput( readFile( diff ) ) != 0 )
	{
		emit error( i18n( "No models or no differences, this file: %1, is not a valid diff file" ).arg( diff ) );
		return false;
	}

	// Do our thing :)
	if ( !blendOriginalIntoModelList( dir ) )
	{
		// Trouble blending the original into the model
		kdDebug(8101) << "Oops cant blend original dir into modellist : " << dir << endl;
		emit error( i18n( "There were problems applying the diff (%1) to the folder (%2)." ).arg( diff ).arg( dir ) );
		return false;
	}

	updateModelListActions();
	show();

	return true;
}

bool KompareModelList::saveDestination( const DiffModel* model_ )
{
	kdDebug() << "KompareModelList::saveDestination: " << endl;

	DiffModel* model = const_cast<DiffModel*>( model_ );
	if( !model->isModified() ) return true;

	KTempFile* temp = new KTempFile();

	if( temp->status() != 0 ) {
		emit error( i18n( "Could not open a temporary file." ) );
		temp->unlink();
		delete temp;
		return false;
	}

	QTextStream* stream = temp->textStream();
	QStringList list;

	QPtrListIterator<DiffHunk> hunkIt( model->hunks() );
	for( ; hunkIt.current(); ++hunkIt ) {
		DiffHunk* hunk = hunkIt.current();

		QPtrListIterator<Difference> diffIt(hunk->differences());

		Difference* diff;
		for( ; (diff = diffIt.current()); ++diffIt )
		{
			if( !diff->applied() )
			{
				DifferenceStringConstIterator it = diff->destinationLines().begin();
				DifferenceStringConstIterator end = diff->destinationLines().end();
				for ( ; it != end; ++it )
				{
					list.append( ( *it ).string() );
				}
			}
			else
			{
				QValueListConstIterator<DifferenceString> it = diff->sourceLines().begin();
				QValueListConstIterator<DifferenceString> end = diff->sourceLines().end();
				for ( ; it != end; ++it )
				{
					list.append( ( *it ).string() );
				}
			}
		}
	}

	kdDebug( 8101 ) << "Everything: " << endl << list.join( "\n" ) << endl;

	if( list.count() > 0 )
		*stream << list.join( "\n" ) << "\n";

	temp->close();
	if( temp->status() != 0 ) {
		emit error( i18n( "Could not write to the temporary file." ) );
		temp->unlink();
		delete temp;
		return false;
	}

	bool result = false;

	if ( m_info->mode == Kompare::ComparingDirs )
	{
		QString destination = model->destinationPath() + model->destinationFile();
		kdDebug(8101) << "Tempfilename   : " << temp->name() << endl;
		kdDebug(8101) << "DestinationURL : " << destination << endl;
		result = KIO::NetAccess::upload( temp->name(), destination, (QWidget*)parent() );
	}
	else
	{
		kdDebug(8101) << "Tempfilename   : " << temp->name() << endl;
		kdDebug(8101) << "DestinationURL : " << m_destination << endl;
		result = KIO::NetAccess::upload( temp->name(), m_destination, (QWidget*)parent() );
	}

	if ( !result )
	{
		emit error( i18n( "Could not upload the temporary file to the destination location %2. The temporary file is still available under: %1. You can manually copy it to the right place." ).arg( temp->name() ).arg( m_destination ) );
	}
	else
	{
		model->slotSetModified( false );
		temp->unlink();
		delete temp;
	}

	return true;
}

bool KompareModelList::saveAll()
{
	if ( !m_models )
		return false;

	QPtrListIterator<DiffModel> it( *m_models );
	while ( it.current() )
	{
		if( !saveDestination( *it ) ) return false;
		++it;
	}
	return true;
}

void KompareModelList::slotDiffProcessFinished( bool success )
{
	if ( success )
	{
		emit status( Kompare::Parsing );
		if ( parseDiffOutput( m_diffProcess->diffOutput() ) != 0 )
		{
			emit error( i18n( "Could not parse diff output." ) );
		}
		else
		{
			if ( m_info->mode != Kompare::ShowingDiff )
				blendOriginalIntoModelList( m_info->localSource );
			updateModelListActions();
			show();
		}
		emit status( Kompare::FinishedParsing );
	}
	else if ( m_diffProcess->exitStatus() == 0 )
	{
		emit error( i18n( "The files are identical." ) );
	}
	else
	{
		emit error( m_diffProcess->stdErr() );
	}

	delete m_diffProcess;
	m_diffProcess = 0;
}

void KompareModelList::slotDirectoryChanged( const QString& /*dir*/ )
{
	// some debug output to see if watching works properly
	kdDebug(8101) << "Yippie directories are being watched !!! :)" << endl;
	if ( m_diffProcess )
	{
		emit status( Kompare::ReRunningDiff );
		m_diffProcess->start();
	}
}

void KompareModelList::slotFileChanged( const QString& /*file*/ )
{
	// some debug output to see if watching works properly
	kdDebug(8101) << "Yippie files are being watched !!! :)" << endl;
	if ( m_diffProcess )
	{
		emit status( Kompare::ReRunningDiff );
		m_diffProcess->start();
	}
}

QStringList KompareModelList::readFile( const QString& fileName )
{
	QFile file( fileName );
	file.open( IO_ReadOnly );

	QTextStream stream( &file );
	QStringList contents;

	while (!stream.eof()) {
		contents.append( stream.readLine() );
	}

	return contents;
}

bool KompareModelList::openDiff( const QString& diffFile )
{
	kdDebug(8101) << "Stupid :) Url = " << diffFile << endl;

	if ( diffFile.isEmpty() )
		return false;

	QStringList diff = readFile( diffFile );

	clear(); // Clear the current models

	emit status( Kompare::Parsing );

	if ( parseDiffOutput( diff ) != 0 )
	{
		emit error( i18n( "Could not parse diff output." ) );
		return false;
	}

	updateModelListActions();
	show();

	emit status( Kompare::FinishedParsing );

	return true;
}

bool KompareModelList::saveDiff( const QString& url, QString directory, DiffSettings* diffSettings )
{
	kdDebug() << "KompareModelList::saveDiff: " << endl;

	m_diffTemp = new KTempFile();
	m_diffURL = url;

	if( m_diffTemp->status() != 0 ) {
		emit error( i18n( "Could not open a temporary file." ) );
		m_diffTemp->unlink();
		delete m_diffTemp;
		m_diffTemp = 0;
		return false;
	}

	m_diffProcess = new KompareProcess( diffSettings, Kompare::Custom, m_source, m_destination, directory );
	connect( m_diffProcess, SIGNAL(diffHasFinished( bool )),
	         this, SLOT(slotWriteDiffOutput( bool )) );

	emit status( Kompare::RunningDiff );
	return m_diffProcess->start();

}

void KompareModelList::slotWriteDiffOutput( bool success )
{
	kdDebug(8101) << "Success = " << success << endl;

	if( success )
	{
		QTextStream* stream = m_diffTemp->textStream();

		QStringList output = m_diffProcess->diffOutput();
		for ( QStringList::ConstIterator it = output.begin(); it != output.end(); ++it )
		{
			*stream << (*it) << "\n";
		}

		m_diffTemp->close();
		if( m_diffTemp->status() != 0 )
		{
			emit error( i18n( "Could not write to the temporary file." ) );
		}

		KIO::NetAccess::upload( m_diffTemp->name(), m_diffURL, (QWidget*)parent() );

		emit status( Kompare::FinishedWritingDiff );
	}

	m_diffURL.truncate( 0 );
	m_diffTemp->unlink();

	delete m_diffTemp;
	m_diffTemp = 0;

	delete m_diffProcess;
	m_diffProcess = 0;
}

void KompareModelList::slotSelectionChanged( const Diff2::DiffModel* model, const Diff2::Difference* diff )
{
// This method will signal all the other objects about a change in selection,
// it will emit setSelection( const DiffModel*, const Difference* ) to all who are connected
	kdDebug(8101) << "KompareModelList::slotSelectionChanged( " << model << ", " << diff << " )" << endl;
	kdDebug(8101) << "Sender is : " << sender()->className() << endl;
//	kdDebug(8101) << kdBacktrace() << endl;

	m_selectedModel = const_cast<DiffModel*>(model);
	m_modelIndex = m_models->findRef( model );
	kdDebug( 8101 ) << "m_modelIndex = " << m_modelIndex << endl;
	m_selectedDifference = const_cast<Difference*>(diff);

	m_selectedModel->setSelectedDifference( m_selectedDifference );

	// setSelected* search for the argument in the lists and return false if not found
	// if found they return true and set the m_selected*
	if ( !setSelectedModel( m_selectedModel ) )
	{
		// Backup plan
		m_selectedModel = firstModel();
		m_selectedDifference = m_selectedModel->firstDifference();
	}
	else if ( !m_selectedModel->setSelectedDifference( m_selectedDifference ) )
	{
		// Another backup plan
		m_selectedDifference = m_selectedModel->firstDifference();
	}

	emit setSelection( model, diff );
	emit setStatusBarModelInfo( findModel( m_selectedModel ), m_selectedModel->findDifference( m_selectedDifference ), modelCount(), differenceCount(), m_selectedModel->appliedCount() );

	updateModelListActions();
}

void KompareModelList::slotSelectionChanged( const Diff2::Difference* diff )
{
// This method will emit setSelection( const Difference* ) to whomever is listening
// when for instance in kompareview the selection has changed
	kdDebug(8101) << "KompareModelList::slotSelectionChanged( " << diff << " )" << endl;
	kdDebug(8101) << "Sender is : " << sender()->className() << endl;

	m_selectedDifference = const_cast<Difference*>(diff);

	if ( !m_selectedModel->setSelectedDifference( m_selectedDifference ) )
	{
		// Backup plan
		m_selectedDifference = m_selectedModel->firstDifference();
	}

	emit setSelection( diff );
	updateModelListActions();
}

void KompareModelList::slotPreviousModel()
{
	if ( ( m_selectedModel = prevModel() ) != 0 )
	{
		m_selectedDifference = m_selectedModel->firstDifference();
	}
	else
	{
		m_selectedModel = firstModel();
		m_selectedDifference = m_selectedModel->firstDifference();
	}

	emit setSelection( m_selectedModel, m_selectedDifference );
	emit setStatusBarModelInfo( findModel( m_selectedModel ), m_selectedModel->findDifference( m_selectedDifference ), modelCount(), differenceCount(), m_selectedModel->appliedCount() );
	updateModelListActions();
}

void KompareModelList::slotNextModel()
{
	if ( ( m_selectedModel = nextModel() ) != 0 )
	{
		m_selectedDifference = m_selectedModel->firstDifference();
	}
	else
	{
		m_selectedModel = lastModel();
		m_selectedDifference = m_selectedModel->firstDifference();
	}

	emit setSelection( m_selectedModel, m_selectedDifference );
	emit setStatusBarModelInfo( findModel( m_selectedModel ), m_selectedModel->findDifference( m_selectedDifference ), modelCount(), differenceCount(), m_selectedModel->appliedCount() );
	updateModelListActions();
}

DiffModel* KompareModelList::firstModel()
{
	kdDebug( 8101 ) << "KompareModelList::firstModel()" << endl;
	m_modelIndex = 0;
	kdDebug( 8101 ) << "m_modelIndex = " << m_modelIndex << endl;

	m_selectedModel = m_models->at( m_modelIndex );

	return m_selectedModel;
}

DiffModel* KompareModelList::lastModel()
{
	kdDebug( 8101 ) << "KompareModelList::lastModel()" << endl;
	m_modelIndex = m_models->count() - 1;
	kdDebug( 8101 ) << "m_modelIndex = " << m_modelIndex << endl;

	m_selectedModel = m_models->at( m_modelIndex );

	return m_selectedModel;
}

DiffModel* KompareModelList::prevModel()
{
	kdDebug( 8101 ) << "KompareModelList::prevModel()" << endl;
	if ( --m_modelIndex < m_models->count() )
	{
		kdDebug( 8101 ) << "m_modelIndex = " << m_modelIndex << endl;
		m_selectedModel = m_models->at( m_modelIndex );
	}
	else
	{
		m_selectedModel = 0;
		m_modelIndex = 0;
		kdDebug( 8101 ) << "m_modelIndex = " << m_modelIndex << endl;
	}

	return m_selectedModel;
}

DiffModel* KompareModelList::nextModel()
{
	kdDebug( 8101 ) << "KompareModelList::nextModel()" << endl;
	if ( ++m_modelIndex < m_models->count() )
	{
		kdDebug( 8101 ) << "m_modelIndex = " << m_modelIndex << endl;
		m_selectedModel = m_models->at( m_modelIndex );
	}
	else
	{
		m_selectedModel = 0;
		m_modelIndex = 0;
		kdDebug( 8101 ) << "m_modelIndex = " << m_modelIndex << endl;
	}

	return m_selectedModel;
}

void KompareModelList::slotPreviousDifference()
{
	kdDebug(8101) << "slotPreviousDifference called" << endl;
	if ( ( m_selectedDifference = m_selectedModel->prevDifference() ) != 0 )
	{
		emit setSelection( m_selectedDifference );
		emit setStatusBarModelInfo( findModel( m_selectedModel ), m_selectedModel->findDifference( m_selectedDifference ), modelCount(), differenceCount(), m_selectedModel->appliedCount() );
		updateModelListActions();
		return;
	}

	kdDebug(8101) << "Fuck no previous difference... ok lets find the previous model..." << endl;

	if ( ( m_selectedModel = prevModel() ) != 0 )
	{
		m_selectedDifference = m_selectedModel->lastDifference();

		emit setSelection( m_selectedModel, m_selectedDifference );
		emit setStatusBarModelInfo( findModel( m_selectedModel ), m_selectedModel->findDifference( m_selectedDifference ), modelCount(), differenceCount(), m_selectedModel->appliedCount() );
		updateModelListActions();
		return;
	}


	kdDebug(8101) << "Crap !!! No previous model, ok backup plan activated..." << endl;

	// Backup plan
	m_selectedModel = firstModel();
	m_selectedDifference = m_selectedModel->firstDifference();

	emit setSelection( m_selectedModel, m_selectedDifference );
	emit setStatusBarModelInfo( findModel( m_selectedModel ), m_selectedModel->findDifference( m_selectedDifference ), modelCount(), differenceCount(), m_selectedModel->appliedCount() );
	updateModelListActions();
}

void KompareModelList::slotNextDifference()
{
	kdDebug(8101) << "slotNextDifference called" << endl;
	if ( ( m_selectedDifference = m_selectedModel->nextDifference() ) != 0 )
	{
		emit setSelection( m_selectedDifference );
		emit setStatusBarModelInfo( findModel( m_selectedModel ), m_selectedModel->findDifference( m_selectedDifference ), modelCount(), differenceCount(), m_selectedModel->appliedCount() );
		updateModelListActions();
		return;
	}

	kdDebug(8101) << "Fuck no next difference... ok lets find the next model..." << endl;

	if ( ( m_selectedModel = nextModel() ) != 0 )
	{
		m_selectedDifference = m_selectedModel->firstDifference();

		emit setSelection( m_selectedModel, m_selectedDifference );
		emit setStatusBarModelInfo( findModel( m_selectedModel ), m_selectedModel->findDifference( m_selectedDifference ), modelCount(), differenceCount(), m_selectedModel->appliedCount() );
		updateModelListActions();
		return;
	}

	kdDebug(8101) << "Crap !!! No next model, ok backup plan activated..." << endl;

	// Backup plan
	m_selectedModel = lastModel();
	m_selectedDifference = m_selectedModel->lastDifference();

	emit setSelection( m_selectedModel, m_selectedDifference );
	emit setStatusBarModelInfo( findModel( m_selectedModel ), m_selectedModel->findDifference( m_selectedDifference ), modelCount(), differenceCount(), m_selectedModel->appliedCount() );
	updateModelListActions();
}

void KompareModelList::slotApplyDifference( bool apply )
{
	m_selectedModel->applyDifference( apply );
	emit applyDifference( apply );
	emit setModified( m_selectedModel->isModified() );
}

void KompareModelList::slotApplyAllDifferences( bool apply )
{
	// FIXME: we need to use hunks here as well
	m_selectedModel->applyAllDifferences( apply );
	emit applyAllDifferences( apply );
	emit setModified( apply );
}

int KompareModelList::parseDiffOutput( const QStringList& lines )
{
	kdDebug(8101) << "KompareModelList::parseDiffOutput" << endl;
	Parser* parser = new Parser();
	m_models = parser->parse( lines );

	m_info->generator = parser->generator();
	m_info->format = parser->format();

	if ( m_models )
	{
		m_selectedModel = firstModel();
		kdDebug(8101) << "Ok there are differences..." << endl;
		m_selectedDifference = m_selectedModel->firstDifference();
		emit setStatusBarModelInfo( 0, 0, modelCount(), differenceCount(), 0);
	}
	else
	{
		// Wow trouble, no models, so no differences...
		kdDebug(8101) << "Now i'll be damned, there should be models here !!!" << endl;
		return -1;
	}

	return 0;
}

bool KompareModelList::blendOriginalIntoModelList( const QString& localURL )
{
	QFileInfo fi( localURL );

	bool result = false;
	DiffModel* model;
	QFile file;

	DiffModelList* models = m_models;
	m_models = new DiffModelList();
	if ( fi.isDir() )
	{ // is a dir
		QDir dir( localURL, QString::null, QDir::Name|QDir::DirsFirst, QDir::All );
		QPtrListIterator<DiffModel> it( *models );
		for ( ; it.current() != 0; ++it )
		{
			model = (*it);
			kdDebug(8101) << "Model : " << model << endl;
			QString filename = model->sourcePath() + model->sourceFile();
			if ( !filename.startsWith( localURL ) )
				filename.prepend( localURL );
			QFileInfo fi2( filename );
			QStringList lines;
			if ( fi2.exists() )
			{
				kdDebug(8101) << "Reading from: " << filename << endl;
				file.setName( filename );
				file.open( IO_ReadOnly );
				QTextStream stream( &file );
				while ( !stream.eof() )
				{
					lines.append( stream.readLine() );
				}
				file.close();
				result = blendFile( model, lines );
			}
			else
			{
				kdDebug(8101) << "File " << filename << " does not exist !" << endl;
				kdDebug(8101) << "Assume empty file !" << endl;
				result = blendFile( model, lines );
			}
		}
	}
	else if ( fi.isFile() )
	{ // is a file
		QFile file( localURL );
		file.open( IO_ReadOnly );
		QTextStream stream( &file );
		QStringList lines;
		while ( !stream.eof() )
		{
			lines.append( stream.readLine() );
		}

		result = blendFile( models->at( 0 ), lines );
	}

	delete models; // Hope it does not delete the models, just the container

	return result;
}

bool KompareModelList::blendFile( DiffModel* model, const QStringList& lines )
{
	if ( !model )
	{
		kdDebug() << "Crap model is empty :(" << endl;
		return false;
	}

	Difference* newDiff;
	DiffModel* newModel = new DiffModel();
	// We don't want a full copy so dont use newModel = new DiffModel( model );
	*newModel = *model;

	int srcLineNo = 1, destLineNo = 1;

	DiffHunk* newHunk   = new DiffHunk( srcLineNo, destLineNo );

	newModel->addHunk( newHunk );

	QStringList::ConstIterator it  = lines.begin();
	QStringList::ConstIterator end = lines.end();

	QPtrList<Difference> diffList = model->allDifferences();
	Difference* diff;

	diffList.first();

	while ( ( diff = diffList.current() ) )
	{
		// Check if there are lines in the original file before the difference
		// that are not yet in the diff. If so create new Context diff
		if ( srcLineNo < diff->sourceLineNumber() )
		{
			newDiff = new Difference( srcLineNo, destLineNo );
			newHunk->add( newDiff );
			while ( srcLineNo < diff->sourceLineNumber() && it != end )
			{
				kdDebug(8101) << "SourceLine = " << srcLineNo << ": " << *it << endl;
				newDiff->addSourceLine( *it );
				newDiff->addDestinationLine( *it );
				srcLineNo++;
				destLineNo++;
				++it;
			}
		}
		// Now i've got to add that diff
		bool conflict = false;
		switch ( diff->type() )
		{
		case Difference::Unchanged:
			kdDebug() << "Unchanged" << endl;
			for ( int i = 0; i < diff->sourceLineCount(); i++ )
			{
				if ( it != end && *it != diff->sourceLineAt( i )->string() )
				{
					kdDebug(8101) << "Conflict: SourceLine = " << srcLineNo << ": " << *it << endl;
					kdDebug(8101) << "Conflict: DiffLine   = " << diff->sourceLineNumber() + i << ": " << diff->sourceLineAt( i )->string() << endl;
					conflict = true;
					break;
				}
//				kdDebug(8101) << "SourceLine = " << srcLineNo << ": " << *it << endl;
//				kdDebug(8101) << "DiffLine   = " << diff->sourceLineNumber() + i << ": " << diff->sourceLineAt( i )->string() << endl;
				srcLineNo++;
				destLineNo++;
				++it;
			}

			if ( !conflict )
			{
				diffList.take();
				newHunk->add( diff );
			}
			else
			{
				// FIXME: Leave that diff out for now
				// We'll have to do all sorts of nice things to get this working properly
				// Like trying offsets and or fuzzy matching or check for tabs that were converted into spaces by diff
				kdDebug(8101) << "Unchanged: Wow, conflict... what should we do now ???" << endl;
				diffList.next();
			}
			break;
		case Difference::Change:
			kdDebug() << "Change" << endl;
			for ( int i = 0; i < diff->sourceLineCount(); i++ )
			{
				if ( it != end && *it != diff->sourceLineAt( i )->string() )
				{
					conflict = true;
					break;
				}
				srcLineNo++;
				destLineNo++;
				++it;
			}

			destLineNo += diff->destinationLineCount();

			if ( !conflict )
			{
				diffList.take();
				newHunk->add( diff );
				newModel->addDiff( diff );
			}
			else
			{
				// FIXME: Leave that diff out for now
				// We'll have to do all sorts of nice things to get this working properly
				// Like trying offsets and or fuzzy matching
				kdDebug(8101) << "Change: Wow, conflict... what should we do now ???" << endl;
				diffList.next();
			}
			break;
		case Difference::Insert:
			kdDebug() << "Insert" << endl;
			destLineNo += diff->destinationLineCount();
			diffList.take();
			newHunk->add( diff );
			newModel->addDiff( diff );
			break;
		case Difference::Delete:
			kdDebug() << "Delete" << endl;
			for ( int i = 0; i < diff->sourceLineCount(); i++ )
			{
				if ( it != end && *it != diff->sourceLineAt( i )->string() )
				{
					conflict = true;
					break;
				}
				srcLineNo++;
				++it;
			}

			if ( !conflict )
			{
				diffList.take();
				newHunk->add( diff );
				newModel->addDiff( diff );
			}
			else
			{
				// FIXME: Leave that diff out for now
				// We'll have to do all sorts of nice things to get this working properly
				// Like trying offsets and or fuzzy matching
				kdDebug(8101) << "Delete: Wow, conflict... what should we do now ???" << endl;
				diffList.next();
			}
			break;
		}
	}

	if ( it != end )
	{
		newDiff = new Difference( srcLineNo, destLineNo, Difference::Unchanged );
		newHunk->add( newDiff );

		while ( it != end )
		{
			newDiff->addSourceLine( *it );
			newDiff->addDestinationLine( *it );
			++it;
		}
	}
/*
	diffList = newModel->differences();

	diff = diffList.first();
	kdDebug(8101) << "Count = " << diffList.count() << endl;
	for ( diff = diffList.first(); diff; diff = diffList.next() )
	{
		kdDebug(8101) << "sourcelinenumber = " << diff->sourceLineNumber() << endl;
	}
*/

	m_models->remove( model );
	delete model;
	m_models->inSort( newModel );

	m_selectedModel = firstModel();

	m_selectedDifference = m_selectedModel->firstDifference();

	return true;
}

void KompareModelList::show()
{
	emit modelsChanged( m_models );
	emit setSelection( m_selectedModel, m_selectedDifference );
}

void KompareModelList::clear()
{
	if ( m_models )
		m_models->clear();

	emit modelsChanged( m_models );
}

void KompareModelList::swap()
{
	QString source = m_source;
	QString destination = m_destination;
	if ( m_info->mode == Kompare::ComparingFiles )
		compareFiles( destination, source );
	else if ( m_info->mode == Kompare::ComparingDirs )
		compareDirs( destination, source );
}

bool KompareModelList::isModified() const
{
	if ( m_noOfModified > 0 )
		return true;
	return false;
}

int KompareModelList::modelCount() const
{
	return m_models ? m_models->count() : 0;
}

int KompareModelList::differenceCount() const
{
	return m_selectedModel ? m_selectedModel->differenceCount() : -1;
}

int KompareModelList::appliedCount() const
{
	return m_selectedModel ? m_selectedModel->appliedCount() : -1;
}

void KompareModelList::slotSetModified( bool modified )
{
	kdDebug(8101) << "m_noOfModified = " << m_noOfModified << endl;

	if ( modified )
		m_noOfModified++;
	else
		m_noOfModified--;

	if ( m_noOfModified < 0 )
	{
		kdDebug(8101) << "Wow something is fucked up..." << endl;
	}
	else if ( m_noOfModified == 0 )
	{
		emit setModified( false );
	}
	else // > 0 :-)
	{
		emit setModified( true );
	}
}

bool KompareModelList::setSelectedModel( DiffModel* model )
{
	kdDebug(8101) << "KompareModelList::setSelectedModel( " << model << " )" << endl;

	if ( model != m_selectedModel )
	{
		if ( m_models->findRef( model ) == -1 )
			return false;
		kdDebug(8101) << "m_selectedModel (was) = " << m_selectedModel << endl;
		m_modelIndex = m_models->findRef( model );
		kdDebug(8101) << "m_selectedModel (is)  = " << m_selectedModel << endl;
		m_selectedModel = model;
	}

	updateModelListActions();

	return true;
}

void KompareModelList::updateModelListActions()
{
	if ( m_models && m_selectedModel && m_selectedDifference )
	{
		if ( ( ( KomparePart* )parent() )->isReadWrite() )
		{
			if ( m_selectedModel->appliedCount() != m_selectedModel->differenceCount() )
				m_applyAll->setEnabled( true );
			else
				m_applyAll->setEnabled( false );

			if ( m_selectedModel->appliedCount() != 0 )
				m_unapplyAll->setEnabled( true );
			else
				m_unapplyAll->setEnabled( false );

			m_applyDifference->setEnabled( true );
			m_unApplyDifference->setEnabled( true );
		}
		else
		{
			m_applyDifference->setEnabled( false );
			m_unApplyDifference->setEnabled( false );
			m_applyAll->setEnabled( false );
			m_unapplyAll->setEnabled( false );
		}

		m_previousFile->setEnabled      ( hasPrevModel() );
		m_nextFile->setEnabled          ( hasNextModel() );
		m_previousDifference->setEnabled( hasPrevDiff() );
		m_nextDifference->setEnabled    ( hasNextDiff() );
	}
	else
	{
		m_applyDifference->setEnabled( false );
		m_unApplyDifference->setEnabled( false );
		m_applyAll->setEnabled( false );
		m_unapplyAll->setEnabled( false );

		m_previousFile->setEnabled      ( false );
		m_nextFile->setEnabled          ( false );
		m_previousDifference->setEnabled( false );
		m_nextDifference->setEnabled    ( false );
	}
}

bool KompareModelList::hasPrevModel()
{
	kdDebug(8101) << "KompareModelList::hasPrevModel()" << endl;

	if (  m_modelIndex > 0 )
	{
//		kdDebug(8101) << "has prev model" << endl;
		return true;
	}

//	kdDebug(8101) << "doesn't have a prev model, this is the first one..." << endl;

	return false;
}

bool KompareModelList::hasNextModel()
{
	kdDebug(8101) << "KompareModelList::hasNextModel()" << endl;

	if ( (  unsigned int )m_modelIndex < (  m_models->count() - 1 ) )
	{
//		kdDebug(8101) << "has next model" << endl;
		return true;
	}

//	kdDebug(8101) << "doesn't have a next model, this is the last one..." << endl;
	return false;
}

bool KompareModelList::hasPrevDiff()
{
//	kdDebug(8101) << "KompareModelList::hasPrevDiff()" << endl;
	int index = m_selectedModel->diffIndex();

	if ( index > 0 )
	{
//		kdDebug(8101) << "has prev difference in same model" << endl;
		return true;
	}

	if ( hasPrevModel() )
	{
//		kdDebug(8101) << "has prev difference but in prev model" << endl;
		return true;
	}

//	kdDebug(8101) << "doesn't have a prev difference, not even in the previous model because there is no previous model" << endl;

	return false;
}

bool KompareModelList::hasNextDiff()
{
//	kdDebug(8101) << "KompareModelList::hasNextDiff()" << endl;
	int index = m_selectedModel->diffIndex();

	if ( index < ( m_selectedModel->differenceCount() - 1 ) )
	{
//		kdDebug(8101) << "has next difference in same model" << endl;
		return true;
	}

	if ( hasNextModel() )
	{
//		kdDebug(8101) << "has next difference but in next model" << endl;
		return true;
	}

//	kdDebug(8101) << "doesn't have a next difference, not even in next model because there is no next model" << endl;

	return false;
}

void KompareModelList::slotActionApplyDifference()
{
	if ( !m_selectedDifference->applied() )
		slotApplyDifference( true );
	slotNextDifference();
	updateModelListActions();
}

void KompareModelList::slotActionUnApplyDifference()
{
	if ( m_selectedDifference->applied() )
		slotApplyDifference( false );
	slotPreviousDifference();
	updateModelListActions();
}

void KompareModelList::slotActionApplyAllDifferences()
{
	slotApplyAllDifferences( true );
	updateModelListActions();
}

void KompareModelList::slotActionUnapplyAllDifferences()
{
	slotApplyAllDifferences( false );
	updateModelListActions();
}

#include "komparemodellist.moc"

/* vim: set ts=4 sw=4 noet: */
