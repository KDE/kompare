/***************************************************************************
                          komparemodellist.cpp  -  description
                             -------------------
    begin                : Tue Jun 26 2001
    copyright            : (C) 2001-2003 by John Firebaugh
                           and Otto Bruggeman
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

using namespace Diff2;

KompareModelList::KompareModelList( DiffSettings* diffSettings, ViewSettings* viewSettings, QObject* parent, const char* name )
	: QObject( parent, name ),
	m_diffProcess( 0 ),
	m_diffSettings( diffSettings ),
	m_viewSettings( viewSettings ),
	m_models( 0 ),
	m_mode( Kompare::ShowingDiff ),
	m_selectedModel( 0 ),
	m_selectedDifference( 0 ),
	m_noOfModified( 0 )
{
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
		result = compareDirs( source, destination );
	}
	else if ( !isDirectory( source ) && !isDirectory( destination ) )
	{
		QString sourceMimeType = ( KMimeMagic::self()->findFileType( source ) )->mimeType();
		QString destinationMimeType = ( KMimeMagic::self()->findFileType( destination ) )->mimeType();

		// Not checking if it is a text file/something diff can even compare, we'll let diff handle that
		if ( !isDiff( sourceMimeType ) && isDiff( destinationMimeType ) )
		{
			result = openFileAndDiff( source, destination );
		}
		else if ( isDiff( sourceMimeType ) && !isDiff( destinationMimeType ) )
		{
			result = openFileAndDiff( destination, source );
		}
		else
		{
			result = compareFiles( source, destination );
		}
	}
	else if ( isDirectory( source ) && !isDirectory( destination ) )
	{
		result = openDirAndDiff( source, destination );
	}
	else
	{
		result = openDirAndDiff( destination, source );
	}

	return result;
}

bool KompareModelList::compareFiles( const QString& source, const QString& destination )
{
	m_source = source;
	m_destination = destination;

	clear(); // Destroy the old models...

	m_mode = Kompare::ComparingFiles;
	m_type = Kompare::SingleFileDiff;

	m_fileWatch = new KDirWatch( this, "filewatch" );
	m_fileWatch->addFile( m_source );
	m_fileWatch->addFile( m_destination );

	connect( m_fileWatch, SIGNAL( dirty( const QString& ) ), this, SLOT( slotFileChanged( const QString& ) ) );
	connect( m_fileWatch, SIGNAL( created( const QString& ) ), this, SLOT( slotFileChanged( const QString& ) ) );
	connect( m_fileWatch, SIGNAL( deleted( const QString& ) ), this, SLOT( slotFileChanged( const QString& ) ) );

	m_fileWatch->startScan();
	m_diffProcess = new KompareProcess( m_diffSettings, m_viewSettings, Kompare::Default, m_source, m_destination );

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

	m_mode = Kompare::ComparingDirs;
	m_type = Kompare::MultiFileDiff;

	m_dirWatch = new KDirWatch( this, "dirwatch" );
	// Watch files in the dirs and watch the dirs recursively
	m_dirWatch->addDir( m_source, true, true );
	m_dirWatch->addDir( m_destination, true, true );

	connect( m_dirWatch, SIGNAL( dirty  ( const QString& ) ), this, SLOT( slotDirectoryChanged( const QString& ) ) );
	connect( m_dirWatch, SIGNAL( created( const QString& ) ), this, SLOT( slotDirectoryChanged( const QString& ) ) );
	connect( m_dirWatch, SIGNAL( deleted( const QString& ) ), this, SLOT( slotDirectoryChanged( const QString& ) ) );

	m_dirWatch->startScan();
	m_diffProcess = new KompareProcess( m_diffSettings, m_viewSettings, Kompare::Default, m_source, m_destination );

	connect( m_diffProcess, SIGNAL(diffHasFinished( bool )),
	         this, SLOT(slotDiffProcessFinished( bool )) );

	emit status( Kompare::RunningDiff );
	m_diffProcess->start();

	return true;
}

bool KompareModelList::openFileAndDiff( const QString& file, const QString& diff )
{
	bool result = false;
	return result;
}

bool KompareModelList::openDirAndDiff( const QString& dir, const QString& diff )
{
	bool result = false;
	return result;
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

	QPtrListIterator<DiffHunk> hunkIt( model->hunks() );
	for( ; hunkIt.current(); ++hunkIt ) {
		DiffHunk* hunk = hunkIt.current();

		QPtrListIterator<Difference> diffIt(hunk->differences());

		Difference* diff;
		for( ; (diff = diffIt.current()); ++diffIt )
		{
			QStringList list;
			if( !diff->applied() )
				list = diff->destinationLines();
			else
				list = diff->sourceLines();

			if( list.count() > 0 )
				*stream << list.join( "\n" ) << "\n";
		}
	}

	temp->close();
	if( temp->status() != 0 ) {
		emit error( i18n( "Could not write to the temporary file." ) );
		temp->unlink();
		delete temp;
		return false;
	}

	QString destination = model->destinationPath() + model->destinationFile();
	kdDebug(8101) << "Tempfilename   : " << temp->name() << endl;
	kdDebug(8101) << "DestinationURL : " << destination << endl;

	bool result = false;

	if ( m_mode == Kompare::ComparingDirs )
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
	if ( success ) {
		emit status( Kompare::Parsing );
		if ( parseDiffOutput( m_diffProcess->diffOutput() ) != 0 ) {
			emit error( i18n( "Could not parse diff output." ) );
		}
		emit status( Kompare::FinishedParsing );
	} else if ( m_diffProcess->exitStatus() == 0 ) {
		emit error( i18n( "The files are identical." ) );
	} else {
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

QStringList& KompareModelList::readFile( const QString& fileName )
{
	QFile file( fileName );
	file.open( IO_ReadOnly );

	QTextStream stream( &file );
	QStringList* contents = new QStringList();

	while (!stream.eof()) {
		contents->append( stream.readLine() );
	}
	
	return *contents;
}

bool KompareModelList::openDiff( const QString& diffFile )
{
	kdDebug(8101) << "Stupid :) Url = " << diffFile << endl;

	if ( diffFile.isEmpty() )
		return false;

	QStringList& diff = readFile( diffFile );

	clear(); // Clear the current models

	emit status( Kompare::Parsing );

	if ( parseDiffOutput( diff ) != 0 )
	{
		emit error( i18n( "Could not parse diff output." ) );
		return false;
	}

	m_mode = Kompare::ShowingDiff;

	emit status( Kompare::FinishedParsing );

	return true;
}

bool KompareModelList::saveDiff( const QString& url, QString directory, DiffSettings* diffSettings, ViewSettings* viewSettings )
{
	kdDebug() << "KompareModelList::saveDiff: " << endl;

/*	m_diffTemp = new KTempFile();

	if( m_diffTemp->status() != 0 ) {
		emit error( i18n( "Could not open a temporary file." ) );
		m_diffTemp->unlink();
		delete m_diffTemp;
		m_diffTemp = 0;
		return false;
	}
*/
	m_diffProcess = new KompareProcess( diffSettings, viewSettings, Kompare::Custom, m_source, m_destination, directory );
	connect( m_diffProcess, SIGNAL(diffHasFinished( bool )),
	         this, SLOT(slotWriteDiffOutput( bool )) );

	emit status( Kompare::RunningDiff );
	return m_diffProcess->start();

}

