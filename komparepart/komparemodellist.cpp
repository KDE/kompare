/***************************************************************************
                          kdiffmodellist.cpp  -  description
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

#include <qfile.h>
#include <qtextstream.h>

#include <ktempfile.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kdebug.h>

#include "diffmodel.h"
#include "diffhunk.h"
#include "difference.h"
#include "kdiffprocess.h"

#include "kdiffmodellist.h"
#include "kdiffmodellist.moc"

KDiffModelList::KDiffModelList()
	: QObject(),
	m_mode( Compare ),
	m_diffProcess( 0 )
{
}

KDiffModelList::~KDiffModelList()
{
	delete m_diffProcess;
}

void KDiffModelList::compare( const KURL& source, const KURL& destination )
{
	m_mode = Compare;
	m_sourceURL = source;
	m_destinationURL = destination;
	
	m_diffProcess = new KDiffProcess( source, destination );
	connect( m_diffProcess, SIGNAL(diffHasFinished( bool )),
	         this, SLOT(slotDiffProcessFinished( bool )) );
	
	emit status( RunningDiff );
	m_diffProcess->start();
}

void KDiffModelList::slotDiffProcessFinished( bool success )
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

void KDiffModelList::readDiffFile( QFile& file )
{
	m_mode = Diff;
	
	QTextStream stream(&file);
	QStringList list;
	while (!stream.eof())
		list.append(stream.readLine());

	emit status( Parsing );
	if( parseDiffs( list ) != 0 ) {
		emit error( i18n( "Could not parse diff output." ) );
	}
	emit status( FinishedParsing );
}

void KDiffModelList::writeDiffFile( QString file, DiffSettings* settings )
{
	m_diffFile = file;
	
	m_diffProcess = new KDiffProcess( m_sourceURL, m_destinationURL, settings );
	connect( m_diffProcess, SIGNAL(diffHasFinished( bool )),
	         this, SLOT(slotWriteDiffOutput( bool )) );
	
	emit status( RunningDiff );
	m_diffProcess->start();
	
}

void KDiffModelList::slotWriteDiffOutput( bool success )
{
	if( success ) {
		QFile file( m_diffFile );
		if( file.open(IO_WriteOnly) ) {
			
			// use QTextStream to dump the text to the file
			QTextStream stream(&file);
			
			QStringList output = m_diffProcess->getDiffOutput();
			for ( QStringList::ConstIterator it = output.begin(); it != output.end(); ++it ) {
				stream << (*it) << "\n";
			}
			
			file.close();
			
			emit status( FinishedWritingDiff );
		}
	}
	
	delete m_diffProcess;
	m_diffProcess = 0;
}

void KDiffModelList::saveDestination( int index )
{
	DiffModel* model = modelAt( index );
	KTempFile* temp = new KTempFile();
	
	if( temp->status() != 0 ) {
		emit error( i18n( "Could not open file." ) );
		return;
	}
	
	QTextStream* stream = temp->textStream();
	
	QListIterator<DiffHunk> hunkIt( model->getHunks() );
	for( ; hunkIt.current(); ++hunkIt ) {
		DiffHunk* hunk = hunkIt.current();
		
		QListIterator<Difference> diffIt(hunk->getDifferences());
		
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
		return;
	}
	
	KIO::NetAccess::upload( temp->name(), m_destinationURL );
	
	temp->unlink();
	delete temp;
}

int KDiffModelList::parseDiffs( const QStringList& lines )
{
	if( lines.count() == 0 ) return 1;

	QStringList::ConstIterator it = lines.begin();
	while( it != lines.end() ) {

		// Discard leading garbage
		while( it != lines.end() && DiffModel::determineDiffFormat( (*it) ) == DiffModel::Unknown ) {
			kdDebug() << "discarding: " << (*it) << endl;
			++it;
		}

		DiffModel* model = new DiffModel();
		int result = model->parseDiff( DiffModel::determineDiffFormat( (*it) ), lines, it );

		kdDebug() << "parsed one file, result: " << result << endl;

		addModel( model );

	}

	return 0; // FIXME better error handling
};

void KDiffModelList::addModel( DiffModel* model )
{
	m_models.append( model );
	emit modelAdded( model );
}
