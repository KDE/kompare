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
#include "levenshteintable.h"

using namespace Diff2;

Difference::Difference( int sourceLineNo, int destinationLineNo, enum Difference::Type type ) :
	m_type( type ),
	m_sourceLineNo( sourceLineNo ),
	m_destinationLineNo( destinationLineNo ),
	m_applied( false ),
	m_table( new LevenshteinTable() )
{
}

Difference::~Difference()
{
}

void Difference::addSourceLine( QString line )
{
	m_sourceLines.append( *( new DifferenceString( line ) ) );
}

void Difference::addDestinationLine( QString line )
{
	m_destinationLines.append( *( new DifferenceString( line ) ) );
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

void Difference::determineInlineDifferences()
{
	if ( m_type != Difference::Change )
		return;

	// Do nothing for now when the slc != dlc
	// One could try to find the closest matching destination string for any
	// of the source strings but this is compute intensive
	if ( sourceLineCount() != destinationLineCount() )
		return;

	int slc = sourceLineCount();

	for ( int i = 0; i < slc; ++i )
	{
		DifferenceString* sl = sourceLineAt( i );
		DifferenceString* dl = destinationLineAt( i );

		// FIXME: If the table cant be created dont do the rest
		m_table->createTable( sl, dl );

		m_table->createListsOfCommands();
	}

	// No longer needed, if we ever need to recalculate the inline differences we should
	// simply recreate the table
	delete m_table;
}