void KompareModelList::slotWriteDiffOutput( bool success )
{
/*	if( success )
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
			emit error( i18n( "Could not write to file." ) );
		}

		KIO::NetAccess::upload( m_diffTemp->name(), m_diffURL, (QWidget*)parent() );

		emit status( Kompare::FinishedWritingDiff );
	}

	m_diffTemp->unlink();

	delete m_diffTemp;
	m_diffTemp = 0;

	delete m_diffProcess;
	m_diffProcess = 0;
*/}

void KompareModelList::slotSelectionChanged( const Diff2::DiffModel* model, const Diff2::Difference* diff )
{
// This method will signal all the other objects about a change in selection,
// it will emit setSelection( const DiffModel*, const Difference* ) to all who are connected
	kdDebug(8101) << "Caught me a model and a diff signal " << endl;
//	kdDebug(8101) << kdBacktrace() << endl;

	m_selectedModel = const_cast<DiffModel*>(model);
	m_selectedDifference = const_cast<Difference*>(diff);

	m_selectedModel->setSelectedDifference( m_selectedDifference );

	for( m_modelIt->toFirst(); !m_modelIt->atLast(); ++(*m_modelIt) )
	{
		if ( model == *(*m_modelIt) )
			break;
	}

	if ( m_modelIt->atLast() && model != *(*m_modelIt) )
		kdDebug(8101) << "Big fat trouble, no model found" << endl;

	delete m_diffIt;
	m_diffIt = new QPtrListIterator<Difference>( m_selectedModel->differences() );

	for( m_diffIt->toFirst(); !m_diffIt->atLast(); ++(*m_diffIt) )
	{
		if ( diff == *(*m_diffIt) )
			break;
	}

	if ( m_diffIt->atLast() && diff != *(*m_diffIt) )
		kdDebug(8101) << "Big fat trouble, no diff found" << endl;

	emit setSelection( model, diff );
}

