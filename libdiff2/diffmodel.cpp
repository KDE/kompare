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

#include <qregexp.h>

#include <kdebug.h>
#include <klocale.h>

#include "difference.h"
#include "diffhunk.h"
#include "diffmodel.h"

/**  */
DiffModel::DiffModel( KURL srcBaseURL, KURL destBaseURL ) :
	m_srcBaseURL( srcBaseURL ),
	m_destBaseURL( destBaseURL ),
	m_sourceFile( "" ),
	m_destinationFile( "" ),
	m_appliedCount( 0 ),
	m_modified( false ),
	m_index( 0 )
{
};

/**  */
DiffModel::~DiffModel()
{
};

int DiffModel::parseDiff( enum Kompare::Format format, const QStringList& lines )
{
	switch( format )
	{
		case Context: return parseContextDiff( lines );
		case Ed:      return parseEdDiff     ( lines );
		case Normal:  return parseNormalDiff ( lines );
		case RCS:     return parseRCSDiff    ( lines );
		case Unified: return parseUnifiedDiff( lines );
		default:      return -1;
	}
}

/**  */
int DiffModel::parseContextDiff( const QStringList& list )
{
	kdDebug( 8101 ) << "Context diff parsing:" << endl;

	// '  ' unchanged (context info)
	// '- ' removed in new file
	// '+ ' added in new file
	// '! ' changed in new file

	// '***************' start of hunk (15 *)
	// '*** number,number ****' start of old file
	// '--- number,number ----' start of new file
	int linenoA, linenoB;
	QStringList::ConstIterator it = list.begin();

	if ( it == list.end() )
		return 1; // no differences, error imo so changed it into 1

	QRegExp source( "^\\*\\*\\* ([^\\t]+)\\t([^\\t]+)(\\t?)(.*)$" );

	while ( it != list.end() )
	{
		if ( source.exactMatch( *it ) )
			break;
		++it;
	}

	if ( it == list.end() )
		return 2; // screwed

	m_sourceFile = source.cap( 1 );
	m_sourceTimestamp = source.cap( 2 );
	m_sourceRevision = source.cap( 4 );

//	kdDebug( 8101 ) << " source: " << m_sourceFile << endl;
//	kdDebug( 8101 ) << "   time: " << m_sourceTimestamp << endl;
//	kdDebug( 8101 ) << "    rev: " << m_sourceRevision << endl;

	if( ++it == list.end() )
		return 3;

	QRegExp dest( "^--- ([^\\t]+)\\t([^\\t]+)(\\t?)(.*)$" );
	if( !dest.exactMatch( *it ) )
		return 4;

	m_destinationFile = dest.cap( 1 );
	m_destinationTimestamp = dest.cap( 2 );
	m_destinationRevision = dest.cap( 4 );

//	kdDebug( 8101 ) << "   dest: " << m_destinationFile << endl;
//	kdDebug( 8101 ) << "   time: " << m_destinationTimestamp << endl;
//	kdDebug( 8101 ) << "    rev: " << m_destinationRevision << endl;

	++it;

	QRegExp head1    ( "^\\*{15}( ?)(.*)$" );
	QRegExp headS    ( "^\\*{3} ([0-9]+),([0-9]+) \\*{4}$" );
	QRegExp headD    ( "^-{3} ([0-9]+),([0-9]+) -{4}$" );
	QRegExp line     ( "^([-!+ ]) (.*)$" );
	QRegExp unchanged( "^  (.*)$" );
	QRegExp changed  ( "^! (.*)$" );
	QRegExp removed  ( "^- (.*)$" );
	QRegExp added    ( "^\\+ (.*)$" );

	int diffIndex = 0;
	while ( it != list.end() ) {

		if( !head1.exactMatch( *it ) )
			return 5;

		++it;

		if( !headS.exactMatch( *it ) )
			return 6;

		linenoA = headS.cap( 1 ).toInt();

		++it;

		QStringList oldLines;
		for( ; it != list.end() && line.exactMatch( *it ); ++it ) {
			oldLines.append( line.cap( 0 ) );
		}

		if( !headD.exactMatch( *it ) )
			return 7;

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
//				kdDebug( 8101 ) << "Delete: " << endl;
				diff = new Difference( linenoA, linenoB );
				diff->setType( Difference::Delete );
				diff->setIndex( diffIndex++ );
				m_differences.append( diff );
				hunk->add( diff );
				for( ; oldIt != oldLines.end() && removed.exactMatch( *oldIt ); ++oldIt ) {
					kdDebug( 8101 ) << " " << removed.cap( 1 ) << endl;
					diff->addSourceLine( removed.cap( 1 ) );
					linenoA++;
				}
			}

			else if( newIt != newLines.end() && added.exactMatch( *newIt ) ) {
//				kdDebug( 8101 ) << "Insert: " << endl;
				diff = new Difference( linenoA, linenoB );
				diff->setType( Difference::Insert );
				diff->setIndex( diffIndex++ );
				m_differences.append( diff );
				hunk->add( diff );
				for( ; newIt != newLines.end() && added.exactMatch( *newIt ); ++newIt ) {
					kdDebug( 8101 ) << " " << added.cap( 1 ) << endl;
					diff->addDestinationLine( added.cap( 1 ) );
					linenoB++;
				}
			}

			else if( (oldIt != oldLines.end() && unchanged.exactMatch( *oldIt ) ) ||
			    (newIt != newLines.end() && unchanged.exactMatch( *newIt ) ) ) {
//				kdDebug( 8101 ) << "Unchanged: " << endl;
				diff = new Difference( linenoA, linenoB );
				diff->setType( Difference::Unchanged );
				hunk->add( diff );
				while( (oldIt != oldLines.end() && unchanged.exactMatch( *oldIt ) ) ||
				       (newIt != newLines.end() && unchanged.exactMatch( *newIt ) ) ) {
					QString l;
					if( oldIt != oldLines.end() ) {
						l = unchanged.cap( 1 );
						kdDebug( 8101 ) << " " << l << endl;
						++oldIt;
					}
					if( newIt != newLines.end() ) {
						l = unchanged.cap( 1 );
						kdDebug( 8101 ) << " " << l << endl;
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
//				kdDebug( 8101 ) << "Changed: " << endl;
				diff = new Difference( linenoA, linenoB );
				diff->setType( Difference::Change );
				diff->setIndex( diffIndex++ );
				m_differences.append( diff );
				hunk->add( diff );
				while( oldIt != oldLines.end() && changed.exactMatch( *oldIt ) ) {
					kdDebug( 8101 ) << " " << changed.cap( 1 ) << endl;
					diff->addSourceLine( changed.cap( 1 ) );
					linenoA++;
					++oldIt;
				}
				while( newIt != newLines.end() && changed.exactMatch( *newIt ) ) {
					kdDebug( 8101 ) << " " << changed.cap( 1 ) << endl;
					diff->addDestinationLine( changed.cap( 1 ) );
					linenoB++;
					++newIt;
				}
			}

			else
				return 8;

		}
	}

	return 0;
};

