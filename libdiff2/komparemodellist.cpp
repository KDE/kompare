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

#include <qfile.h>
#include <qregexp.h>

#include <kdebug.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <ktempfile.h>

#include "difference.h"
#include "diffhunk.h"
#include "diffmodel.h"
#include "kompareprocess.h"

#include "komparemodellist.h"

#define kdDebug() kdDebug(8101)

KompareModelList::KompareModelList()
	: QObject(),
	m_diffProcess( 0 ),
	m_diffTemp( 0 ),
	m_mode( Compare ),
	m_selectedModel( 0 ),
	m_selectedDifference( 0 ),
	m_modelIt( 0 ),
	m_diffIt( 0 )
{
}

KompareModelList::~KompareModelList()
{
	KIO::NetAccess::removeTempFile( m_sourceTemp );
	KIO::NetAccess::removeTempFile( m_destinationTemp );
	
	delete m_modelIt;

	delete m_diffProcess;
}

bool KompareModelList::compare( const KURL& source, const KURL& destination )
{
	m_sourceURL = source;
	m_destinationURL = destination;

	bool sourceIsDirectory = false;
	bool destinationIsDirectory = false;

	clear();

	// determine if files are dirs...
	if ( m_sourceURL.directory(false,false) == m_sourceURL.url() )
		sourceIsDirectory = true;
	if ( m_destinationURL.directory(false,false) == m_destinationURL.url() )
		destinationIsDirectory = true;

	if ( sourceIsDirectory && destinationIsDirectory )
	{
		// dealing with dirs... if dirs are local proceed else abort
		if ( m_sourceURL.protocol() == "file" && m_destinationURL.protocol() == "file" )
		{
			// proceed
			m_mode = Compare;
			m_type = MultiFileDiff;

			m_diffProcess = new KompareProcess( m_sourceURL.path(), m_destinationURL.path() );
		}
		else
		{
			// abort, we cant handle remote dirs (yet)
			emit error( i18n("Kompare can't handle remote directories (yet)") );
			return false;
		}
	}
	else if ( !sourceIsDirectory && !destinationIsDirectory )
	{
		// dealing with files... proceed even if both are remote
		m_mode = Compare;
		m_type = SingleFileDiff;

		if( m_type == SingleFileDiff && !KIO::NetAccess::download( m_sourceURL, m_sourceTemp ) ) {
			emit error( KIO::NetAccess::lastErrorString() );
			return false;
		}

		if( m_type == SingleFileDiff && !KIO::NetAccess::download( m_destinationURL, m_destinationTemp ) ) {
			emit error( KIO::NetAccess::lastErrorString() );
			return false;
		}

		m_diffProcess = new KompareProcess( m_sourceTemp, m_destinationTemp );
	}
	else
	{
		// one of them is a directory and the other a file... cant
		// compare such a monster
		// FIXME
		// Wishlist item from puetzk: in this case (file and dir)
		// (make a temp dir, apply the patch (the file) and compare the dir with the temp dir)
		emit error( i18n("You can not compare a directory with a file") );
		return false;
	}

	connect( m_diffProcess, SIGNAL(diffHasFinished( bool )),
	         this, SLOT(slotDiffProcessFinished( bool )) );

	emit status( RunningDiff );
	m_diffProcess->start();

	return true;
}

