/***************************************************************************
                                diffmodel.cpp  -  description
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

#include <qtextstream.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "difference.h"
#include "diffmodel.h"
#include "diffhunk.h"
#include "kdiffmodellist.h"
#include "qregexp3.h"

/**  */
DiffModel::DiffModel()
	: m_sourceFile( i18n( "Source" ) ),
	m_destinationFile( i18n( "Destination" ) ),
	m_appliedCount( 0 ),
	m_modified( false )
{
};

/**  */
DiffModel::~DiffModel()
{
};

KDiff::Format DiffModel::determineDiffFormat( QString line )
{
	if( line.find( QRegExp( "^[0-9]+[0-9,]*[acd][0-9]+[0-9,]*$" ), 0 ) == 0 )
		return Normal;
	else if( line.find( QRegExp( "^--- [^\\t]+\\t" ), 0 ) == 0 )
		return Unified;
	else if( line.find( QRegExp( "^\\*\\*\\* [^\\t]+\\t" ), 0 ) == 0 )
		return Context;
	else if( line.find( QRegExp( "^[acd][0-9]+ [0-9]+" ), 0 ) == 0 )
		return RCS;
	else if( line.find( QRegExp( "^[0-9]+[0-9,]*[acd]" ), 0 ) == 0 )
		return Ed;

	return Unknown;
}

int DiffModel::parseDiff( Format format, const QStringList& lines, QStringList::ConstIterator& it )
{
	m_format = format;

	switch( format )
	{
		case Context: return parseContextDiff( lines, it );
		case Ed:      return parseEdDiff(      lines, it );
		case Normal:  return parseNormalDiff(  lines, it );
		case RCS:     return parseRCSDiff(     lines, it );
		case Unified: return parseUnifiedDiff( lines, it );
		default:      return 1;
	}
}

