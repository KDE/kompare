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

Difference::Difference( int sourceLineNo, int destinationLineNo )
	: m_type( Unchanged ),
	m_sourceLineNo( sourceLineNo ),
	m_destinationLineNo( destinationLineNo )
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

QString Difference::asString() const
{
	int linecountA = sourceLineCount();
	int linecountB = destinationLineCount();
	int lineendA = m_sourceLineNo+linecountA-1;
	int lineendB = m_destinationLineNo+linecountB-1;
	QString res;
	if (linecountB == 0)
		res = QString("%1,%2d%3").arg(m_sourceLineNo).arg(lineendA).arg(m_destinationLineNo-1);
	else if (linecountA == 0)
		res = QString("%1a%2,%3").arg(m_sourceLineNo-1).arg(m_destinationLineNo).arg(lineendB);
	else if (m_sourceLineNo == lineendA)
		if (m_destinationLineNo == lineendB)
			res = QString("%1c%2").arg(m_sourceLineNo).arg(m_destinationLineNo);
		else
			res = QString("%1c%2,%3").arg(m_sourceLineNo).arg(m_destinationLineNo).arg(lineendB);
	else if (m_destinationLineNo == lineendB)
		res = QString("%1,%2c%3").arg(m_sourceLineNo).arg(lineendA).arg(m_destinationLineNo);
	else
		res = QString("%1,%2c%3,%4").arg(m_sourceLineNo).arg(lineendA).arg(m_destinationLineNo).arg(lineendB);

	return res;
}

#include "difference.moc"

