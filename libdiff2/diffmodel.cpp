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
	m_sourcePath( "" ),
	m_destinationPath( "" ),
	m_sourceFile( "" ),
	m_destinationFile( "" ),
	m_sourceTimestamp( "" ),
	m_destinationTimestamp( "" ),
	m_sourceRevision( "" ),
	m_destinationRevision( "" ),
	m_appliedCount( 0 ),
	m_modified( false ),
	m_diffIndex( 0 ),
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
		m_sourceFile = m_source.mid( pos+1, m_source.length() - pos );
	else
		m_sourceFile = m_source;

	kdDebug(8101) << m_source << " was split into " << m_sourcePath << " and " << m_sourceFile << endl;
}

void DiffModel::splitDestinationInPathAndFileName()
{
	int pos;

	if( ( pos = m_destination.findRev( "/" ) )>= 0 )
		m_destinationPath = m_destination.mid( 0, pos+1 );

	if( ( pos = m_destination.findRev( "/" ) ) >= 0 )
		m_destinationFile = m_destination.mid( pos+1, m_destination.length() - pos );
	else
		m_destinationFile = m_source;

	kdDebug(8101) << m_destination << " was split into " << m_destinationPath << " and " << m_destinationFile << endl;
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

		m_diffIndex = model.m_diffIndex;
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

Difference* DiffModel::firstDifference()
{
	kdDebug( 8101 ) << "DiffModel::firstDifference()" << endl;
	m_diffIndex = 0;
	kdDebug( 8101 ) << "m_diffIndex = " << m_diffIndex << endl;

	m_selectedDifference = m_differences.at( m_diffIndex );

	return m_selectedDifference;
}

Difference* DiffModel::lastDifference()
{
	kdDebug( 8101 ) << "DiffModel::lastDifference()" << endl;
	m_diffIndex = m_differences.count() - 1;
	kdDebug( 8101 ) << "m_diffIndex = " << m_diffIndex << endl;

	m_selectedDifference = m_differences.at( m_diffIndex );

	return m_selectedDifference;
}

Difference* DiffModel::prevDifference()
{
	kdDebug( 8101 ) << "DiffModel::prevDifference()" << endl;
	if ( --m_diffIndex < m_differences.count() )
	{
		kdDebug( 8101 ) << "m_diffIndex = " << m_diffIndex << endl;
		m_selectedDifference = m_differences.at( m_diffIndex );
	}
	else
	{
		m_selectedDifference = 0;
		m_diffIndex = 0;
		kdDebug( 8101 ) << "m_diffIndex = " << m_diffIndex << endl;
	}

	return m_selectedDifference;
}

Difference* DiffModel::nextDifference()
{
	kdDebug( 8101 ) << "DiffModel::nextDifference()" << endl;
	if (  ++m_diffIndex < m_differences.count() )
	{
		kdDebug( 8101 ) << "m_diffIndex = " << m_diffIndex << endl;
		m_selectedDifference = m_differences.at( m_diffIndex );
	}
	else
	{
		m_selectedDifference = 0;
		m_diffIndex = 0; // just for safety...
		kdDebug( 8101 ) << "m_diffIndex = " << m_diffIndex << endl;
	}

	return m_selectedDifference;
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
		m_modified = false;
	else
		m_modified = true;

	emit setModified( m_modified );

	m_selectedDifference->apply( apply );
}

void DiffModel::applyAllDifferences( bool apply )
{
	if ( apply )
	{
		m_appliedCount = m_differences.count();
		m_modified = true;
	}
	else
	{
		m_appliedCount = 0;
		m_modified = false;
	}

	Difference* difference = m_differences.first();
	while ( difference )
	{
		difference->apply( apply );
		difference = m_differences.next();
	}
}

void DiffModel::slotSetModified( bool modified )
{
	m_modified = modified;
	if ( modified )
		emit setModified( true );
	else
		emit setModified( false );
}

bool DiffModel::setSelectedDifference( Difference* diff )
{
	kdDebug(8101) << "diff = " << diff << endl;
	kdDebug(8101) << "m_selectedDifference = " << m_selectedDifference << endl;

	if ( diff != m_selectedDifference )
	{
		if ( ( m_differences.findRef( diff ) ) == -1 )
			return false;
		m_diffIndex = m_differences.findRef( diff );
		kdDebug( 8101 ) << "m_diffIndex = " << m_diffIndex << endl;
		m_selectedDifference = diff;
	}

	return true;
}

#include "diffmodel.moc"

/* vim: set ts=4 sw=4 noet: */
