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

#include <qstringlist.h>

class QString;

namespace Diff2
{

class Difference
{
public:
	enum Type { Change, Insert, Delete, Unchanged };

public:
	Difference( int sourceLineNo, int destinationLineNo, enum Difference::Type type = Difference::Unchanged );
	~Difference();

public:
	enum Difference::Type type() const { return m_type; };

	int sourceLineNumber() const { return m_sourceLineNo; };
	int destinationLineNumber() const { return m_destinationLineNo; };

	int sourceLineCount() const;
	int destinationLineCount() const;

	const QString& sourceLineAt( int i ) const { return m_sourceLines[ i ]; };
	const QString& destinationLineAt( int i ) const { return m_destinationLines[ i ]; };

	const QStringList sourceLines() const { return m_sourceLines; };
	const QStringList destinationLines() const { return m_destinationLines; };

	void apply( bool apply );
	bool applied() const { return m_applied; };

	void setType( enum Difference::Type type ) { m_type = type; };

	void addSourceLine( QString line );
	void addDestinationLine( QString line );

private:
	enum Difference::Type m_type;

	int                   m_sourceLineNo;
	int                   m_destinationLineNo;

	QStringList           m_sourceLines;
	QStringList           m_destinationLines;

	bool                  m_applied;
};

} // End of namespace Diff2

#endif

