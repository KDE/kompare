
#include <qtextstream.h>
#include <kdebug.h>
#include <klocale.h>

#include "difference.h"
#include "diffmodel.h"
#include "diffhunk.h"

/**  */
DiffModel::DiffModel()
{
	sourceFilename = i18n( "Source" );
	destinationFilename = i18n( "Destination" );
	sourceTimestamp = "";
	destinationTimestamp = "";
};

/**  */
DiffModel::~DiffModel()
{
};

/**  */
int DiffModel::parseDiff( const QStringList& list )
{
	DiffFormat format;
	if( list.count() == 0 ) return 1;
	QString line = list[0];
	kdDebug() << "Determining format from this line: " << line << endl;
	if( line.find( QRegExp( "^[0-9]+[0-9,]*[acd][0-9]+[0-9,]*$" ), 0 ) == 0 )
		format = Normal;
	else if( line.find( QRegExp( "^--- " ), 0 ) == 0 )
		format = Unified;
	else if( line.find( QRegExp( "^\\*\\*\\* " ), 0 ) == 0 )
		format = Context;
	else format = Unknown;

	return parseDiff( list, format );
};

int DiffModel::parseDiff( const QStringList& lines, enum DiffFormat format )
{
	switch( format )
	{
		case Context: return parseContextDiff( lines );
		case Ed:      return parseEdDiff(      lines );
		case Normal:  return parseNormalDiff(  lines );
		case RCS:     return parseRCSDiff(     lines );
		case Unified: return parseUnifiedDiff( lines );
		default:      return 1;
	}
}

/**  */
int DiffModel::parseContextDiff( const QStringList& list )
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
	int i;

	enum Difference::Type type;

	kdDebug() << "Context diff parsing:" << endl;

	QStringList::ConstIterator it = list.begin();

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
	filenameA = line.mid( 0, pos );
	kdDebug() << "filenameA: " << filenameA << endl;
	// get date if necessary

	++it;
	line = (*it);
	kdDebug() << line << endl;

	// Figure out the filename and date of new file
	if ( ( pos = QRegExp( "^---" ).match( line, 0, &len ) ) < 0 ) return 1; // invalid context format
	line.replace( QRegExp( "^--- " ), "" ); // remove leading '--- '
	// read name of new file until \t
	if ( ( pos = QRegExp( "\\t" ).match( line, 0, &len ) ) < 0 ) return 1; // invalid context format
	filenameB = line.mid( 0, pos );
	kdDebug() << "filenameB: " << filenameB << endl;
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
				diff = new Difference( linenoA, linenoB );
				diff->type = Difference::Insert;
				for( QStringList::ConstIterator newIt = newLines.begin(); newIt != newLines.end(); ++newIt)
				{
					QString line = (*newIt);
					if ( !line.find( QRegExp( "^\\+ " ), 0 ) == 0 )
					{
						diff->addSourceLine( line.replace( 0, 2, "" ) );
						diff->addDestinationLine( line );
					}
					else
					{
						diff->addDestinationLine( line.replace( 0, 2, "" ) );
					}
				}
				differences.append( diff );
				hunk->add( diff );
			}
			else if ( newLines.count() == 0 )
			{ // Only removed lines -> make difference from oldLines and type Delete
				diff = new Difference( linenoA, linenoB );
				diff->type = Difference::Delete;
				for( QStringList::ConstIterator oldIt = oldLines.begin(); oldIt != oldLines.end(); ++oldIt)
				{
					QString line = (*oldIt);
					if ( !line.find( QRegExp( "^- " ), 0 ) == 0 )
					{
						diff->addSourceLine( line.replace( 0, 2, "" ) );
						diff->addDestinationLine( line );
					}
					else
					{
						diff->addSourceLine( line.replace( 0, 2, "" ) );
					}
				}
				differences.append( diff );
				hunk->add( diff );
			}