void KompareModelList::slotSelectionChanged( const Diff2::Difference* diff )
{
// This method will emit setSelection( const Difference* ) to whomever is listening
// when for instance in kompareview the selection has changed
	kdDebug(8101) << "Caught me a signal, yihaa" << endl;

	m_selectedDifference = const_cast<Difference*>(diff);
	m_selectedModel->setSelectedDifference( m_selectedDifference );

	for( m_diffIt->toFirst(); !m_diffIt->atLast(); ++(*m_diffIt) )
	{
		if ( diff == *(*m_diffIt) )
			break;
	}

	if ( m_diffIt->atLast() && diff != *(*m_diffIt) )
		kdDebug(8101) << "Big fat trouble, no diff found" << endl;

	emit setSelection( diff );
}

void KompareModelList::slotPreviousModel()
{
	// cannot get below 1
	if ( !m_modelIt->atFirst() )
	{
		m_selectedModel = --(*m_modelIt);

		delete m_diffIt;
		m_diffIt = new QPtrListIterator<Difference>( m_selectedModel->differences() );

		m_selectedDifference = m_diffIt->toFirst();
		m_selectedModel->setSelectedDifference( m_selectedDifference );

		emit setSelection( m_selectedModel, m_selectedDifference );
	}
}

void KompareModelList::slotNextModel()
{
	if ( !m_modelIt->atLast() )
	{
		m_selectedModel = ++(*m_modelIt);

		delete m_diffIt;
		m_diffIt = new QPtrListIterator<Difference>( m_selectedModel->differences() );

		m_selectedDifference = m_diffIt->toFirst();
		m_selectedModel->setSelectedDifference( m_selectedDifference );

		emit setSelection( m_selectedModel, m_selectedDifference );
	}
}

void KompareModelList::slotPreviousDifference()
{
	if ( m_selectedDifference->index() > 0 )
	{
		m_selectedDifference = --(*m_diffIt);
		m_selectedModel->setSelectedDifference( m_selectedDifference );

		emit setSelection( m_selectedDifference );
	}
	else if ( m_selectedDifference->index() == 0 && m_selectedModel->index() > 0 )
	{
		m_selectedModel = --(*m_modelIt);

		delete m_diffIt;
		m_diffIt = new QPtrListIterator<Difference>( m_selectedModel->differences() );

		m_selectedDifference = m_diffIt->toLast();
		m_selectedModel->setSelectedDifference( m_selectedDifference );

		emit setSelection( m_selectedModel, m_selectedDifference );
	}
	// no previous difference (should not happen)
}

void KompareModelList::slotNextDifference()
{
	if ( m_selectedDifference->index() < ( m_selectedModel->differenceCount() - 1 ) )
	{
		m_selectedDifference = ++(*m_diffIt);
		m_selectedModel->setSelectedDifference( m_selectedDifference );

		emit setSelection( m_selectedDifference );
	}
	else if ( ( m_selectedDifference->index() == ( m_selectedModel->differenceCount() - 1 ) ) &&
			  ( m_selectedModel->index() < ( int( m_models->count() ) - 1 ) ) )
	{
		m_selectedModel = ++(*m_modelIt);

		delete m_diffIt;
		m_diffIt = new QPtrListIterator<Difference>( m_selectedModel->differences() );

		m_selectedDifference = m_diffIt->toFirst();
		m_selectedModel->setSelectedDifference( m_selectedDifference );

		emit setSelection( m_selectedModel, m_selectedDifference );
	}
	// no next difference (should not happen)
}

void KompareModelList::slotApplyDifference( bool apply )
{
	m_selectedModel->applyDifference( apply );
	emit applyDifference( apply );
}

void KompareModelList::slotApplyAllDifferences( bool apply )
{
	// FIXME: we need to use hunks here as well
	m_selectedModel->applyAllDifferences( apply );
	emit applyAllDifferences( apply );
}

