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
DiffModel::DiffModel( const QString& source, const QString& destination ) :
	m_source( source ),
	m_destination( destination ),
	m_sourceTimestamp( "" ),
	m_destinationTimestamp( "" ),
	m_sourceRevision( "" ),
	m_destinationRevision( "" ),
	m_appliedCount( 0 ),
	m_modified( false ),
	m_index( 0 ),
	m_selectedDifference( 0 )
{
	splitSourceInPathAndFileName();
	splitDestinationInPathAndFileName();
}

/**  */
DiffModel::~DiffModel()
{
}

void DiffModel::splitSourceInPathAndFileName()
{
	int pos;

	if( ( pos = m_source.findRev( "/" ) ) >= 0 )
		m_sourcePath = m_source.mid( 0, pos+1 );

	if( ( pos = m_source.findRev( "/" ) ) >= 0 )
		m_sourceFile = m_source.replace( 0, pos+1, "" );
}

void DiffModel::splitDestinationInPathAndFileName()
{
	int pos;

	if( ( pos = m_destination.findRev( "/" ) )>= 0 )
		m_destinationPath = m_destination.mid( 0, pos+1 );

	if( ( pos = m_destination.findRev( "/" ) ) >= 0 )
		m_destinationFile = m_destination.replace( 0, pos+1, "" );
}

DiffModel& DiffModel::operator=( const DiffModel& model )
{
	if ( &model != this ) // Guard from self-assignment
	{
		m_source = model.m_source;
		m_destination = model.m_destination;
		m_sourcePath = model.m_sourcePath;
		m_sourceFile = model.m_sourceFile;
		m_sourceTimestamp = model.m_sourceTimestamp;
		m_sourceRevision = model.m_sourceRevision;
		m_destinationPath = model.m_destinationPath;
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
	return m_sourceFile;
}

const QString DiffModel::destinationFile() const
{
	return m_destinationFile;
}

const QString DiffModel::sourcePath() const
{
	return m_sourcePath;
}

const QString DiffModel::destinationPath() const
{
	return m_destinationPath;
}

void DiffModel::setSourceFile( QString path )
{
	m_source = path;
	splitSourceInPathAndFileName();
}

void DiffModel::setDestinationFile( QString path )
{
	m_destination = path;
	splitDestinationInPathAndFileName();
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