//			else if ( oldLines.count() == newLines.count() )
//			{ // only changed i hope... could be as much as added as deleted in the same piece though...
//			}
			else
			{ // added &| removed &| changed lines, figure it out...
				QStringList::ConstIterator oldIt = oldLines.begin();
				QStringList::ConstIterator newIt = newLines.begin();
				while( (oldIt != oldLines.end()) && (newIt != newLines.end()) )
				{
					if ( ((*oldIt).find( QRegExp( "^  " ), 0 ) == 0) && ((*newIt).find( QRegExp( "^  " ), 0 ) == 0) )
					{
						diff = new Difference( linenoA, linenoB );
						diff->type = Difference::Unchanged;
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
						differences.append( diff );
						hunk->add( diff );
					}
					else if ( ((*oldIt).find( QRegExp( "^! " ), 0 ) == 0) && (((*newIt).find( QRegExp( "^! " ), 0 ) == 0)) )
					{
						diff = new Difference( linenoA, linenoB );
						diff->type = Difference::Change;
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
						differences.append( diff );
						hunk->add( diff );
					}
					else if ( (*oldIt).find( QRegExp( "^- " ), 0 ) == 0 )
					{
						diff = new Difference( linenoA, linenoB );
						diff->type = Difference::Delete;
						kdDebug() << "Type is: Delete" << endl;
						while( ( oldIt != oldLines.end() ) && ( (*oldIt).find( QRegExp( "^- " ), 0 ) == 0 ) )
						{
							QString line = (*oldIt);
							kdDebug() << "'^- ' Found in old: " << line << endl;
							diff->addSourceLine( line.replace( 0, 2, "" ) );
							linenoA++;
							++oldIt;
						}
						differences.append( diff );
						hunk->add( diff );
					}
					else if ( (*newIt).find( QRegExp( "^\\+ " ), 0 ) == 0 )
					{
						diff = new Difference( linenoA, linenoB );
						diff->type = Difference::Insert;
						kdDebug() << "Type is: Insert" << endl;
						while( ( newIt != newLines.end() ) && ( (*newIt).find( QRegExp( "^\\+ " ), 0 ) == 0 ) )
						{
							QString line = (*newIt);
							kdDebug() << "'^+ ' Found in new: " << line << endl;
							diff->addDestinationLine( line.replace( 0, 2, "" ) );
							linenoB++;
							++newIt;
						}
						differences.append( diff );
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

			hunks.append( hunk );

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
int DiffModel::parseEdDiff( const QStringList& /*list*/ )
{
	kdDebug() << "Ed diff parsing:" << endl;

	return 0;
};

/**  */
int DiffModel::parseNormalDiff( const QStringList& list )
{
	QString line;
	int pos, len;
	int nolinesA, nolinesB;
	int linenoA, linenoB;
	int i;

	enum Difference::Type type;

	kdDebug() << "Normal  diff parsing:" << endl;

	// first attempt to get normal diff working
	QStringList::ConstIterator it = list.begin();

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
		hunks.append( hunk );

		hunk->lineStartA = linenoA;
		hunk->lineStartB = linenoB;

		Difference* diff = new Difference( linenoA, linenoB );
		hunk->add( diff );

		differences.append( diff );

		diff->type = type;

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
		if ( diff->type != Difference::Insert )
			for ( i = 0; i < nolinesA; i++ )
			{
				++it;
				line = (*it);
				kdDebug() << "Aline " << i << ": " << line << endl;
				line.replace( QRegExp( "^< " ), "" ); // hope this works out ok...
				diff->addSourceLine( line );
			}

		if ( diff->type == Difference::Change )
			++it; // this should be the --- line

		kdDebug() << "nolinesB: " << nolinesB << endl;
		if ( diff->type != Difference::Delete )
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
int DiffModel::parseRCSDiff( const QStringList& /*list*/ )
{
	kdDebug() << "RCS  diff parsing:" << endl;

	return 0;
};

/**  */
int DiffModel::parseUnifiedDiff( const QStringList& list )
{
	QString line;
	int pos, len;
	int linenoA, linenoB;

	kdDebug() << "Unified  diff parsing:" << endl;

	QStringList::ConstIterator it = list.begin();

	if( it == list.end() ) return 0; // no differences
	line = (*it);
	kdDebug() << line << endl;
	if( line.find( QRegExp( "^--- " ), 0 ) < 0 ) return 1;
	line.replace( QRegExp( "^--- " ), "" );
	if ( ( pos = QRegExp( "^[^\\t]+" ).match( line, 0, &len ) ) < 0 ) return 1;
	sourceFilename = line.mid(pos, len);
	kdDebug() << sourceFilename << endl;
	line.replace( QRegExp( "^[^\\t]+\\t" ), "" );
	sourceTimestamp = line;
	kdDebug() << sourceTimestamp << endl;

	if( ++it == list.end() ) return 1;
	line = (*it);
	if( line.find( QRegExp( "^\\+\\+\\+ " ), 0 ) < 0 ) return 1;
	line.replace( QRegExp( "^\\+\\+\\+ " ), "" );
	if ( ( pos = QRegExp( "^[^\\t]+" ).match( line, 0, &len ) ) < 0 ) return 1;
	destinationFilename = line.mid(pos, len);
	kdDebug() << destinationFilename << endl;
	line.replace( QRegExp( "^[^\\t]+\\t" ), "" );
	destinationTimestamp = line;
	kdDebug() << destinationTimestamp << endl;

	++it;
	while( it != list.end() )
	{
		line = (*it);

		if ( line.find( QRegExp( "^@@ -" ), 0 ) < 0 ) return 1;

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
		hunks.append( hunk );

		++it;
		while( it != list.end() && ((*it).find( QRegExp( "^[ \\-+]" ), 0 ) == 0) )
		{
			// read differences until we come to a new hunk
			line = (*it);

			Difference* diff = new Difference( linenoA, linenoB );
			hunk->add( diff );

			if( line.find( QRegExp( "^ " ), 0 ) == 0 ) {
				diff->type = Difference::Unchanged;
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
				if      ( diff->sourceLineCount() == 0 )      diff->type = Difference::Insert;
				else if ( diff->destinationLineCount() == 0 ) diff->type = Difference::Delete;
				else                                          diff->type = Difference::Change;
				differences.append( diff );
			}
		}
	}
	return 0;
}

#include "diffmodel.moc"