int KompareModelList::parseDiffOutput( const QStringList& lines )
{
	kdDebug(8101) << "KompareModelList::parseDiffOutput" << endl;
	Parser* parser = new Parser();
	m_models = parser->parse( lines );

	if ( m_models )
	{
		delete m_modelIt;
		m_modelIt = new QPtrListIterator<DiffModel>( *m_models );
		m_selectedModel = m_modelIt->toFirst();
		kdDebug(8101) << "Ok there are differences..." << endl;
		delete m_diffIt;
		m_diffIt = new QPtrListIterator<Difference>( m_selectedModel->differences() );
		m_selectedDifference = m_diffIt->toFirst();
	}
	else
	{
		// Wow trouble, no models, so no differences...
		kdDebug(8101) << "Now i'll be damned, there should be models here !!!" << endl;
		return -1;
	}

	show();

	return 0;
}

bool KompareModelList::blendOriginalIntoModelList( const QString& localURL )
{
	QFileInfo fi( localURL );
	if ( !fi.exists() )
	{
		kdDebug(8101) << "KML::blendOriginalIntoModelList : localURL \"" << localURL << "\" does not exist !" << endl;
		return false;
	}

	bool result = false;
	DiffModel* model;
	QFile file;

	QPtrList<DiffModel>* models = m_models;
	m_models = new QPtrList<DiffModel>();
	if ( fi.isDir() )
	{ // is a dir
		QDir dir( localURL, QString::null, QDir::Name|QDir::DirsFirst, QDir::All );
		QPtrListIterator<DiffModel> it( *models );
		for ( ; it.current() != 0; ++it )
		{
			model = (*it);
			kdDebug(8101) << "Model : " << model << endl;
			QString filename = model->sourcePath() + model->sourceFile();
			QFileInfo fi2( localURL + filename );
			if ( fi2.exists() )
			{
				file.setName( localURL + filename );
				file.open( IO_ReadOnly );
				QTextStream stream( &file );
				QStringList lines;
				while ( !stream.eof() )
				{
					lines.append( stream.readLine() );
				}
				file.close();
				result = blendFile( model, lines );
			}
			else
			{
				kdDebug(8101) << "File " << localURL + filename << " does not exist !" << endl;
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

		result = blendFile( modelAt( 0 ), lines );
	}

	delete models; // Hope it does not delete the models, just the container

	return result;

}

bool KompareModelList::blendFile( DiffModel* model, const QStringList& lines )
{
	if ( !model )
		return false;

	Difference* newDiff;
	DiffModel* newModel = new DiffModel();
	// This is not a full copy so dont use newModel = new DiffModel( model );
	*newModel = *model;

	DiffHunk* newHunk   = new DiffHunk( 1,1 );

	newModel->addHunk( newHunk );

	int srcLineNo = 1, destLineNo = 1;

	QStringList::ConstIterator it = lines.begin();

	QPtrList<Difference> diffList = model->allDifferences();
	Difference* diff;

	for ( diff = diffList.first(); diff; diff = diffList.next() )
	{
		newDiff = new Difference( srcLineNo, destLineNo );
		newHunk->add( newDiff );
		kdDebug(8101) << "SrcLineNo " << srcLineNo << " should be smaller than " << diff->sourceLineNumber() << endl;
		while ( srcLineNo < diff->sourceLineNumber() && it != lines.end() )
		{
			newDiff->addSourceLine( *it );
			newDiff->addDestinationLine( *it );
			kdDebug(8101) << "Line " << srcLineNo << " : " << *it << endl;
			srcLineNo++;
			destLineNo++;
			++it;
		}
		// Now i've got to add the difference (and check if the difference
		// is appliable)
		switch ( diff->type() )
		{
		case Difference::Unchanged:
			newDiff = new Difference( srcLineNo, destLineNo, Difference::Unchanged );
			kdDebug(8101) << "newDiff = new Difference( " << srcLineNo << ", " << destLineNo << ", Difference::Unchanged );" << endl;
			newHunk->add( newDiff );
			for ( int i = 0; i < diff->sourceLineCount(); i++ )
			{
				kdDebug(8101) << "Line " << srcLineNo << " : " << *it << endl;
				kdDebug(8101) << "Contxt: Fileline:     " << *it << endl;
				kdDebug(8101) << "Contxt: ContextLine : " << diff->sourceLineAt( i ) << endl;
				if ( it != lines.end() && *it == diff->sourceLineAt( i ) )
				{
					// Context matches, so we are on the right track
					newDiff->addSourceLine( *it );
					newDiff->addDestinationLine( *it );
					kdDebug(8101) << "Line " << srcLineNo << " : " << *it << endl;
					++it;
					srcLineNo++;
					destLineNo++;
				}
			}
			break;
		case Difference::Change:
			newDiff = new Difference( srcLineNo, destLineNo, Difference::Change );
			newHunk->add( newDiff );
			newModel->addDiff( newDiff );
			for ( int i = 0; i < diff->sourceLineCount(); i++ )
			{
				kdDebug(8101) << "Change: Fileline   : " << *it << endl;
				kdDebug(8101) << "Change: SourceLine : " << diff->sourceLineAt( i ) << endl;
				if ( it != lines.end() && *it == diff->sourceLineAt( i ) )
				{
					newDiff->addSourceLine( *it );
//					kdDebug(8101) << "Line " << srcLineNo << " : " << *it << endl;
					srcLineNo++;
				}
				else
				{
					kdDebug(8101) << "Oops sourceLine does not match deleted line" << endl;
				}
				++it;
			}
			for ( int i = 0; i < diff->destinationLineCount(); i++ )
			{
				newDiff->addDestinationLine( diff->destinationLineAt( i ) );
				kdDebug(8101) << "DestLine " << destLineNo << " : " << diff->destinationLineAt( i ) << endl;
				destLineNo++;
			}
			break;
		case Difference::Insert:
			kdDebug(8101) << "Are we even getting in here ?" << endl;
			newDiff = new Difference( srcLineNo, destLineNo, Difference::Insert );
			newHunk->add( newDiff );
			newModel->addDiff( newDiff );
			for ( int i = 0; it != lines.end() && i < diff->destinationLineCount(); i++ )
			{
				newDiff->addDestinationLine( diff->destinationLineAt( i ) );
				kdDebug(8101) << "DestLine " << destLineNo << " : " << diff->destinationLineAt( i ) << endl;
				destLineNo++;
			}
			break;
		case Difference::Delete:
			newDiff = new Difference( srcLineNo, destLineNo, Difference::Delete );
			newHunk->add( newDiff );
			newModel->addDiff( newDiff );
			for ( int i = 0; it != lines.end() && i < diff->sourceLineCount(); i++ )
			{
				kdDebug(8101) << "Delete: Fileline:    " << *it << endl;
				kdDebug(8101) << "Delete: SourceLine : " << diff->sourceLineAt( i ) << endl;
				if ( *it == diff->sourceLineAt( i ) )
				{
					newDiff->addSourceLine( *it );
					kdDebug(8101) << "Line " << srcLineNo << " : " << *it << endl;
					srcLineNo++;
				}
				else
				{
					// deleted sourcelines dont match with fileline
					kdDebug(8101) << "Deleted line does not match sourceline" << endl;
				}
			}
			break;
		}
	}

	newDiff = new Difference( srcLineNo, destLineNo, Difference::Unchanged );
	newHunk->add( newDiff );

	while ( it != lines.end() )
	{
		newDiff->addSourceLine( *it );
		newDiff->addDestinationLine( *it );
		kdDebug(8101) << "Contxt: Fileline:     " << *it << endl;
		++it;
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
	m_models->append( newModel );

	delete m_modelIt;
	m_modelIt = new QPtrListIterator<DiffModel>( *m_models );
	m_selectedModel = m_modelIt->toFirst();

	if ( m_selectedModel->differences().count() != 0 )
	{
		kdDebug(8101) << "Ok there are differences : " << m_selectedModel->differences().count() << endl;
		delete m_diffIt;
		m_diffIt = new QPtrListIterator<Difference>( m_selectedModel->differences() );
		m_selectedDifference = m_diffIt->toFirst();
	}
	else
	{
		// Wow trouble, no differences in the model, abort
		kdDebug(8101) << "Damn there are no differences but there are models... WTF ???" << endl;
		return -1;
	}

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
	if ( m_mode == Kompare::ComparingFiles )
		compareFiles( m_destination, m_source );
	else if ( m_mode == Kompare::ComparingDirs )
		compareDirs( m_destination,  m_source );
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

#include "komparemodellist.moc"

/* vim: set ts=4 sw=4 noet: */
