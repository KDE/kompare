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
	// We need a constructor without arguments for contextdiff
	DiffHunk();
	// This constructor can be used for the other formats
	DiffHunk( int lineStartA, int lineStartB );
	~DiffHunk();

	QString asString() const;
	const QList<Difference>& getDifferences() const { return differences; };
	const QString getFunction() const { return function; };
	
	void add( Difference* diff );
	
	int lineStartA;
	int lineStartB;

private:
	QList<Difference> differences;
	QString           function;
};

#endif
