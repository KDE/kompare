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

/**  */
DiffModel::DiffModel()
	: m_sourceFile( i18n( "Source" ) ),
	m_destinationFile( i18n( "Destination" ) )
{
};

/**  */
DiffModel::~DiffModel()
{
};

DiffModel::DiffFormat DiffModel::determineDiffFormat( QString line )
{
	if( line.find( QRegExp( "^[0-9]+[0-9,]*[acd][0-9]+[0-9,]*$" ), 0 ) == 0 )
		return DiffModel::Normal;
	else if( line.find( QRegExp( "^--- [^\\t]+\\t" ), 0 ) == 0 )
		return DiffModel::Unified;
	else if( line.find( QRegExp( "^\\*\\*\\* [^\\t]+\\t" ), 0 ) == 0 )
		return DiffModel::Context;
	else if( line.find( QRegExp( "^[acd][0-9]+ [0-9]+" ), 0 ) == 0 )
		return DiffModel::RCS;
	else if( line.find( QRegExp( "^[0-9]+[0-9,]*[acd]" ), 0 ) == 0 )
		return DiffModel::Ed;

	return DiffModel::Unknown;
}

int DiffModel::parseDiff( enum DiffFormat format, const QStringList& lines, QStringList::ConstIterator& it )
{
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
	QString line;
	QString filenameA, filenameB;
	int pos, len;
	int nolinesA, nolinesB;
	int linenoA, linenoB;

	kdDebug() << "Context diff parsing:" << endl;

	if ( it == list.end() ) return 0; // no differences
	line = (*it);
	kdDebug() << line << endl;

	nolinesA = 0;
	nolinesB = 0;
	kdDebug() << line << endl;

	// Figure out the filename and date of old file
	if ( ( pos = QRegExp( "^\\*\\*\\*" ).match( line, 0, &len ) ) < 0 ) return 1; // invalid context format
	line.replace( QRegExp( "^\\*\\*\\* " ), "" ); // remove leading '*** '
	kdDebug() << line << endl;
	// read name of old file until \t
	if ( ( pos = QRegExp( "\\t" ).match( line, 0, &len ) ) < 0 ) return 1; // invalid context format
	m_sourceFile = line.mid( 0, pos );
	kdDebug() << m_sourceFile << endl;
	// get date if necessary

	++it;
	line = (*it);
	kdDebug() << line << endl;

	// Figure out the filename and date of new file
	if ( ( pos = QRegExp( "^---" ).match( line, 0, &len ) ) < 0 ) return 1; // invalid context format
	line.replace( QRegExp( "^--- " ), "" ); // remove leading '--- '
	// read name of new file until \t
	if ( ( pos = QRegExp( "\\t" ).match( line, 0, &len ) ) < 0 ) return 1; // invalid context format
	m_destinationFile = line.mid( 0, pos );
	kdDebug() << m_destinationFile << endl;
	// get date if necessary

	kdDebug() << "Start parsing the file..." << endl;
	// Start processing the hunks in the diff
	while ( it != list.end() )
	{
		++it;
		line = (*it);
		kdDebug() << line << endl;
		// actual start of the hunks
		if ( ( pos = QRegExp( "^\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*" ).match( line, 0, &len ) ) < 0 ) return 1; // weirdness has happened
		// ok we found a hunk...
		kdDebug() << "We found the start of a hunk" << endl;
		m_noOfHunks++;

		++it;

		while ( it != list.end() )
		{
			line = (*it);
			kdDebug() << line << endl;
			// ok old looks like '*** number,number ****'
			// WARNING: after old new can come immediately to indicate only added lines
			kdDebug() << "Extracting line numbers:" << line << endl;
			if ( ( pos = QRegExp( "^\\*\\*\\* " ).match( line, 0, &len ) ) < 0 ) return 1;
			line.replace( QRegExp( "^\\*\\*\\* "), "" );
			kdDebug() << "After replace ***: " << line << endl;
			// get starting line of old file
			if ( ( pos = QRegExp( "^[0-9]+" ).match( line, 0, &len ) ) < 0 ) return 1;
			linenoA = line.mid( pos, len ).toInt();
			kdDebug() << "LinenoA: " << linenoA << endl;
			line.replace( QRegExp( "^[0-9]+,"), "" );
			if ( ( pos = QRegExp( "^[0-9]+" ).match( line, 0, &len ) ) < 0 ) return 1;
			nolinesA = line.mid( pos, len ).toInt() - linenoA + 1;
			kdDebug() << "NolinesA: " << nolinesA << endl;

			++it;
			line = (*it);
			kdDebug() << "Line before --- check: " << line << endl;
			QStringList oldLines;
			QStringList newLines;

			// do while not start of "new" part of hunk
			while ( (it != list.end()) && (QRegExp( "^---" ).match( line, 0, &len ) < 0) )
			{
				kdDebug() << "Searching in old part of hunk" << endl;
				kdDebug() << "Added this line to oldLines: " << line << endl;
				oldLines.append(line);
				++it;
				line = (*it);
			}

			kdDebug() << "Whoops we found the new part of the hunk" << endl;

			// ok new looks like '--- number,number ----'
			// WARNING: after new a new hunk can come immediately to indicate only removed lines
			if ( ( pos = QRegExp( "^--- " ).match( line, 0, &len ) ) < 0 ) return 1;
			line.replace( QRegExp( "^--- "), "" );
			kdDebug() << "After replace ---: " << line << endl;
			// get starting line of old file
			if ( ( pos = QRegExp( "^[0-9]+" ).match( line, 0, &len ) ) < 0 ) return 1;
			linenoB = line.mid( pos, len ).toInt();
			kdDebug() << "LinenoB: " << linenoB << endl;
			line.replace( QRegExp( "^[0-9]+,"), "" );
			if ( ( pos = QRegExp( "^[0-9]+" ).match( line, 0, &len ) ) < 0 ) return 1;
			nolinesB = line.mid( pos, len ).toInt() - linenoB + 1;
			kdDebug() << "NolinesB: " << nolinesB << endl;

			++it;
			line = (*it);
			kdDebug() << line << endl;

			// do while not next hunk
			while ( (it != list.end()) && (QRegExp( "^\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*" ).match( line, 0, &len ) < 0) )
			{
				kdDebug() << "Searching in new part of hunk" << endl;
				kdDebug() << "Added this line to newLines: " << line << endl;
				newLines.append(line);
				++it;
				line = (*it);
			}

			kdDebug() << "We found the start of a hunk or eof: " << line << endl;

			DiffHunk* hunk = new DiffHunk();
			hunk->lineStartA = linenoA;
			hunk->lineStartB = linenoB;

			Difference* diff;

			// Now we have the lines nicely grouped in old- and newLines, categorize them now...
			if ( oldLines.count() == 0 )
			{ // Only added lines -> make difference from newLines and type Insert
//				diff = new Difference( linenoA, linenoB );
//				diff->setType( Difference::Insert );
				for( QStringList::ConstIterator newIt = newLines.begin(); newIt != newLines.end(); ++newIt)
				{
					QString line = (*newIt);
					if ( line.find( QRegExp( "^  " ), 0 ) == 0 )
					{
						oldLines.append( line );
					}
				}
//				m_differences.append( diff );
//				hunk->add( diff );
			}
			if ( newLines.count() == 0 )
			{ // Only removed lines -> make difference from oldLines and type Delete
				for( QStringList::ConstIterator oldIt = oldLines.begin(); oldIt != oldLines.end(); ++oldIt)
				{
					QString line = (*oldIt);
					if ( line.find( QRegExp( "^  " ), 0 ) == 0 )
					{
						newLines.append( line );
					}
				}
			}
//			else if ( oldLines.count() == newLines.count() )
//			{ // only changed i hope... could be as much as added as deleted in the same piece though...
//			}
//			else
			{ // added &| removed &| changed lines, figure it out...
				QStringList::ConstIterator oldIt = oldLines.begin();
				QStringList::ConstIterator newIt = newLines.begin();
				while( (oldIt != oldLines.end()) && (newIt != newLines.end()) )
				{
					if ( ((*oldIt).find( QRegExp( "^  " ), 0 ) == 0) && ((*newIt).find( QRegExp( "^  " ), 0 ) == 0) )
					{
						diff = new Difference( linenoA, linenoB );
						diff->setType( Difference::Unchanged );
						kdDebug() << "Type is: Unchanged" << endl;
						while( (oldIt != oldLines.end()) && (newIt != newLines.end()))
						{
							if ( ((*oldIt).find( QRegExp( "^  " ), 0 ) == 0) && ((*newIt).find( QRegExp( "^  " ), 0 ) == 0) )
							{
								QString line = (*oldIt);
								kdDebug() << "'^  ' Found in old: " << line << endl;
								diff->addSourceLine( line.replace( 0, 2, "" ) );
								line = (*newIt);
								kdDebug() << "'^  ' Found in new: " << line << endl;
								diff->addDestinationLine( line.replace( 0, 2, "" ) );
								linenoA++;
								linenoB++;
								++oldIt;
								++newIt;
							}
							else
								break; // leave this while
						}
						// don't add unchanged to differences
						hunk->add( diff );
					}
					else if ( ((*oldIt).find( QRegExp( "^! " ), 0 ) == 0) && (((*newIt).find( QRegExp( "^! " ), 0 ) == 0)) )
					{
						diff = new Difference( linenoA, linenoB );
						diff->setType( Difference::Change );
						kdDebug() << "Type is: Change" << endl;
						while ( (oldIt != oldLines.end()) && (*oldIt).find( QRegExp( "^! " ), 0 ) == 0 )
						{
							QString line = (*oldIt);
							kdDebug() << "'^! ' Found in old: " << line << endl;
							diff->addSourceLine( line.replace( 0, 2, "" ) );
							linenoA++;
							++oldIt;
						}
						while( (newIt != newLines.end()) && ((*newIt).find( QRegExp( "^! " ), 0 ) == 0) )
						{
							QString line = (*newIt);
							kdDebug() << "'^! ' Found in new: " << line << endl;
							diff->addDestinationLine( line.replace( 0, 2, "" ) );
							linenoB++;
							++newIt;
						}
						m_differences.append( diff );
						hunk->add( diff );
					}
					else if ( (*oldIt).find( QRegExp( "^- " ), 0 ) == 0 )
					{
						diff = new Difference( linenoA, linenoB );
						diff->setType( Difference::Delete );
						kdDebug() << "Type is: Delete" << endl;
						while( ( oldIt != oldLines.end() ) && ( (*oldIt).find( QRegExp( "^- " ), 0 ) == 0 ) )
						{
							QString line = (*oldIt);
							kdDebug() << "'^- ' Found in old: " << line << endl;
							diff->addSourceLine( line.replace( 0, 2, "" ) );
							linenoA++;
							++oldIt;
						}
						m_differences.append( diff );
						hunk->add( diff );
					}
					else if ( (*newIt).find( QRegExp( "^\\+ " ), 0 ) == 0 )
					{
						diff = new Difference( linenoA, linenoB );
						diff->setType( Difference::Insert );
						kdDebug() << "Type is: Insert" << endl;
						while( ( newIt != newLines.end() ) && ( (*newIt).find( QRegExp( "^\\+ " ), 0 ) == 0 ) )
						{
							QString line = (*newIt);
							kdDebug() << "'^+ ' Found in new: " << line << endl;
							diff->addDestinationLine( line.replace( 0, 2, "" ) );
							linenoB++;
							++newIt;
						}
						m_differences.append( diff );
						hunk->add( diff );
					}
					else
					{
						kdDebug() << "Some nasty error occurred in line: " << endl;
						kdDebug() << "Old line is: " << (*oldIt) << endl;
						kdDebug() << "New line is: " << (*newIt) << endl;
					}
					kdDebug() << "Next type" << endl;
				}
			}

			m_hunks.append( hunk );

			if ( it == list.end() )
			{
				kdDebug() << "End of diff found..." << endl;
				break;
			}
			else
			{
				kdDebug() << "New hunk separator found" << endl;
				++it;
				line = (*it);
			}
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
	QString line;
	int pos, len;
	int nolinesA, nolinesB;
	int linenoA, linenoB;
	int i;

	enum Difference::Type type;

	kdDebug() << "Normal  diff parsing:" << endl;

	// first attempt to get normal diff working

	if( it == list.end() ) return 0; // no differences
	line = (*it);
	kdDebug() << line << endl;

// needed for multifile diff
//	if ( line.find( QRegExp( "diff" ), 0 ) < 0 ) return 1;
//	line.replace( QRegExp( "diff" ) );

	while ( it != list.end() )
	{
		nolinesA = 0;
		nolinesB = 0;
		kdDebug() << line << endl;

		if ( ( pos = QRegExp( "^[0-9]+" ).match( line, 0, &len ) ) < 0 ) return 1;
		m_noOfHunks++;
		linenoA = line.mid( pos, len ).toInt();
		kdDebug() << "LinenoA: " << linenoA << endl;
		line.replace( QRegExp( "^[0-9]+" ), "" );

		if ( line.find( QRegExp( "^," ), 0 ) == 0 )
		{
			line.replace( QRegExp( "^," ), "" );
			kdDebug() << line << endl;
			if ( ( pos = QRegExp( "^[0-9]+" ).match( line, 0, &len ) ) < 0 ) return 1;
			nolinesA = line.mid( pos, len ).toInt() - linenoA + 1;
			kdDebug() << "NolinesA: " << nolinesA << endl;
			line.replace( QRegExp( "^[0-9]+" ), "" );
		}
		else
		{
			nolinesA = 1;
		}

		if ( line.find( QRegExp( "^a" ), 0 ) == 0 ) {
			type = Difference::Insert;
		} else if ( line.find( QRegExp( "^c" ), 0 ) == 0 ) {
			type = Difference::Change;
		} else if ( line.find( QRegExp( "^d" ), 0 ) == 0 ) {
			type = Difference::Delete;
		} else return 1;

		line.replace( QRegExp( "^[acd]" ), "" );

		if ( ( pos = QRegExp( "^[0-9]+" ).match( line, 0, &len ) ) < 0 ) return 1;
		linenoB = line.mid( pos, len ).toInt();
		kdDebug() << "LinenoB: " << linenoB << endl;
		line.replace( QRegExp( "^[0-9]+" ), "" );

		// Too bad... the number of hunks == the number of differences
		// so in this case it would unnecessary to have a hunk as well
		DiffHunk* hunk = new DiffHunk( linenoA, linenoB );
		m_hunks.append( hunk );

		hunk->lineStartA = linenoA;
		hunk->lineStartB = linenoB;

		Difference* diff = new Difference( linenoA, linenoB );
		hunk->add( diff );

		m_differences.append( diff );

		diff->setType( type );

		if ( line.find( QRegExp( "^," ), 0 ) == 0 )
		{
			line.replace( QRegExp( "^," ), "" );
			kdDebug() << line << endl;
			if ( ( pos = QRegExp( "^[0-9]+" ).match( line, 0, &len ) ) < 0 ) return 1;
			nolinesB = line.mid( pos, len ).toInt() - linenoB + 1;
			kdDebug() << "nolinesB: " << nolinesB << endl;
			line.replace( QRegExp( "^[0-9]+" ), "" );
			kdDebug() << line << endl;
		}
		else
		{
			nolinesB = 1;
		}

		kdDebug() << "nolinesA: " << nolinesA << endl;
		// no of lines is known now...
		if ( diff->type() != Difference::Insert )
			for ( i = 0; i < nolinesA; i++ )
			{
				++it;
				line = (*it);
				kdDebug() << "Aline " << i << ": " << line << endl;
				line.replace( QRegExp( "^< " ), "" ); // hope this works out ok...
				diff->addSourceLine( line );
			}

		if ( diff->type() == Difference::Change )
			++it; // this should be the --- line

		kdDebug() << "nolinesB: " << nolinesB << endl;
		if ( diff->type() != Difference::Delete )
			for ( i = 0; i < nolinesB; i++ )
			{
				++it;
				line = (*it);
				kdDebug() << "Bline " << i << ": " << line << endl;
				line.replace( QRegExp( "^> " ), "" ); // hope this works out ok...
				diff->addDestinationLine( line );
			}

		// TODO some checks to see if it worked
		// like comparing the count with the number of source and dest lines

		// next line
		++it;
		line = (*it);

	} // end of while
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
	QString line;
	int pos, len;
	int linenoA, linenoB;

	kdDebug() << "Unified  diff parsing:" << endl;

	if( it == list.end() ) return 0; // no differences
	line = (*it);
	kdDebug() << line << endl;
	if( line.find( QRegExp( "^--- " ), 0 ) < 0 ) return 1;
	line.replace( QRegExp( "^--- " ), "" );
	if ( ( pos = QRegExp( "^[^\\t]+" ).match( line, 0, &len ) ) < 0 ) return 1;
	KURL url;
	url.setPath( line.mid(pos, len) );
	m_sourceFile = url.fileName();
	kdDebug() << m_sourceFile << endl;
	line.replace( QRegExp( "^[^\\t]+\\t" ), "" );
	m_sourceTimestamp = line;
	kdDebug() << m_sourceTimestamp << endl;

	if( ++it == list.end() ) return 1;
	line = (*it);
	if( line.find( QRegExp( "^\\+\\+\\+ " ), 0 ) < 0 ) return 1;
	line.replace( QRegExp( "^\\+\\+\\+ " ), "" );
	if ( ( pos = QRegExp( "^[^\\t]+" ).match( line, 0, &len ) ) < 0 ) return 1;
	url.setPath( line.mid(pos, len) );
	m_destinationFile = url.fileName();
	kdDebug() << m_destinationFile << endl;
	line.replace( QRegExp( "^[^\\t]+\\t" ), "" );
	m_destinationTimestamp = line;
	kdDebug() << m_destinationTimestamp << endl;

	++it;
	while( it != list.end() )
	{
		line = (*it);

		if ( line.find( QRegExp( "^@@ -" ), 0 ) < 0 ) return 1;
		m_noOfHunks++;

		// strip off the begin of the hunk header
		line.replace( QRegExp( "^@@ -" ), "" );

		// get the start line number for first file
		if ( ( pos = QRegExp( "^[0-9]+" ).match( line, 0, &len ) ) < 0 ) return 1;
		linenoA = line.mid(pos, len).toInt();
		line.replace( QRegExp( "^[0-9]+," ), "" );

		// length for first file
		if ( ( pos = QRegExp( "^[0-9]+" ).match( line, 0, &len ) ) < 0 ) return 1;
		line.replace( QRegExp( "^[0-9]+ \\+" ), "" );

		// start line for second file
		if ( ( pos = QRegExp( "^[0-9]+" ).match( line, 0, &len ) ) < 0 ) return 1;
		linenoB = line.mid(pos, len).toInt();
		line.replace( QRegExp( "^[0-9]+," ), "" );

		if ( ( pos = QRegExp( "^[0-9]+" ).match( line, 0, &len ) ) < 0 ) return 1;

		DiffHunk* hunk = new DiffHunk( linenoA, linenoB );
		m_hunks.append( hunk );

		++it;
		while( it != list.end() && ((*it).find( QRegExp( "^[ \\-+]" ), 0 ) == 0) )
		{
			// read differences until we come to a new hunk
			line = (*it);

			Difference* diff = new Difference( linenoA, linenoB );
			hunk->add( diff );

			if( line.find( QRegExp( "^ " ), 0 ) == 0 ) {
				diff->setType( Difference::Unchanged );
				for( ; it != list.end() && ((*it).find( QRegExp( "^ " ), 0 ) == 0); ++it ) {
					line = (*it);
					line.replace( 0, 1, "" );
					diff->addSourceLine( line );
					diff->addDestinationLine( line );
					linenoA++;
					linenoB++;
				}
			} else if ( line.find( QRegExp( "^[-+]" ), 0 ) == 0 ) {
				for( ; it != list.end() && ((*it).find( QRegExp( "^-" ), 0 ) == 0); ++it )
				{
					line = (*it);
					line.replace( 0, 1, "" );
					diff->addSourceLine( line );
					linenoA++;
				}
				for( ; it != list.end() && ((*it).find( QRegExp( "^\\+" ), 0 ) == 0); ++it )
				{
					line = (*it);
					line.replace( 0, 1, "" );
					diff->addDestinationLine( line );
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

void DiffModel::toggleApplied( int diffIndex )
{
	Difference* d = m_differences.at( diffIndex );
	d->toggleApplied();
	emit appliedChanged( d );
}

#include "diffmodel.moc"