/**  */
int DiffModel::parseContextDiff( const QStringList& list, QStringList::ConstIterator& it )
{
	// '  ' unchanged (context info)
	// '- ' removed in new file
	// '+ ' added in new file
	// '! ' changed in new file

	// '***************' start of hunk (15 *)
	// '*** number,number ****' start of old file
	// '--- number,number ----' start of new file
	int linenoA, linenoB;

	kdDebug() << "Context diff parsing:" << endl;

	if ( it == list.end() ) return 0; // no differences

	QRegExp3 source( "^\\*\\*\\* ([^\\t]+)\\t([^\\t]+)(\\t?)(.*)$" );
	if( !source.exactMatch( *it ) ) return 1;
	
	m_sourceFile = source.cap( 1 );
	m_sourceTimestamp = source.cap( 2 );
	m_sourceRevision = source.cap( 4 );
	
	kdDebug() << " source: " << m_sourceFile << endl;
	kdDebug() << "   time: " << m_sourceTimestamp << endl;
	kdDebug() << "    rev: " << m_sourceRevision << endl;
	
	if( ++it == list.end() ) return 1;
	
	QRegExp3 dest( "^--- ([^\\t]+)\\t([^\\t]+)(\\t?)(.*)$" );
	if( !dest.exactMatch( *it ) ) return 1;
	
	m_destinationFile = dest.cap( 1 );
	m_destinationTimestamp = dest.cap( 2 );
	m_destinationRevision = dest.cap( 4 );
	
	kdDebug() << "   dest: " << m_destinationFile << endl;
	kdDebug() << "   time: " << m_destinationTimestamp << endl;
	kdDebug() << "    rev: " << m_destinationRevision << endl;
	
	++it;
	
	QRegExp3 head1( "^\\*{15}( ?)(.*)$" );
	QRegExp3 headS( "^\\*{3} ([0-9]+),([0-9]+) \\*{4}$" );
	QRegExp3 headD( "^-{3} ([0-9]+),([0-9]+) -{4}$" );
	QRegExp3 line( "^([-!+ ]) (.*)$" );
	QRegExp3 unchanged( "^  (.*)$" );
	QRegExp3 changed(    "^! (.*)$" );
	QRegExp3 removed(   "^- (.*)$" );
	QRegExp3 added(     "^\\+ (.*)$" );
	
	while ( it != list.end() ) {
		
		if( !head1.exactMatch( *it ) ) return 1;
		
		++it;
		
		if( !headS.exactMatch( *it ) ) return 1;
		
		linenoA = headS.cap( 1 ).toInt();
		
		++it;
		
		QStringList oldLines;
		for( ; it != list.end() && line.exactMatch( *it ); ++it ) {
			oldLines.append( line.cap( 0 ) );
		}
		
		if( !headD.exactMatch( *it ) ) return 1;
		
		linenoB = headD.cap( 1 ).toInt();
		
		++it;
		
		QStringList newLines;
		for( ; it != list.end() && line.exactMatch( *it ); ++it ) {
			newLines.append( line.cap( 0 ) );
		}
		
		DiffHunk* hunk = new DiffHunk( linenoA, linenoB, head1.cap( 2 )  );
		m_hunks.append( hunk );
		
		QStringList::Iterator oldIt = oldLines.begin();
		QStringList::Iterator newIt = newLines.begin();
		
		Difference* diff;
		while( oldIt != oldLines.end() || newIt != newLines.end() ) {
			
			if( oldIt != oldLines.end() && removed.exactMatch( *oldIt ) ) {
				kdDebug() << "Delete: " << endl;
				diff = new Difference( linenoA, linenoB );
				diff->setType( Difference::Delete );
				m_differences.append( diff );
				hunk->add( diff );
				for( ; oldIt != oldLines.end() && removed.exactMatch( *oldIt ); ++oldIt ) {
					kdDebug() << " " << removed.cap( 1 ) << endl;
					diff->addSourceLine( removed.cap( 1 ) );
					linenoA++;
				}
			}
			
			else if( newIt != newLines.end() && added.exactMatch( *newIt ) ) {
				kdDebug() << "Insert: " << endl;
				diff = new Difference( linenoA, linenoB );
				diff->setType( Difference::Insert );
				m_differences.append( diff );
				hunk->add( diff );
				for( ; newIt != newLines.end() && added.exactMatch( *newIt ); ++newIt ) {
					kdDebug() << " " << added.cap( 1 ) << endl;
					diff->addDestinationLine( added.cap( 1 ) );
					linenoB++;
				}
			}
			
			else if( (oldIt != oldLines.end() && unchanged.exactMatch( *oldIt ) ) ||
			    (newIt != newLines.end() && unchanged.exactMatch( *newIt ) ) ) {
				kdDebug() << "Unchanged: " << endl;
				diff = new Difference( linenoA, linenoB );
				diff->setType( Difference::Unchanged );
				hunk->add( diff );
				while( (oldIt != oldLines.end() && unchanged.exactMatch( *oldIt ) ) ||
				       (newIt != newLines.end() && unchanged.exactMatch( *newIt ) ) ) {
					QString l;
					if( oldIt != oldLines.end() ) {
						l = unchanged.cap( 1 );
						kdDebug() << " " << l << endl;
						++oldIt;
					}
					if( newIt != newLines.end() ) {
						l = unchanged.cap( 1 );
						kdDebug() << " " << l << endl;
						++newIt;
					}
					diff->addSourceLine( l );
					diff->addDestinationLine( l );
					linenoA++;
					linenoB++;
				}
			}
			
			else if( (oldIt != oldLines.end() && changed.exactMatch( *oldIt ) ) ||
			    (newIt != newLines.end() && changed.exactMatch( *newIt ) ) ) {
				kdDebug() << "Changed: " << endl;
				diff = new Difference( linenoA, linenoB );
				diff->setType( Difference::Change );
				m_differences.append( diff );
				hunk->add( diff );
				while( oldIt != oldLines.end() && changed.exactMatch( *oldIt ) ) {
					kdDebug() << " " << changed.cap( 1 ) << endl;
					diff->addSourceLine( changed.cap( 1 ) );
					linenoA++;
					++oldIt;
				}
				while( newIt != newLines.end() && changed.exactMatch( *newIt ) ) {
					kdDebug() << " " << changed.cap( 1 ) << endl;
					diff->addDestinationLine( changed.cap( 1 ) );
					linenoB++;
					++newIt;
				}
			}
			
			else return 1;
			
		}
	}
	
	return 0;
};

