/***************************************************************************
                                diffhunk.h  -  description
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

#ifndef DIFFHUNK_H
#define DIFFHUNK_H

#include <qptrlist.h>

namespace Diff2
{

class Difference;

class DiffHunk
{
public:
	DiffHunk( int sourceLine, int destinationLine, QString function = QString::null );
	~DiffHunk();

	const QPtrList<Difference>& differences() const { return m_differences; };
	const QString function() const { return m_function; };

	const int sourceLineNumber() const      { return m_sourceLine; };
	const int destinationLineNumber() const { return m_destinationLine; };

	void add( Difference* diff );

private:
	int               m_sourceLine;
	int               m_destinationLine;
	QPtrList<Difference> m_differences;
	QString           m_function;
};

} // End of namespace Diff2

#endif
