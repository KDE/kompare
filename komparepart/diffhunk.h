/***************************************************************************
                                diffhunk.h  -  description
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

#ifndef DIFFHUNK_H
#define DIFFHUNK_H

#include <qlist.h>

class Difference;

class DiffHunk
{
public:
	DiffHunk( int sourceLine, int destinationLine, QString function = QString::null );
	~DiffHunk();

	const QList<Difference>& getDifferences() const { return m_differences; };
	const QString getFunction() const { return m_function; };
	
	void add( Difference* diff );
	
private:
	int               m_sourceLine;
	int               m_destinationLine;
	QList<Difference> m_differences;
	QString           m_function;
};

#endif