/**  */
int DiffModel::parseEdDiff( const QStringList& list, QStringList::ConstIterator& it )
{
	kdDebug() << "Ed diff parsing:" << endl;

	QString line;

	if ( it == list.end() )
		return 0; // Nothing to parse, should not happen though

	while ( it != list.end() )
	{
		line = (*it);
		kdDebug() << "Line is:" << line << endl;
		if ( line.find( QRegExp( "^[0-9]+[acd]" ), 0 ) == 0 )
		{
			kdDebug() << "Found a added/removed/changed part" << endl;
			// Only startline with operator, nothing else
			// Number of lines is the same as number of lines changed
			while( line.find( QRegExp( "^." ), 0 ) != 0 )
			{
				kdDebug() << "A/C/D: Line is: " << line << endl;
				++it;
				line = (*it);
			}
		}
		else if ( line.find( QRegExp( "^[0-9]+,[0-9]a" ), 0 ) == 0 )
		{
			kdDebug() << "Found a added line with number of lines to add." << endl;
			// Hmmm... i wonder if this ever occurs...
			while( line.find( QRegExp( "^." ), 0 ) != 0 )
			{
				kdDebug() << "Added: Line is: " << line << endl;
				// Add lines until we reach a line that starts with a '.'
				++it;
				line = (*it);
			}
		}
		else if ( line.find( QRegExp( "^[0-9]+,[0-9]c" ), 0 ) == 0 )
		{
			kdDebug() << "Found a changed line with number of lines changed." << endl;
			// Startline, endline, operator change
			// Number of lines in the change has changed
			// so could be added or deleted in disguise (sp?)
			while( line.find( QRegExp( "^." ), 0 ) != 0 )
			{
				kdDebug() << "Change: Line is: " << line << endl;
				++it;
				line = (*it);
			}
		}
		else if ( line.find( QRegExp( "^[0-9]+,[0-9]d" ), 0 ) == 0 )
		{
			kdDebug() << "Found a delete line with number of lines to delete." << endl;
			// Startline, endline, operator delete
			while( line.find( QRegExp( "^." ), 0 ) != 0 )
			{
				kdDebug() << "Delete: Line is: " << line << endl;
				++it;
				line = (*it);
			}
		}
		else
		{
			kdDebug() << "Oops something is screwed here..." << endl;
			kdDebug() << line << endl;
			return 1;
		}
		++it;
	}

//	KMessageBox::information( 0, i18n( "Sorry not yet implemented" ) );
//	return 1;
	return 0;
};

/**  */
int DiffModel::parseNormalDiff( const QStringList& list, QStringList::ConstIterator& it )
{
	int linenoA, linenoB;
	
	kdDebug() << "Normal diff parsing:" << endl;
	
	while ( it != list.end() ) {
		
		QRegExp3 head( "^([0-9]+)([acd]?)([0-9]*),([0-9]+)([acd]?)([0-9]*)$" );
		if( !head.exactMatch( *it ) ) return 1;
		
		linenoA = head.cap( 1 ).toInt();
		linenoB = head.cap( 4 ).toInt();
		
		DiffHunk* hunk = new DiffHunk( linenoA, linenoB );
		m_hunks.append( hunk );
		
		Difference* diff = new Difference( linenoA, linenoB );
		hunk->add( diff );
		m_differences.append( diff );
		
		if( head.cap( 2 ) == "a" ) {
			diff->setType( Difference::Insert );
		} else if ( head.cap( 2 ) == "c" ) {
			diff->setType( Difference::Change );
		} else if ( head.cap( 5 ) == "d" ) {
			diff->setType( Difference::Delete );
		} else return 1;
		
		QRegExp3 source( "< (.*)$" );
		QRegExp3 dest( "> (.*)$" );
		QRegExp3 div( "^---$" );
		
		it++;
		
		for( ; it != list.end() && source.exactMatch( *it ); ++it ) {
			diff->addSourceLine( source.cap( 1 ) );
		}
		if( it != list.end() && div.exactMatch( *it ) ) {
			++it;
		}
		for( ; it != list.end() && dest.exactMatch( *it ); ++it ) {
			diff->addDestinationLine( dest.cap( 1 ) );
		}
	}
	
	return 0;
};

