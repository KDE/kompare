
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
DiffModel::DiffFormat DiffModel::determineDiffFormat( const QStringList& list )
{
	if( list.count() == 0 ) return Unknown;
	QString line = list[0];
	kdDebug() << line << endl;
	if( line.find( QRegExp( "^[0-9]+[0-9,]*[acd][0-9]+[0-9,]*$" ), 0 ) == 0 )
		return Normal;
	if( line.find( QRegExp( "^--- " ), 0 ) == 0 )
		return Unified;
	if( line.find( QRegExp( "^\\*\\*\\* " ), 0 ) == 0 )
		return Context;
	return Unknown;
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
int DiffModel::parseDiffFile( QFile& file, QList<DiffModel>& models )
{
	if (file.open(IO_ReadOnly) == false)
		return 1;

	QTextStream stream(&file);
	QStringList lines;
	while (!stream.eof())
		lines.append(stream.readLine());

	file.close();

	DiffModel* model = new DiffModel(); // TODO: handle multi-file diff
	models.append( model );

	return model->parseDiff( lines, determineDiffFormat( lines ) );
}

/**  */
int DiffModel::parseContextDiff( const QStringList& list )
{
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
	kdDebug() << "filenameB: " << filenameA << endl;
	// get date if necessary

	// Start processing the hunks in the diff
	while ( it != list.end() )
	{
		++it;
		line = (*it);
		kdDebug() << line << endl;
		// actual start of the hunks
		if ( ( pos = QRegExp( "^\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*\\*" ).match( line, 0, &len ) ) < 0 ) return 1; // weirdness has happened
		// ok we found a hunk...
		// create DiffHunk
		DiffHunk* hunk = new DiffHunk();
		hunks.append( hunk );
		while ( it != list.end() )
		{
			// ok old looks like '*** number,number ****'
			// WARNING: after old new can come immediately to indicate only added lines
			if ( ( pos = QRegExp( "^\\*\\*\\* " ).match( line, 0, &len ) ) < 0 ) return 1;
			line.replace( QRegExp( "^\\*\\*\\* "), "" );
			// get starting line of old file
			if ( ( pos = QRegExp( "^[0-9]+" ).match( line, 0, &len ) ) < 0 ) return 1;
			linenoA = hunk->lineStartA = line.mid( pos, len ).toInt();
			kdDebug() << "LinenoA: " << linenoA << endl;
			line.replace( QRegExp( "^[0-9]+,"), "" );
			if ( ( pos = QRegExp( "^[0-9]+" ).match( line, 0, &len ) ) < 0 ) return 1;
			nolinesA = line.mid( pos, len ).toInt() - linenoA + 1;
			kdDebug() << "NolinesA: " << nolinesA << endl;

			++it;
			line = (*it);
			kdDebug() << line << endl;

			while ( QRegExp( "^--- " ).match( line, 0, &len ) < 0 )
			{
				// Assume it's still a valid diff file
				if ( line.find( QRegExp( "^+ " ), 0 ) == 0 )
				{
				}
				++it;
				line = (*it);
				kdDebug() << line << endl;
			}

			// ok new looks like '--- number,number ----'
			// WARNING: after new a new hunk can come immediately to indicate only removed lines
			if ( ( pos = QRegExp( "^--- " ).match( line, 0, &len ) ) < 0 ) return 1;
			line.replace( QRegExp( "^--- "), "" );
			// get starting line of old file
			if ( ( pos = QRegExp( "^[0-9]+" ).match( line, 0, &len ) ) < 0 ) return 1;
			linenoB = hunk->lineStartB = line.mid( pos, len ).toInt();
			kdDebug() << "LinenoB: " << linenoB << endl;
			line.replace( QRegExp( "^[0-9]+,"), "" );
			if ( ( pos = QRegExp( "^[0-9]+" ).match( line, 0, &len ) ) < 0 ) return 1;
			nolinesB = line.mid( pos, len ).toInt() - linenoB + 1;
			kdDebug() << "NolinesB: " << nolinesB << endl;

		}
	}
	return 0;
};

/**  */
int DiffModel::parseEdDiff( const QStringList& /*list*/ )
{
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
	return 0;
};

/**  */
int DiffModel::parseUnifiedDiff( const QStringList& list )
{
	QString line;
	int pos, len;
	int linenoA, linenoB;

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
};

#include "diffmodel.moc"
