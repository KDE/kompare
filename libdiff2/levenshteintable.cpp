/*******************************************************************************
**
** Filename   : levenshteintable.cpp
** Created on : 08 november, 2003
** Copyright  : (c) 2003 Otto Bruggeman
** Email      : bruggie@home.nl
**
*******************************************************************************/

/*******************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
*******************************************************************************/

#if INLINE_DIFFERENCES

#include <iostream>

#include <qstring.h>

#include <kdebug.h>
#include <kglobal.h>

#include "levenshteintable.h"

#include "difference.h"

using namespace Diff2;

LevenshteinTable::LevenshteinTable()
  : m_width( 256 ),
    m_height( 256 ),
    m_size( m_height * m_width ),
    m_table( new unsigned int[ m_size ] ),
    m_source( 0 ),
    m_destination( 0 )
{
}

LevenshteinTable::LevenshteinTable( unsigned int width, unsigned int height )
  : m_width( width ),
    m_height( height ),
    m_size( m_width * m_height ),
    m_table( new unsigned int[ m_size ] ),
    m_source( 0 ),
    m_destination( 0 )
{
}

LevenshteinTable::~LevenshteinTable()
{
	delete[] m_table;
	m_source = 0;
	m_destination = 0;
}

int LevenshteinTable::getContent( unsigned int posX, unsigned int posY ) const
{
//	kdDebug(8101) << "Width = " << m_width << ", height = " << m_height << ", posX = " << posX << ", posY = " << posY << endl;
	return m_table[ posY * m_width + posX ];
}

int LevenshteinTable::setContent( unsigned int posX, unsigned int posY, int value )
{
	m_table[ posY * m_width + posX ] = value;

	return 0;
}

bool LevenshteinTable::setSize( unsigned int width, unsigned int height )
{
	// Set a limit of 16.7 million entries, will be about 64 MB of ram, that should be plenty
	if ( ( ( width ) * ( height ) ) > ( 256 * 256 * 256 ) )
		return false;

	if ( ( ( width ) * ( height ) ) > m_size )
	{
		delete[] m_table;

		m_size = width * height;
		m_table = new unsigned int[ m_size ];
	}

	m_width = width;
	m_height = height;

	return true;
}

void LevenshteinTable::dumpLevenshteinTable()
{
	for ( unsigned int i = 0; i < m_height; ++i )
	{
		for ( unsigned int j = 0; j < m_width; ++j )
		{
			std::cout.width( 3 );
			std::cout << getContent( j, i );
		}
		std::cout << std::endl;
	}
}

unsigned int LevenshteinTable::createTable( DifferenceString* source, DifferenceString* destination )
{
	m_source      = source;
	m_destination = destination;

	QString s = ' ' + source->string();      // Optimization, so i dont have to subtract 1 from the indexes every single time
	QString d = ' ' + destination->string(); // and add 1 to the width and height of the table

	unsigned int m = s.length();
	unsigned int n = d.length();

	if ( m == 1 )
		return --n;

	if ( n == 1 )
		return --m;

	setSize( m, n );

	unsigned int i;
	unsigned int j;

	// initialize first row
	for ( i = 0; i < m; ++i )
		setContent( i, 0, i );
	// initialize first column
	for ( j = 0; j < n; ++j )
		setContent( 0, j, j );

	int cost = 0, north = 0, west = 0, northwest = 0;

	QChar si, dj;
	// Optimization, calculate row wise instead of column wise, wont trash the cache so much with large strings
	for ( j = 1; j < n; ++j )
	{
		dj = d[ j ];

		for ( i = 1; i < m; ++i )
		{
			si = s[ i ];
			if ( si == dj )
				cost = 0;
			else
				cost = 1;

			north     = getContent( i, j-1 ) + 1;
			west      = getContent( i-1, j ) + 1;
			northwest = getContent( i-1, j-1 ) + cost;

			setContent( i, j, kMin( north, kMin( west, northwest ) ) );
		}
	}

	return getContent( m-1, n-1 );
}

int LevenshteinTable::chooseRoute( int c1, int c2, int c3 )
{
//	kdDebug(8101) << "c1 = " << c1 << ", c2 = " << c2 << ", c3 = " << c3 << endl;
	// preference order: c2, c3, c1, hopefully this will work out for me
	if ( c2 <= c1 && c2 <= c3 )
		return 1;

	if ( c3 <= c2 && c3 <= c1 )
		return 2;

	return 0;
}

