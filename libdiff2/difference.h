/***************************************************************************
                                difference.h  -  description
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

#ifndef DIFFERENCE_H
#define DIFFERENCE_H

#include <qptrlist.h>
#include <qstringlist.h>

#include <kdebug.h>

class QString;

namespace Diff2
{

class Command
{
public:
	enum Type { Start = 0, End = 1 };

public:
	Command()
	{ m_type = Command::Start; m_offset = 0; }
	Command( enum Command::Type type, unsigned int offset )
	{ m_type = type; m_offset = offset; }
	~Command() {}

public:
	enum Command::Type type()   const { return m_type;   }
	unsigned int       offset() const { return m_offset; }

	void setType  ( enum Command::Type type ) { m_type   = type;   }
	void setOffset( unsigned int offset )     { m_offset = offset; }

public:
	enum Command::Type m_type;
	unsigned int       m_offset;
};

class DifferenceString
{
public:
	DifferenceString()
	{
//		kdDebug(8101) << "DifferenceString::DifferenceString()" << endl;
		m_string = QString::null;
		m_commandsList = QValueList<Command*>();
	}
	DifferenceString( const QString& string, const QValueList<Command*>& commandsList = QValueList<Command*>() )
	{
//		kdDebug(8101) << "DifferenceString::DifferenceString( " << string << ", " << commandsList << " )" << endl;
		m_string = string;
		m_commandsList = commandsList;
	}
	~DifferenceString() {}

public:
	const QString& string() const
	{
		return m_string;
	}
	// Cant be const unfortunately
	QValueList<Command*> commandsList()
	{
		return m_commandsList;
	}
	void setString( const QString& string )
	{
		m_string = string;
	};
	void setCommandsList( const QValueList<Command*>& commandsList )
	{
		m_commandsList = commandsList;
	}

private:
	QValueList<Command*> m_commandsList;
	QString m_string;
};

class LevenshteinTable;

typedef QValueList<DifferenceString*> DifferenceStringList;
typedef QValueListIterator<DifferenceString*> DifferenceStringListIterator;
typedef QValueListConstIterator<DifferenceString*> DifferenceStringListConstIterator;

class Difference
{
public:
	enum Type { Change, Insert, Delete, Unchanged };

public:
	Difference( int sourceLineNo, int destinationLineNo, enum Difference::Type type = Difference::Unchanged );
	~Difference();

public:
	enum Difference::Type type() const { return m_type; };

	int sourceLineNumber() const { return m_sourceLineNo; }
	int destinationLineNumber() const { return m_destinationLineNo; }

	int sourceLineCount() const;
	int destinationLineCount() const;

	DifferenceString* sourceLineAt( int i ) { return m_sourceLines[ i ]; }
	DifferenceString* destinationLineAt( int i ) { return m_destinationLines[ i ]; }

	const DifferenceStringList sourceLines() const { return m_sourceLines; }
	const DifferenceStringList destinationLines() const { return m_destinationLines; }

	void apply( bool apply );
	bool applied() const { return m_applied; }

	void setType( enum Difference::Type type ) { m_type = type; }

	void addSourceLine( QString line );
	void addDestinationLine( QString line );

	/** This method will calculate the differences between the individual strings and store them as Commands */
	void determineInlineDifferences();

	QString recreateDifference() const;

private:
	enum Difference::Type m_type;

	int                   m_sourceLineNo;
	int                   m_destinationLineNo;

	DifferenceStringList  m_sourceLines;
	DifferenceStringList  m_destinationLines;

	bool                  m_applied;

	LevenshteinTable*     m_table;
};

} // End of namespace Diff2

#endif

