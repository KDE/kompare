/***************************************************************************
                                diffhunk.cpp  -  description
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


#include "difference.h"
#include "diffhunk.h"

using namespace Diff2;

DiffHunk::DiffHunk( int sourceLine, int destinationLine, QString function ) :
	m_sourceLine( sourceLine ),
	m_destinationLine( destinationLine ),
	m_function( function )
{
}

DiffHunk::~DiffHunk()
{
}

void DiffHunk::add( Difference* diff )
{
	m_differences.append( diff );
}

QString DiffHunk::recreateHunk() const
{
	QString hunk;

	// recreate header
	hunk += QString::fromLatin1( "@@ -%1,%2 +%3,%4 @@" ).arg( m_sourceLine ).arg("").arg( m_destinationLine ).arg("");
	if ( !m_function.isEmpty() )
		hunk += m_function;
	hunk += QString::fromLatin1( "\n" );
	// recreate body
	QValueListConstIterator<Difference*> diffIt = m_differences.begin();
	QValueListConstIterator<Difference*> dEnd   = m_differences.end();

	for ( ; diffIt != dEnd; ++diffIt )
	{
		hunk += (*diffIt)->recreateDifference();
	}

	return hunk;
}
