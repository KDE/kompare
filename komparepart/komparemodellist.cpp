/***************************************************************************
                          komparemodellist.cpp  -  description
                             -------------------
    begin                : Tue Jun 26 2001
    copyright            : (C) 2001 by John Firebaugh
    email                : jfirebaugh@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <kdebug.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <ktempfile.h>

#include "difference.h"
#include "diffhunk.h"
#include "diffmodel.h"
#include "kompareprocess.h"

#include "komparemodellist.h"

KompareModelList::KompareModelList()
	: QObject(),
	m_mode( Compare ),
	m_diffProcess( 0 ),
	m_diffTemp( 0 )
{
}

KompareModelList::~KompareModelList()
{
	KIO::NetAccess::removeTempFile( m_sourceTemp );
	KIO::NetAccess::removeTempFile( m_destinationTemp );
	
	delete m_diffProcess;
}

bool KompareModelList::compare( const KURL& source, const KURL& destination )
{
	m_mode = Compare;
	
	m_sourceURL = source;
	m_destinationURL = destination;
	
	clear();
	
	if( !KIO::NetAccess::download( m_sourceURL, m_sourceTemp ) ) {
		emit error( KIO::NetAccess::lastErrorString() );
		return false;
	}
	
	if( !KIO::NetAccess::download( m_destinationURL, m_destinationTemp ) ) {
		emit error( KIO::NetAccess::lastErrorString() );
		return false;
	}
	
	m_diffProcess = new KompareProcess( m_sourceTemp, m_destinationTemp );
	connect( m_diffProcess, SIGNAL(diffHasFinished( bool )),
	         this, SLOT(slotDiffProcessFinished( bool )) );
	
	emit status( RunningDiff );
	m_diffProcess->start();
	
	return true;
}

bool KompareModelList::saveDestination( int index )
{
	DiffModel* model = modelAt( index );
	
	if( !model->isModified() ) return true;
	
	KTempFile* temp = new KTempFile();
	
	if( temp->status() != 0 ) {
		emit error( i18n( "Could not open file." ) );
		temp->unlink();
		delete temp;
		return false;
	}
	
	QTextStream* stream = temp->textStream();
	
	QPtrListIterator<DiffHunk> hunkIt( model->getHunks() );
	for( ; hunkIt.current(); ++hunkIt ) {
		DiffHunk* hunk = hunkIt.current();
		
		QPtrListIterator<Difference> diffIt(hunk->getDifferences());
		
		for( ; diffIt.current(); ++diffIt ) {
			Difference* diff = diffIt.current();
			
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
		emit error( i18n( "Could not write to file." ) );
		temp->unlink();
		delete temp;
		return false;
	}
	
	KIO::NetAccess::upload( temp->name(), m_destinationURL );
	
	model->setModified( false );
	
	temp->unlink();
	delete temp;
	
	return true;
}

bool KompareModelList::saveAll()
{
	for( uint i = 0; i < m_models.count(); ++i ) {
		if( !saveDestination( i ) ) return false;
	}
	return true;
}

void KompareModelList::slotDiffProcessFinished( bool success )
{
	if( success ) {
		emit status( Parsing );
		if( parseDiffs( m_diffProcess->getDiffOutput() ) != 0 ) {
			emit error( i18n( "Could not parse diff output." ) );
		}
		emit status( FinishedParsing );
	} else if( m_diffProcess->exitStatus() == 0 ) {
		emit error( i18n( "The files are identical." ) );
	} else {
		emit error( m_diffProcess->getStderr() );
	}
	
	delete m_diffProcess;
	m_diffProcess = 0;
}

bool KompareModelList::openDiff( const KURL& url )
{
	m_mode = Diff;
	m_diffURL = url;
	
	clear();
	
	QString diffTemp;
	
	if( !KIO::NetAccess::download( m_diffURL, diffTemp ) ) {
		return false;
	}
	
	QFile file( diffTemp );
	
	if( !file.open( IO_ReadOnly ) ) {
		KIO::NetAccess::removeTempFile( diffTemp );
		return false;
	}
	
	QTextStream stream( &file );
	QStringList list;
	while (!stream.eof()) {
		list.append(stream.readLine());
	}
	
	emit status( Parsing );
	if( parseDiffs( list ) != 0 ) {
		emit error( i18n( "Could not parse diff output." ) );
		KIO::NetAccess::removeTempFile( diffTemp );
		return false;
	}
	
	KIO::NetAccess::removeTempFile( diffTemp );
	emit status( FinishedParsing );
	return true;
}

bool KompareModelList::saveDiff( const KURL& url, QString directory, DiffSettings* settings )
{
	m_diffURL = url;
	
	m_diffTemp = new KTempFile();
	
	if( m_diffTemp->status() != 0 ) {
		emit error( i18n( "Could not open file." ) );
		m_diffTemp->unlink();
		delete m_diffTemp;
		m_diffTemp = 0;
		return false;
	}
	
	m_diffProcess = new KompareProcess( m_sourceTemp, m_destinationTemp, directory, settings );
	connect( m_diffProcess, SIGNAL(diffHasFinished( bool )),
	         this, SLOT(slotWriteDiffOutput( bool )) );
	
	emit status( RunningDiff );
	return m_diffProcess->start();
	
}

void KompareModelList::slotWriteDiffOutput( bool success )
{
	if( success ) {
		
		QTextStream* stream = m_diffTemp->textStream();
		
		QStringList output = m_diffProcess->getDiffOutput();
		for ( QStringList::ConstIterator it = output.begin(); it != output.end(); ++it ) {
			*stream << (*it) << "\n";
		}
		
		m_diffTemp->close();
		if( m_diffTemp->status() != 0 ) {
			emit error( i18n( "Could not write to file." ) );
		}
		
		KIO::NetAccess::upload( m_diffTemp->name(), m_diffURL );
		
		emit status( FinishedWritingDiff );
	}
	
	m_diffTemp->unlink();
	delete m_diffTemp;
	m_diffTemp = 0;
	delete m_diffProcess;
	m_diffProcess = 0;
}

int KompareModelList::parseDiffs( const QStringList& lines )
{
	if( lines.count() == 0 ) return 1;

	QStringList::ConstIterator it = lines.begin();
	while( it != lines.end() ) {

		// Discard leading garbage
		while( it != lines.end() && DiffModel::determineDiffFormat( (*it) ) == Unknown ) {
			kdDebug() << "discarding: " << (*it) << endl;
			++it;
		}

		DiffModel* model = new DiffModel();
		int result = model->parseDiff( DiffModel::determineDiffFormat( (*it) ), lines, it );

		kdDebug() << "parsed one file, result: " << result << endl;

		m_models.append( model );

	}
	emit modelsChanged();
	return 0; // FIXME better error handling
};

void KompareModelList::clear()
{
	m_models.clear();
	emit modelsChanged();
}

void KompareModelList::swap()
{
	compare( KURL( m_destinationURL ), KURL( m_sourceURL ) );
}

bool KompareModelList::isModified() const
{
	QPtrListIterator<DiffModel> it( m_models );
	for( ; it.current(); ++it ) {
		if( (*it)->isModified() ) return true;
	}
	return false;
}

KURL KompareModelList::sourceBaseURL() const
{
//	if( m_sourceURL.isDirectory() ) {
//		return m_sourceURL;
//	} else {
		return m_sourceURL.upURL();
//	}
}

KURL KompareModelList::destinationBaseURL() const
{
//	if( m_destinationURL.isDirectory() ) {
//		return m_destinationURL;
//	} else {
		return m_destinationURL.upURL();
//	}
}

#include "komparemodellist.moc"