bool KompareModelList::saveDestination( const DiffModel* model_ )
{
	DiffModel* model = const_cast<DiffModel*>( model_ );
	if( !model->isModified() ) return true;

	KTempFile* temp = new KTempFile();

	if( temp->status() != 0 ) {
		emit error( i18n( "Could not open file." ) );
		temp->unlink();
		delete temp;
		return false;
	}

	QTextStream* stream = temp->textStream();

	QPtrListIterator<DiffHunk> hunkIt( model->hunks() );
	for( ; hunkIt.current(); ++hunkIt ) {
		DiffHunk* hunk = hunkIt.current();

		QPtrListIterator<Difference> diffIt(hunk->differences());

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
	QPtrListIterator<DiffModel> it( m_models );
	while ( it.current() )
	{
		if( !saveDestination( *it ) ) return false;
		++it;
	}
	return true;
}

void KompareModelList::slotDiffProcessFinished( bool success )
{
	if( success ) {
		emit status( Parsing );
		if( parseDiffOutput( m_diffProcess->diffOutput() ) != 0 ) {
			emit error( i18n( "Could not parse diff output." ) );
		}
		emit status( FinishedParsing );
	} else if( m_diffProcess->exitStatus() == 0 ) {
		emit error( i18n( "The files are identical." ) );
	} else {
		emit error( m_diffProcess->stdErr() );
	}

	delete m_diffProcess;
	m_diffProcess = 0;
}

bool KompareModelList::openDiff( const KURL& url )
{
	// determine whether it is a multifile or singlefile diff and whether cvs diff or diff is used
	m_diffURL = url;
	bool isLocal = false;

	kdDebug() << "URL.fileName() = " << url.fileName( false ) << endl;

	if ( url.protocol() == "file" ) {
		isLocal = true;
	}

	QString diffTemp;

	if( !isLocal && !KIO::NetAccess::download( m_diffURL, diffTemp ) ) {
		return false;
	}

	QFile file;

	if( isLocal ) {
		if ( url.fileName( false ) == "-" ) {
			kdDebug() << "Using stdin to read the diff" << endl;
			file.open( IO_ReadOnly, stdin );
		}
		else
		{
			kdDebug() << "Reading from file " << url.url() << endl;
			file.setName( url.path() );
			file.open( IO_ReadOnly );
		}
	}
	else
	{
		file.setName( diffTemp );
		file.open( IO_ReadOnly );
	}

	if( !isLocal && !file.open( IO_ReadOnly ) ) {
		KIO::NetAccess::removeTempFile( diffTemp );
		return false;
	}

	QTextStream stream( &file );
	QStringList list;
	QString line;
	while (!stream.eof()) {
		list.append( stream.readLine() );
	}

	emit status( Parsing );
	if ( parseDiffOutput( list ) != 0 ) {
		emit error( i18n( "Could not parse diff output." ) );
		if ( !isLocal ) {
			KIO::NetAccess::removeTempFile( diffTemp );
		}
		return false;
	}

	if ( !isLocal ) {
		KIO::NetAccess::removeTempFile( diffTemp );
	}

	m_mode = Diff;
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

		QStringList output = m_diffProcess->diffOutput();
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

void KompareModelList::slotSelectionChanged( const DiffModel* model, const Difference* diff )
{
// This method will signal all the other objects about a change in selection,
// it will emit setSelection( const DiffModel*, const Difference* ) to all who are connected
	kdDebug() << "Caught me a model and a diff signal " << endl;

	m_selectedModel = const_cast<DiffModel*>(model);
	m_selectedDifference = const_cast<Difference*>(diff);

	m_selectedModel->setSelectedDifference( m_selectedDifference );

	for( m_modelIt->toFirst(); !m_modelIt->atLast(); ++(*m_modelIt) )
	{
		if ( model == *(*m_modelIt) )
			break;
	}

	if ( m_modelIt->atLast() && model != *(*m_modelIt) )
		kdDebug() << "Big fat trouble, no model found" << endl;

	delete m_diffIt;
	m_diffIt = new QPtrListIterator<Difference>( m_selectedModel->differences() );

	for( m_diffIt->toFirst(); !m_diffIt->atLast(); ++(*m_diffIt) )
	{
		if ( diff == *(*m_diffIt) )
			break;
	}

	if ( m_diffIt->atLast() && diff != *(*m_diffIt) )
		kdDebug() << "Big fat trouble, no diff found" << endl;

	emit setSelection( model, diff );
}

void KompareModelList::slotSelectionChanged( const Difference* diff )
{
// This method will emit setSelection( const Difference* ) to whomever is listening
// when for instance in kompareview the selection has changed
	kdDebug() << "Caught me a signal, yihaa" << endl;

	m_selectedDifference = const_cast<Difference*>(diff);
	m_selectedModel->setSelectedDifference( m_selectedDifference );

	for( m_diffIt->toFirst(); !m_diffIt->atLast(); ++(*m_diffIt) )
	{
		if ( diff == *(*m_diffIt) )
			break;
	}

	if ( m_diffIt->atLast() && diff != *(*m_diffIt) )
		kdDebug() << "Big fat trouble, no diff found" << endl;

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
	else if ( ( m_selectedDifference->index() == ( m_selectedModel->differenceCount() - 1 ) ) && ( m_selectedModel->index() < ( m_models.count() - 1 ) ) )
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

int KompareModelList::determineDiffFormat( const QStringList& lines )
{
	QStringList::ConstIterator it = lines.begin();

	while( it != lines.end() )
	{
		if( (*it).find( QRegExp( "^[0-9]+[0-9,]*[acd][0-9]+[0-9,]*$" ), 0 ) == 0 ) {
			m_format = Normal;
			return 0;
		}
		// this one works for cvs diff generated unified diffs
		else if( (*it).find( QRegExp( "^--- [^\\t]+\\t" ), 0 ) == 0 ) {
			m_format = Unified;
			return 0;
		}
		// this one should work for diff generated unified diffs ->
		// trouble is context diff has a second line with '^--- ' as well...
		else if( (*it).find( QRegExp( "^--- " ), 0 ) == 0 ) {
			m_format = Unified;
			return 0;
		}
		// This one works for cvs diff generated context diffs
		else if( (*it).find( QRegExp( "^\\*\\*\\* [^\\t]+\\t" ), 0 ) == 0 ) {
			m_format = Context;
			return 0;
		}
		// This one works for diff generated context diffs
		else if( (*it).find( QRegExp( "^\\+\\+\\+ " ), 0 ) == 0 ) {
			m_format = Context;
			return 0;
		}
		else if( (*it).find( QRegExp( "^[acd][0-9]+ [0-9]+" ), 0 ) == 0 ) {
			m_format = RCS;
			return 0;
		}
		else if( (*it).find( QRegExp( "^[0-9]+[0-9,]*[acd]" ), 0 ) == 0 ) {
			m_format = Ed;
			return 0;
		}
		++it;
	}
	return -1;
}

int KompareModelList::parseDiffOutput( const QStringList& lines )
{
	// FIXME: remove code duplication
	if( lines.count() == 0 ) return -1;

	bool cvsdiff = false;

	if ( determineDiffFormat( lines ) == -1 ) // format is stored in this class
		return -2;

	// determine if cvs diff or plain diff
	QStringList::ConstIterator it = lines.begin();
	while( it != lines.end() )
	{
		if ( (*it).startsWith( "Index: " ) )
		{
			cvsdiff = true;
			// needed so the splitter wont split the first
			// "diff " or "Index: " line into a diff
			kdDebug() << "File is a CVSDiff" << endl;
			++it;
			break;
		}
		else if ( (*it).startsWith( "diff " ) )
		{
			cvsdiff = false;
			// needed so the splitter wont split the first
			// "diff " or "Index: " line into a diff
			kdDebug() << "File is a regular Diff" << endl;
			++it;
			break;
		}
		++it;
	}

	QStringList* file = new QStringList();

	if ( it == lines.end() )
	{
		// no "diff " or "Index: " found :( ->
		// problem... either not a valid diff file
		// or some other weird shit, we'll let the
		// parser decide and leave the default
		// cvsdiff = false; (single file diff does
		// not have a "diff " line, so it could work)
		kdDebug() << "Assuming single file diff" << endl;
		it = lines.begin();
	}

	int modelIndex = 0;
	// FIXME: lots of duplicate code... needs to be refactored into a switch case
	if ( cvsdiff )
	{
		// cvs diff
		while( it != lines.end() )
		{
//			kdDebug() << (*it) << endl;
			// FIXME
			// not completely correct, diff and Index: have to be outside of
			// an ed/rcs hunk to be a valid new file, else it could be that
			// a line in the file starts with "diff " or "Index: "
			if ( (*it).startsWith( "Index: " ) )
			{
				DiffModel* model = new DiffModel();
				int result = model->parseDiff( m_format, *file );
				if ( result == 0 )
				{
					kdDebug() << "One file parsed" << endl;
					model->setIndex( modelIndex++ );
					m_models.append( model );
				}
				else
				{
					kdDebug() << "Trouble, file parsing failed with result = " << result << endl;
					kdDebug() << file->join( "\n" ) << endl;
					delete model;
				}
				file->clear();
			}
			else if ( (*it).startsWith( "Only in " ) )
			{
				// skip...
				kdDebug() << "Skipping : " << (*it) << endl;
			}
			else
			{
				file->append( *it );
			}
			++it;
		}

		// if file is not empty then there was another file, doing this seperately out of the loop
		if ( file->count() > 0 )
		{
			DiffModel* model = new DiffModel();
			int result = model->parseDiff( m_format, *file );
			if ( result == 0 )
			{
				kdDebug() << "One file parsed" << endl;
				model->setIndex( modelIndex++ );
				m_models.append( model );
			}
			else
			{
				kdDebug() << "Trouble, file parsing failed with result = " << result << endl;
				kdDebug() << file->join( "\n" ) << endl;
				delete model;
			}
			file->clear();
		}

		if ( m_models.count() > 1 ) {
			kdDebug() << "MultiFileCVSDiff" << endl;
			m_type = MultiFileCVSDiff;
		}
		else {
			kdDebug() << "SingleFileCVSDiff" << endl;
			m_type = SingleFileCVSDiff;
		}
	} // end if cvsdiff
	else
	{
		// diff
		while ( it != lines.end() )
		{
//			kdDebug() << (*it) << endl;
			// FIXME
			// not completely correct, diff and Index: have to be outside of
			// an ed/rcs hunk to be a valid new file, else it could be that
			// a line in the file starts with "diff -" or "Index: "
			if ( (*it).startsWith( "diff " ) )
			{
				DiffModel* model = new DiffModel();
				int result = model->parseDiff( m_format, *file );
				if ( result == 0 )
				{
					kdDebug() << "One file parsed" << endl;
					model->setIndex( modelIndex++ );
					m_models.append( model );
				}
				else
				{
					kdDebug() << "Trouble, file parsing failed with result = " << result << endl;
					kdDebug() << file->join( "\n" ) << endl;
					delete model;
				}
				file->clear();
			}
			else if ( (*it).startsWith( "Only in " ) )
			{
				// skip...
				kdDebug() << "Skipping : " << (*it) << endl;
			}
			else
			{
				file->append( *it );
			}
			++it;
		}

		if ( file->count() > 0 )
		{
			DiffModel* model = new DiffModel();
			int result = model->parseDiff( m_format, *file );
			if ( result == 0 )
			{
				kdDebug() << "One file parsed" << endl;
				model->setIndex( modelIndex++ );
				m_models.append( model );
			}
			else
			{
				kdDebug() << "Trouble, file parsing failed with result = " << result << endl;
				kdDebug() << file->join( "\n" ) << endl;
				delete model;
			}
			file->clear();
		}

		if ( m_models.count() > 1 ) {
			kdDebug() << "MultiFileDiff" << endl;
			m_type = MultiFileDiff;
		}
		else {
			kdDebug() << "SingleFileDiff" << endl;
			m_type = SingleFileDiff;
		}
	}
	// delete old one and contruct a new one
	delete m_modelIt;
	m_modelIt = new QPtrListIterator<DiffModel>( m_models );
	m_selectedModel = m_modelIt->toFirst();
	delete m_diffIt;
	m_diffIt = new QPtrListIterator<Difference>( m_selectedModel->differences() );
	m_selectedDifference = m_diffIt->toFirst();

	emit modelsChanged( &m_models );
	emit setSelection( m_selectedModel, m_selectedDifference );

	return 0; // FIXME better error handling
};

void KompareModelList::clear()
{
	m_models.clear();
	emit modelsChanged( &m_models );
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
	if( m_sourceURL.directory(false,false) == m_sourceURL.url() ) {
		return m_sourceURL;
	} else {
		return m_sourceURL.upURL();
	}
}

KURL KompareModelList::destinationBaseURL() const
{
	if( m_destinationURL.directory(false,false) == m_destinationURL.url() ) {
		return m_destinationURL;
	} else {
		return m_destinationURL.upURL();
	}
}

#include "komparemodellist.moc"

/* vim: set ts=4 sw=4 noet: */