void LevenshteinTable::createListsOfCommands()
{
//	std::cout << source.latin1() << std::endl;
//	std::cout << destination.latin1() << std::endl;
//	dumpLevenshteinTable();

	unsigned int x = m_width-1;
	unsigned int y = m_height-1;

	Diff2::Command* c;

	int n, nw, w, direction, currentValue;
	while ( x > 0 && y > 0 )
	{
		currentValue = getContent( x, y );

		nw = getContent( x - 1, y - 1 );
		n  = getContent( x, y - 1 );
		w  = getContent( x - 1, y );

		direction = chooseRoute( n, nw, w );

		switch ( direction )
		{
		case 0: // north
//			kdDebug(8101) << "Picking north" << endl;
//			kdDebug(8101) << "Source[" << ( x - 1 ) << "] = " << QString( source[ x-1 ] ) << ", destination[" << ( y - 1 ) << "] = " << QString( destination[ y-1 ] ) << endl;

			c = m_destination->commandsList()->first();
			if ( c && c->m_type == Command::End )
			{
//				kdDebug(8101) << "CurrentValue: " << currentValue << endl;
				if ( n == currentValue )
					m_destination->commandsList()->prepend( new Command( Command::Start, y ) );
				// else: the change continues, dont do anything
			}
			else
			{
//				kdDebug(8101) << "CurrentValue: " << currentValue << endl;
				if ( n < currentValue )
					m_destination->commandsList()->prepend( new Command( Command::End, y ) );
			}

			--y;
			break;
		case 1: // northwest
//			kdDebug(8101) << "Picking northwest" << endl;
//			kdDebug(8101) << "Source[" << ( x - 1 ) << "] = " << QString( source[ x-1 ] ) << ", destination[" << ( y - 1 ) << "] = " << QString( destination[ y-1 ] ) << endl;

			c = m_destination->commandsList()->first();
			if ( c && c->m_type == Command::End )
			{
//				kdDebug(8101) << "End found: CurrentValue: " << currentValue << endl;
				if ( nw == currentValue )
					m_destination->commandsList()->prepend( new Command( Command::Start, y ) );
				// else: the change continues, dont do anything
			}
			else
			{
//				kdDebug(8101) << "CurrentValue: " << currentValue << endl;
				if ( nw < currentValue )
					m_destination->commandsList()->prepend( new Command( Command::End, y ) );
			}

			c = m_source->commandsList()->first();
			if ( c && c->m_type == Command::End )
			{
//				kdDebug(8101) << "End found: CurrentValue: " << currentValue << endl;
				if ( nw == currentValue )
					m_source->commandsList()->prepend( new Command( Command::Start, x ) );
				// else: the change continues, dont do anything
			}
			else
			{
//				kdDebug(8101) << "CurrentValue: " << currentValue << endl;
				if ( nw < currentValue )
					m_source->commandsList()->prepend( new Command( Command::End, x ) );
			}

			--y;
			--x;
			break;
		case 2: // west
//			kdDebug(8101) << "Picking west" << endl;
//			kdDebug(8101) << "Source[" << ( x - 1 ) << "] = " << QString( source[ x-1 ] ) << ", destination[" << ( y - 1 ) << "] = " << QString( destination[ y-1 ] ) << endl;

			c = m_source->commandsList()->first();
			if ( c && c->m_type == Command::End )
			{
//				kdDebug(8101) << "End found: CurrentValue: " << currentValue << endl;
				if ( w == currentValue )
					m_source->commandsList()->prepend( new Command( Command::Start, x ) );
				// else: the change continues, dont do anything
			}
			else
			{
//				kdDebug(8101) << "CurrentValue: " << currentValue << endl;
				if ( w < currentValue )
					m_source->commandsList()->prepend( new Command( Command::End, x ) );
			}

			--x;
			break;
		}
	}

//	kdDebug(8101) << "Source string: " << m_source->string() << endl;
//	c = m_source->commandsList()->first();
//	QStringList list;
//	unsigned int prevValue = 0;
//	for ( ; c; c = m_source->commandsList()->next() )
//	{
//		kdDebug(8101) << "Source Command Entry : Type: " << c->type() << ", Offset: " << c->offset() << endl;
//		list.append( m_source->string().mid( prevValue, c->offset() - prevValue ) );
//		prevValue = c->offset();
//	}
//	if ( prevValue < m_source->string().length() - 1 )
//	{
//		list.append( m_source->string().mid( prevValue, m_source->string().length() - prevValue ) );
//	}
//	kdDebug(8101) << "Source Resulting stringlist : " << list.join("\n") << endl;

//	list.clear();
//	prevValue = 0;

//	kdDebug(8101) << "Destination string: " << m_destination->string() << endl;
//	for ( ; c; c = m_destination->commandsList()->next() )
//	{
//		kdDebug(8101) << "Destination Command Entry : Type: " << c->type() << ", Offset: " << c->offset() << endl;
//		list.append( m_destination->string().mid( prevValue, c->offset() - prevValue ) );
//		prevValue = c->offset();
//	}
//	if ( prevValue < m_destination->string().length() - 1 )
//	{
//		list.append( m_destination->string().mid( prevValue, m_destination->string().length() - prevValue ) );
//	}
//	kdDebug(8101) << "Destination Resulting string : " << list.join("\n") << endl;
}

#endif // INLINE_DIFFERENCES