/**  */
int DiffModel::parseEdDiff( const QStringList& list )
{
	// FIXME:
	// Somehow we have to ask the user what file this is a diff against,
	// if this is a single file diff. If (s)he can't help us with that
	// we are fucked...
	kdDebug( 8101 ) << "Ed diff parsing:" << endl;

	QString line;
	Difference* diff = 0L;
	QStringList::ConstIterator it = list.begin();

	if ( it == list.end() )
		return 0; // Nothing to parse, should not happen though

	QRegExp dot		( "^." );					// starts with a dot
	QRegExp head1	( "^(\\d+)([acd])" );		// number[acd]
	QRegExp head2	( "^(\\d+),(\\d+)[acd]" );	// number,number[acd]

	int diffIndex = 0;
	while ( it != list.end() )
	{
		kdDebug( 8101 ) << "Line is:" << line << endl;
		if ( head1.exactMatch( *it ) )
		{
			// number[acd]
			kdDebug( 8101 ) << "Found an added/removed/changed part" << endl;
			++it;	// next line or lines have the changes
			// op is operator
			// ln is linenumber
			char op = *(head1.cap( 2 ).latin1());
			int  ln = head1.cap( 1 ).toInt();

			switch ( op )
			{
			case 'a' :
				diff = new Difference( ln, 0 );
				diff->setType( Difference::Insert );
				diff->setIndex( diffIndex++ );
				break;
			case 'c' :
				diff = new Difference( 0, 0 );
				diff->setType( Difference::Change );
				diff->setIndex( diffIndex++ );
				break;
			case 'd' :
				diff = new Difference( 0, ln );
				diff->setType( Difference::Delete );
				diff->setIndex( diffIndex++ );
				break;
			default	:
				break;	// something is fucked here...
			}

			while( !dot.exactMatch( *it ) )
			{
				kdDebug( 8101 ) << "A/C/D: Line is: " << line << endl;
			}
			// dot line found, end of block...
		}
		else if ( head2.exactMatch( *it ) )
		{
			// number,number[acd]
			kdDebug( 8101 ) << "Found an added/removed/changed part with number of lines" << endl;
			++it;	// next line or lines have the changes
			// op is operator
			// ln is linenumber
			// nol is number of lines
			char op = *(head2.cap( 3 ).latin1());
			int  ln = head2.cap( 1 ).toInt();
			int nol = head2.cap( 2 ).toInt();

			switch ( op )
			{
			case 'a' :
				diff->setType( Difference::Insert );
				break;
			case 'c' :
				diff->setType( Difference::Change );
				break;
			case 'd' :
				diff->setType( Difference::Delete );
				break;
			default	:
				break;	// something is fucked here...
			}

			while( !dot.exactMatch( *it ) )
			{
				kdDebug( 8101 ) << "A/C/D: Line is: " << line << endl;
			}
			// dot line found, end of block...
		}
//		else
//		{
//			kdDebug( 8101 ) << "Oops something is screwed here..." << endl;
//			kdDebug( 8101 ) << line << endl;
//			return 1;
//		}
		++it;
	}

//	KMessageBox::information( 0, i18n( "Sorry not yet implemented" ) );
//	return 1;
	return 0;
};