/**  */
int DiffModel::parseRCSDiff( const QStringList& list, QStringList::ConstIterator& it )
{
	kdDebug() << "RCS  diff parsing:" << endl;

	/* A rcs diff has one or 2 lines describing the changes */
	/* Unfortunately there is no char that indicates the textlines */
	/* from the difftype + linenos lines */
	/* Another problem is the fact that the deleted lines are not itself */
	/* in the difffile/diffoutput */
	/* But i guess that was not needed for rcs, since the RCS- */
	/* server/program knows the original */
	/* Anyway a line looks like this: */
	/* aLINENUMBER NUMBEROFLINES -> added NUMBEROFLINES lines in newfile at LINENUMBER, the added lines will follow this statement*/
	/* dLINENUMBER NUMBEROFLINES -> deleted in NUMBEROFLINES in oldfile at LINENUMBER */

	QString line;
	int linenoA, linenoB, nolinesA, nolinesB;

	if ( it == list.end() )
		return 0; // No lines to parse

	kdDebug() << "There are lines..." << endl;

	DiffHunk* hunk = new DiffHunk( linenoA, linenoB );
	m_hunks.append( hunk );

	while( it != list.end() )
	{
		line = (*it);
		kdDebug() << "Line is: " << line << endl;

		if ( line.find( QRegExp( "^a[0-9]+ [0-9]+" ), 0 ) == 0 ) 
		{
			kdDebug() << "Added line found." << endl;
			int len, pos;
			line.replace( 0, 1, "" ); // Strip the 'a'
			if ( (pos = QRegExp( "^[0-9]+" ).match( line, 0, &len ) ) < 0 ) return 1;
			linenoB = line.mid(pos, len).toInt();
			line.replace( pos, len+1, "" ); // Also strip the extra ' '
			if ( (pos = QRegExp( "^[0-9]+" ).match( line, 0, &len ) ) < 0 ) return 1;
			nolinesB = line.mid(pos, len).toInt();

			DiffHunk* hunk = new DiffHunk( 0, linenoB );
			m_hunks.append( hunk );

			Difference* diff = new Difference( 0, linenoB ); // A is unknown
			diff->setType( Difference::Insert );

			++it;
			line = (*it);

			while ( ( it != list.end() ) && ( line.find( QRegExp( "^[ad][0-9]+ [0-9]+" ), 0 ) != 0 ) )
			{
				// Add it to the difference
				kdDebug() << "AddDestinationLine( " << line << " )" << endl;
				diff->addDestinationLine( line );

				++it;
				line = (*it);
			}
			--it; // We went too far, correcting it here...
			hunk->add( diff );
		}
		else if ( line.find( QRegExp( "^d[0-9]+ [0-9]+" ), 0 ) == 0 )
		{
			kdDebug() << "Delete line found." << endl;
			// We dont have the actual deleted lines in the diffoutput/difffile
			// This will be a big problem in the viewerclass...
			int len, pos;
			line.replace( 0, 1, "" ); // Strip the 'd'
			if ( (pos = QRegExp( "^[0-9]+" ).match( line, 0, &len ) ) < 0 ) return 1;
			linenoA = line.mid(pos, len).toInt();
			line.replace( pos, len+1, "" ); // Also strip the extra ' '
			if ( (pos = QRegExp( "^[0-9]+" ).match( line, 0, &len ) ) < 0 ) return 1;
			nolinesA = line.mid(pos, len).toInt();

// Next lines are commented out because in delete the lines are not here
// so there is nothing to add but an empty diff and an empty hunk so for
// now this is omitted. Maybe we could ask the user for the original file
// so we can indicate the deleted lines.

//			DiffHunk* hunk = new DiffHunk( 0, linenoB );
//			m_hunks.append( hunk );

//			Difference diff = new Difference( linenoA, 0 );
//			diff->setType( Difference::Delete );

			// We should now add the source lines, unfortunately there are none...

//			hunk->add( diff );
		}
		else
		{
			kdDebug() << "Oops, something is wrong here..." << endl;
			return 1; // faulty output or something went wrong during parsing
		}
		++it;
	}

//	KMessageBox::information( 0, i18n( "Sorry not yet implemented" ) );
//	return 1;
	return 0;
};

