/***************************************************************************
                                difference.cpp  -  description
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

#include "difference.h"

Difference::Difference( int sourceLineNo, int destinationLineNo ) :
	m_type( Unchanged ),
	m_sourceLineNo( sourceLineNo ),
	m_destinationLineNo( destinationLineNo ),
	m_applied( false )
{
};

Difference::~Difference()
{
};

void Difference::addSourceLine( QString line )
{
	m_sourceLines.append( line );
}

void Difference::addDestinationLine( QString line )
{
	m_destinationLines.append( line );
}

int Difference::sourceLineCount() const
{
	return m_sourceLines.count();
}

int Difference::destinationLineCount() const
{
	return m_destinationLines.count();
}

void Difference::apply( bool apply )
{
	m_applied = apply;
}

QString Difference::asString() const
{
	int lineCountA = sourceLineCount();
	int lineCountB = destinationLineCount();
	int lineEndA = m_sourceLineNo+lineCountA-1;
	int lineEndB = m_destinationLineNo+lineCountB-1;
	QString res;
	if (lineCountB == 0)
		res = QString("%1,%2d%3").arg(m_sourceLineNo).arg(lineEndA).arg(m_destinationLineNo-1);
	else if (lineCountA == 0)
		res = QString("%1a%2,%3").arg(m_sourceLineNo-1).arg(m_destinationLineNo).arg(lineEndB);
	else if (m_sourceLineNo == lineEndA)
		if (m_destinationLineNo == lineEndB)
			res = QString("%1c%2").arg(m_sourceLineNo).arg(m_destinationLineNo);
		else
			res = QString("%1c%2,%3").arg(m_sourceLineNo).arg(m_destinationLineNo).arg(lineEndB);
	else if (m_destinationLineNo == lineEndB)
		res = QString("%1,%2c%3").arg(m_sourceLineNo).arg(lineEndA).arg(m_destinationLineNo);
	else
		res = QString("%1,%2c%3,%4").arg(m_sourceLineNo).arg(lineEndA).arg(m_destinationLineNo).arg(lineEndB);

	return res;
}
