/***************************************************************************
                                difference.cpp  -  description
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

using namespace Diff2;

Difference::Difference( int sourceLineNo, int destinationLineNo, enum Difference::Type type ) :
	m_type( type ),
	m_sourceLineNo( sourceLineNo ),
	m_destinationLineNo( destinationLineNo ),
	m_applied( false )
{
}

Difference::~Difference()
{
}

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