/**  */
int DiffModel::parseNormalDiff( const QStringList& list )
{
	kdDebug( 8101 ) << "Normal diff parsing:" << endl;

	QRegExp head( "^([0-9]+)([acd]?)([0-9]*),([0-9]+)([acd]?)([0-9]*)$" );
	QRegExp source( "< (.*)$" );
	QRegExp dest  ( "> (.*)$" );
	QRegExp div   ( "^---$" );

	if ( list.count() == 0 )
		return 1; // no lines to compare

	int linenoA, linenoB;
	QStringList::ConstIterator it = list.begin();

	while ( it != list.end() )
	{
		if ( head.exactMatch( *it ) )
		{
			--it;
			break;
		}
	}

	if ( it == list.end() )
		return 2; //

	int diffIndex = 0;
	while ( it != list.end() )
	{
		if( !head.exactMatch( *it ) ) return 1;

		linenoA = head.cap( 1 ).toInt();
		linenoB = head.cap( 4 ).toInt();

		DiffHunk* hunk = new DiffHunk( linenoA, linenoB );
		m_hunks.append( hunk );

		Difference* diff = new Difference( linenoA, linenoB );
		diff->setIndex( diffIndex++ );
		hunk->add( diff );
		m_differences.append( diff );

		if( head.cap( 2 ) == "a" ) {
			diff->setType( Difference::Insert );
		} else if ( head.cap( 2 ) == "c" ) {
			diff->setType( Difference::Change );
		} else if ( head.cap( 5 ) == "d" ) {
			diff->setType( Difference::Delete );
		} else return 1;

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
int DiffModel::parseRCSDiff( const QStringList& list )
{
	kdDebug( 8101 ) << "RCS  diff parsing:" << endl;

	// FIXME: does not work...
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
	int linenoA = 0, linenoB = 0, nolinesA, nolinesB;
	QStringList::ConstIterator it = list.begin();

	if ( it == list.end() )
		return 0; // No lines to parse

	kdDebug( 8101 ) << "There are lines..." << endl;

	DiffHunk* hunk = new DiffHunk( linenoA, linenoB );
	m_hunks.append( hunk );

	int diffIndex = 0;
	while( it != list.end() )
	{
		line = (*it);
		kdDebug( 8101 ) << "Line is: " << line << endl;

		if ( line.find( QRegExp( "^a[0-9]+ [0-9]+" ), 0 ) == 0 )
		{
			kdDebug( 8101 ) << "Added line found." << endl;
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
			diff->setIndex( diffIndex++ );

			++it;
			line = (*it);

			while ( ( it != list.end() ) && ( line.find( QRegExp( "^[ad][0-9]+ [0-9]+" ), 0 ) != 0 ) )
			{
				// Add it to the difference
				kdDebug( 8101 ) << "AddDestinationLine( " << line << " )" << endl;
				diff->addDestinationLine( line );

				++it;
				line = (*it);
			}
			--it; // We went too far, correcting it here...
			hunk->add( diff );
		}
		else if ( line.find( QRegExp( "^d[0-9]+ [0-9]+" ), 0 ) == 0 )
		{
			kdDebug( 8101 ) << "Delete line found." << endl;
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
//			diff->setIndex( diffIndex++ );

			// We should now add the source lines, unfortunately there are none...

//			hunk->add( diff );
		}
		else
		{
			kdDebug( 8101 ) << "Oops, something is wrong here..." << endl;
			return 1; // faulty output or something went wrong during parsing
		}
		++it;
	}

//	KMessageBox::information( 0, i18n( "Sorry not yet implemented" ) );
//	return 1;
	return 0;
};

/**  */
int DiffModel::parseUnifiedDiff( const QStringList& list )
{
	kdDebug( 8101 ) << "Unified diff parsing:" << endl;

	QStringList::ConstIterator it = list.begin();
	int linenoA, linenoB;

	if( it == list.end() ) return 1; // no differences

	QRegExp source   ( "^--- ([^\\t ]+)[\\t ]+([^\\t ]+)(\\t?)(.*)$" );
	QRegExp dest     ( "^\\+\\+\\+ ([^\\t ]+)[\\t ]+([^\\t ]+)(\\t?)(.*)$" );
	QRegExp line     ( "^([ \\-+])(.*)$" );
	QRegExp unchanged( "^ (.*)$" );
	QRegExp removed  ( "^-(.*)$" );
	QRegExp added    ( "^\\+(.*)$" );
	QRegExp head     ( "^@@ -([0-9]+),([0-9]+) \\+([0-9]+),([0-9]+) @@( ?)(.*)$" );

	while( it != list.end() ) // dont assume we start with a source line
	{
		if( source.exactMatch( *it ) )
			break;
		++it;
	}

	if ( it == list.end() ) // now we can say something is screwed
		return 2;

	m_sourceFile = source.cap( 1 );
	m_sourceTimestamp = source.cap( 2 );
	m_sourceRevision  = source.cap( 4 );

//	kdDebug( 8101 ) << " source: " << m_sourceFile << endl;
//	kdDebug( 8101 ) << "   time: " << m_sourceTimestamp << endl;
//	kdDebug( 8101 ) << "    rev: " << m_sourceRevision << endl;

	if( ++it == list.end() ) return 3; // no next line

	if( !dest.exactMatch( *it ) ) return 4; // destination line is not valid

	m_destinationFile = dest.cap( 1 );
	m_destinationTimestamp = dest.cap( 2 );
	m_destinationRevision  = dest.cap( 4 );

//	kdDebug( 8101 ) << "   dest: " << m_destinationFile << endl;
//	kdDebug( 8101 ) << "   time: " << m_destinationTimestamp << endl;
//	kdDebug( 8101 ) << "    rev: " << m_destinationRevision << endl;

	++it;
	int diffIndex = 0;
	while( it != list.end() )
	{
		if( !head.exactMatch( *it ) ) {
			kdDebug( 8101 ) << "faulty line is   : " << (*it) << endl;
			kdDebug( 8101 ) << "previous line was: " << (*(--it)) << endl;
			return 5; // invalid head line
		}

		linenoA = head.cap( 1 ).toInt();
		linenoB = head.cap( 3 ).toInt();

//		kdDebug( 8101 ) << " hunk: " << linenoA << " " << linenoB << endl;

		DiffHunk* hunk = new DiffHunk( linenoA, linenoB, head.cap( 5 ) );
		m_hunks.append( hunk );

		++it;

		while( it != list.end() && line.exactMatch( *it ) )
		{
			Difference* diff = new Difference( linenoA, linenoB );
			hunk->add( diff );

			if( line.cap( 1 ) == " " )
			{	// context crap
				diff->setType( Difference::Unchanged );
				for( ; it != list.end() && unchanged.exactMatch( *it ); ++it ) {
					diff->addSourceLine( unchanged.cap( 1 ) );
					diff->addDestinationLine( unchanged.cap( 1 ) );
					linenoA++;
					linenoB++;
				}
			}
			else
			{	// This is a real difference, not context crap
				for( ; it != list.end() && removed.exactMatch( *it ); ++it ) {
					diff->addSourceLine( removed.cap( 1 ) );
					linenoA++;
				}
				for( ; it != list.end() && added.exactMatch( *it ); ++it ) {
					diff->addDestinationLine( added.cap( 1 ) );
					linenoB++;
				}
				if ( diff->sourceLineCount() == 0 )
				{
					diff->setType( Difference::Insert );
				}
				else if ( diff->destinationLineCount() == 0 )
				{
					diff->setType( Difference::Delete );
				}
				else
				{
					diff->setType( Difference::Change );
				}
				diff->setIndex( diffIndex++ );
				m_differences.append( diff );
			}
		}
	}
	return 0; // everything went ok
}

const QString DiffModel::srcFile() const
{
	int i = m_sourceFile.findRev( "/" );
	if( i >= 0 )
		return QString( m_sourceFile ).replace( 0, i+1, "" );
	else
		return m_sourceFile;
}

const QString DiffModel::destFile() const
{
	int i = m_destinationFile.findRev( "/" );
	if( i >= 0 )
		return QString( m_destinationFile ).replace( 0, i+1, "" );
	else
		return m_destinationFile;
}

const QString DiffModel::srcPath() const
{
	int pos = m_sourceFile.findRev( "/" );
	if( pos >= 0 )
		return m_sourceFile.mid( 0, pos+1 );
	else
		return QString( "" );
}

const QString DiffModel::destPath() const
{
	int pos = m_destinationFile.findRev( "/" );
	if( pos >= 0 )
		return m_destinationFile.mid( 0, pos+1 );
	else
		return QString( "" );
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

	QPtrListIterator<Difference> it( m_differences );
	for ( ;it.current(); ++it )
		(*it)->apply( apply );
}

void DiffModel::setModified( bool modified )
{
	m_modified = modified;
}

void DiffModel::setSelectedDifference( Difference* diff )
{
	if ( diff != m_selectedDifference )
		m_selectedDifference = diff;
}

#include "diffmodel.moc"

/* vim: set ts=4 sw=4 noet: */
