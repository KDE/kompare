/**************************************************************************
**                              cvsdiffparser.cpp
**                              -----------------
**      begin                   : Sun Aug  4 15:05:35 2002
**      copyright               : (C) 2002-2003 by Otto Bruggeman
**      email                   : otto.bruggeman@home.nl
**
***************************************************************************/
/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   ( at your option ) any later version.
**
***************************************************************************/

#include <qregexp.h>

#include <kdebug.h>

#include "cvsdiffparser.h"

using namespace Diff2;

CVSDiffParser::CVSDiffParser( const QStringList& diff ) : ParserBase( diff )
{
	// The regexps needed for context cvs diff parsing, the rest is the same as in parserbase.cpp
	// third capture in header1 is non optional for cvs diff, it is the revision
	m_contextDiffHeader1.setPattern( "^\\*\\*\\* ([^\\t]+)\\t([^\\t]+)\\t(.*)$" );
	m_contextDiffHeader2.setPattern( "^--- ([^\\t]+)\\t([^\\t]+)(|\\t(.*))$" );
}

CVSDiffParser::~CVSDiffParser()
{
}

enum Kompare::Format CVSDiffParser::determineFormat()
{
//	kdDebug(8101) << "Determining the format of the CVSDiff" << endl;

	QRegExp normalRE ( "^[0-9]+[0-9,]*[acd][0-9]+[0-9,]*$" );
	QRegExp unifiedRE( "^--- [^\\t]+\\t" );
	QRegExp contextRE( "^\\*\\*\\* [^\\t]+\\t" );
	QRegExp rcsRE    ( "^[acd][0-9]+ [0-9]+" );
	QRegExp edRE     ( "^[0-9]+[0-9,]*[acd]" );

	QStringList::ConstIterator it = m_diffLines.begin();

	while( it != m_diffLines.end() )
	{
		if( (*it).find( normalRE, 0 ) == 0 )
		{
//			kdDebug(8101) << "Difflines are from a Normal diff..." << endl;
			return Kompare::Normal;
		}
		else if( (*it).find( unifiedRE, 0 ) == 0 )
		{
//			kdDebug(8101) << "Difflines are from a Unified diff..." << endl;
			return Kompare::Unified;
		}
		else if( (*it).find( contextRE, 0 ) == 0 )
		{
//			kdDebug(8101) << "Difflines are from a Context diff..." << endl;
			return Kompare::Context;
		}
		else if( (*it).find( rcsRE, 0 ) == 0 )
		{
//			kdDebug(8101) << "Difflines are from a RCS diff..." << endl;
			return Kompare::RCS;
		}
		else if( (*it).find( edRE, 0 ) == 0 )
		{
//			kdDebug(8101) << "Difflines are from an ED diff..." << endl;
			return Kompare::Ed;
		}
		++it;
	}
//	kdDebug(8101) << "Difflines are from an unknown diff..." << endl;
	return Kompare::UnknownFormat;
}

bool CVSDiffParser::parseEdDiffHeader()
{
	return false;
}

bool CVSDiffParser::parseRCSDiffHeader()
{
	return false;
}

bool CVSDiffParser::parseEdHunkHeader()
{
	return false;
}

bool CVSDiffParser::parseRCSHunkHeader()
{
	return false;
}

bool CVSDiffParser::parseEdHunkBody()
{
	return false;
}

bool CVSDiffParser::parseRCSHunkBody()
{
	return false;
}
