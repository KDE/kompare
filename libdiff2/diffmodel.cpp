/***************************************************************************
                                diffmodel.cpp  -  description
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

#include <qregexp.h>

#include <kdebug.h>
#include <klocale.h>

#include "difference.h"
#include "diffhunk.h"
#include "diffmodel.h"

using namespace Diff2;

/**  */
DiffModel::DiffModel( KURL srcBaseURL, KURL destBaseURL ) :
	m_sourceBaseURL( srcBaseURL ),
	m_destinationBaseURL( destBaseURL ),
	m_sourceFile( "" ),
	m_sourceTimestamp( "" ),
	m_sourceRevision( "" ),
	m_destinationFile( "" ),
	m_destinationTimestamp( "" ),
	m_destinationRevision( "" ),
	m_appliedCount( 0 ),
	m_modified( false ),
	m_index( 0 ),
	m_selectedDifference( 0 )
{
}

/**  */
DiffModel::~DiffModel()
{
}

DiffModel& DiffModel::operator=( const DiffModel& model )
{
	if ( &model != this ) // Guard from self-assignment
	{
		m_sourceBaseURL = model.m_sourceBaseURL;
		m_destinationBaseURL = model.m_destinationBaseURL;
		m_sourceFile = model.m_sourceFile;
		m_sourceTimestamp = model.m_sourceTimestamp;
		m_sourceRevision = model.m_sourceRevision;
		m_destinationFile = model.m_destinationFile;
		m_destinationTimestamp = model.m_destinationTimestamp;
		m_destinationRevision = model.m_destinationRevision;
		m_appliedCount = model.m_appliedCount;
		m_modified = model.m_modified;

		m_index = model.m_index;
		m_selectedDifference = model.m_selectedDifference;
	}

	return *this;
}

const QPtrList<Difference>& DiffModel::allDifferences()
{
	if ( m_hunks.count() != 0 )
	{
		DiffHunk* hunk;
		for ( hunk = m_hunks.first(); hunk; hunk = m_hunks.next() )
		{
			QPtrListIterator<Difference> diffIt( hunk->differences() );
			while ( *diffIt )
			{
				m_allDifferences.append( *diffIt );
				++diffIt;
			}
		}
		return m_allDifferences;
	}
	else
	{
		QPtrList<Difference> *diffList = new QPtrList<Difference>();
		return *diffList;
	}
}

const QString DiffModel::sourceFile() const
{
	QString sourceFile = m_sourceFile;
	int pos;

	if( ( pos = sourceFile.findRev( "/" ) ) >= 0 )
		sourceFile = sourceFile.replace( 0, pos+1, "" );

	kdDebug() << "SF : " << sourceFile << endl;

	return sourceFile;
}

const QString DiffModel::destinationFile() const
{
	QString destinationFile = m_destinationFile;
	int pos;

	if( ( pos = destinationFile.findRev( "/" ) ) >= 0 )
		destinationFile = destinationFile.replace( 0, pos+1, "" );

	kdDebug() << "DF : " << destinationFile << endl;

	return destinationFile;
}

const QString DiffModel::sourcePath() const
{
	QString sourcePath( "" );
	int pos;

	if( ( pos = m_sourceFile.findRev( "/" ) ) >= 0 )
		sourcePath = m_sourceFile.mid( 0, pos+1 );

	kdDebug() << "SP : " << sourcePath << endl;

	return sourcePath;
}

const QString DiffModel::destinationPath() const
{
	QString destinationPath( "" );
	int pos;

	if( ( pos = m_destinationFile.findRev( "/" ) )>= 0 )
		destinationPath = m_destinationFile.mid( 0, pos+1 );

	kdDebug() << "DP : " << destinationPath << endl;

	return destinationPath;
}

void DiffModel::setSourceFile( QString file )
{
	m_sourceFile = file;
}

void DiffModel::setDestinationFile( QString file )
{
	m_destinationFile = file;
}

void DiffModel::setSourceTimestamp( QString timestamp )
{
	m_sourceTimestamp = timestamp;
}

void DiffModel::setDestinationTimestamp( QString timestamp )
{
	m_destinationTimestamp = timestamp;
}

void DiffModel::setSourceRevision( QString revision )
{
	m_destinationRevision = revision;
}

void DiffModel::setDestinationRevision( QString revision )
{
	m_destinationRevision = revision;
}

void DiffModel::addHunk( DiffHunk* hunk )
{
	m_hunks.append( hunk );
}

void DiffModel::addDiff( Difference* diff )
{
	m_differences.append( diff );
}

void DiffModel::applyDifference( bool apply )
{
	if ( apply && !m_selectedDifference->applied() )
		m_appliedCount++;
	else if ( !apply && m_selectedDifference->applied() )
		m_appliedCount--;

	if ( m_appliedCount == 0 )
	{
		m_modified = false;
		emit setModified( false );
	}
	else
	{
		m_modified = true;
		emit setModified( true );
	}

	m_selectedDifference->apply( apply );
}

void DiffModel::applyAllDifferences( bool apply )
{
	if ( apply )
	{
		m_appliedCount = m_differences.count();
		m_modified = true;
		emit setModified( true );
	}
	else
	{
		m_appliedCount = 0;
		m_modified = false;
		emit setModified( false );
	}

	QPtrListIterator<Difference> it( m_differences );
	for ( ;it.current(); ++it )
		(*it)->apply( apply );
}

void DiffModel::slotSetModified( bool modified )
{
	m_modified = modified;
	if ( modified )
		emit setModified( true );
	else
		emit setModified( false );
}

void DiffModel::setSelectedDifference( Difference* diff )
{
	if ( diff != m_selectedDifference )
		m_selectedDifference = diff;
}

#include "diffmodel.moc"

/* vim: set ts=4 sw=4 noet: */
