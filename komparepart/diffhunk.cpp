/***************************************************************************
                                diffhunk.cpp  -  description
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

#include <klocale.h>

#include "diffhunk.h"
#include "difference.h"

DiffHunk::DiffHunk()
{
}

DiffHunk::DiffHunk( int lineStartA, int lineStartB )
{
	this->lineStartA = lineStartA;
	this->lineStartB = lineStartB;
}

DiffHunk::~DiffHunk()
{
}

QString DiffHunk::asString() const
{
	return i18n("Source line %1, Destination line %2").arg(lineStartA).arg(lineStartB);
}

void DiffHunk::add( Difference* diff )
{
	differences.append( diff );
}