/**  */
int DiffModel::parseUnifiedDiff( const QStringList& list, QStringList::ConstIterator& it )
{
	int linenoA, linenoB;

	kdDebug() << "Unified diff parsing:" << endl;
	
	if( it == list.end() ) return 0; // no differences
	
	QRegExp3 source( "^--- ([^\\t]+)\\t([^\\t]+)(\\t?)(.*)$" );
	if( !source.exactMatch( *it ) ) return 1;
	
	m_sourceFile = source.cap( 1 );
	m_sourceTimestamp = source.cap( 2 );
	m_sourceRevision = source.cap( 4 );
	
	kdDebug() << " source: " << m_sourceFile << endl;
	kdDebug() << "   time: " << m_sourceTimestamp << endl;
	kdDebug() << "    rev: " << m_sourceRevision << endl;
	
	if( ++it == list.end() ) return 1;
	
	QRegExp3 dest( "^\\+\\+\\+ ([^\\t]+)\\t([^\\t]+)(\\t?)(.*)$" );
	if( !dest.exactMatch( *it ) ) return 1;
	
	m_destinationFile = dest.cap( 1 );
	m_destinationTimestamp = dest.cap( 2 );
	m_destinationRevision = dest.cap( 4 );
	
	kdDebug() << "   dest: " << m_destinationFile << endl;
	kdDebug() << "   time: " << m_destinationTimestamp << endl;
	kdDebug() << "    rev: " << m_destinationRevision << endl;

	++it;
	while( it != list.end() ) {
		
		QRegExp3 head( "^@@ -([0-9]+),([0-9]+) \\+([0-9]+),([0-9]+) @@( ?)(.*)$" );
		if( !head.exactMatch( *it ) ) return 1;
		
		linenoA = head.cap( 1 ).toInt();
		linenoB = head.cap( 3 ).toInt();
		
		kdDebug() << " hunk: " << linenoA << " " << linenoB << endl;
		
		DiffHunk* hunk = new DiffHunk( linenoA, linenoB, head.cap( 6 ) );
		m_hunks.append( hunk );
		
		++it;
		
		QRegExp3 line( "^([ \\-+])(.*)$" );
		QRegExp3 unchanged( "^ (.*)$" );
		QRegExp3 removed(   "^-(.*)$" );
		QRegExp3 added(     "^\\+(.*)$" );
		while( it != list.end() && line.exactMatch( *it ) ) {
			
			Difference* diff = new Difference( linenoA, linenoB );
			hunk->add( diff );
			
			if( line.cap( 1 ) == " " ) {
				diff->setType( Difference::Unchanged );
				for( ; it != list.end() && unchanged.exactMatch( *it ); ++it ) {
					diff->addSourceLine( unchanged.cap( 1 ) );
					diff->addDestinationLine( unchanged.cap( 1 ) );
					linenoA++;
					linenoB++;
				}
			} else {
				for( ; it != list.end() && removed.exactMatch( *it ); ++it ) {
					diff->addSourceLine( removed.cap( 1 ) );
					linenoA++;
				}
				for( ; it != list.end() && added.exactMatch( *it ); ++it ) {
					diff->addDestinationLine( added.cap( 1 ) );
					linenoB++;
				}
				if      ( diff->sourceLineCount() == 0 )      diff->setType( Difference::Insert );
				else if ( diff->destinationLineCount() == 0 ) diff->setType( Difference::Delete );
				else                                          diff->setType( Difference::Change );
				m_differences.append( diff );
			}
		}
	}
	return 0;
}

QString DiffModel::sourceFile()
{
	int i = m_sourceFile.findRev( "/" );
	if( i >= 0 )
		return QString( m_sourceFile ).replace( 0, i+1, "" );
	else
		return m_sourceFile;
}

QString DiffModel::destinationFile()
{
	int i = m_destinationFile.findRev( "/" );
	if( i >= 0 )
		return QString( m_destinationFile ).replace( 0, i+1, "" );
	else
		return m_destinationFile;
}

void DiffModel::toggleApplied( int diffIndex )
{
	Difference* d = m_differences.at( diffIndex );
	if( d->applied() )
		m_appliedCount--;
	else
		m_appliedCount++;
	d->toggleApplied();
	
	setModified( true );
	emit appliedChanged( d );
}

void DiffModel::setModified( bool modified )
{
	m_modified = modified;
}

#include "diffmodel.moc"
