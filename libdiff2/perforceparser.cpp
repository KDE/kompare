/**************************************************************************
**                              perforceparser.cpp
**                              ------------------
**      begin                   : Sun Aug  4 15:05:35 2002
**      copyright               : (C) 2002-2004 Otto Bruggeman
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

#include "perforceparser.h"

using namespace Diff2;

PerforceParser::PerforceParser( const KompareModelList* list, const QStringList& diff ) : ParserBase( list, diff )
{
	m_contextDiffHeader1.setPattern( "==== (.*) - (.*) ====" );
	m_rcsDiffHeader.setPattern     ( "==== (.*) - (.*) ====" );
	m_unifiedDiffHeader1.setPattern( "==== (.*) - (.*) ====" );
}

PerforceParser::~PerforceParser()
{
}

enum Kompare::Format PerforceParser::determineFormat()
{
	kdDebug(8101) << "Determining the format of the Perforce Diff" << endl;

	QRegExp unifiedRE( "^@@" );
	QRegExp contextRE( "^\\*{15}" );
	QRegExp rcsRE    ( "^[acd][0-9]+ [0-9]+" );
	// Summary is not supported since it gives no useful parsable info

	QStringList::ConstIterator it = m_diffLines.begin();

	while( it != m_diffLines.end() )
	{
		if( (*it).find( unifiedRE, 0 ) == 0 )
		{
			kdDebug(8101) << "Difflines are from a Unified diff..." << endl;
			return Kompare::Unified;
		}
		else if( (*it).find( contextRE, 0 ) == 0 )
		{
			kdDebug(8101) << "Difflines are from a Context diff..." << endl;
			return Kompare::Context;
		}
		else if( (*it).find( rcsRE, 0 ) == 0 )
		{
			kdDebug(8101) << "Difflines are from a RCS diff..." << endl;
			return Kompare::RCS;
		}
		++it;
	}
	kdDebug(8101) << "Difflines are from an unknown diff..." << endl;
	return Kompare::UnknownFormat;
}

bool PerforceParser::parseContextDiffHeader()
{
//	kdDebug(8101) << "ParserBase::parseContextDiffHeader()" << endl;
	bool result = false;

	while ( m_diffIterator != m_diffLines.end() )
	{
		if ( m_contextDiffHeader1.exactMatch( *(m_diffIterator)++ ) )
		{
			kdDebug(8101) << "Matched length Header1 = " << m_contextDiffHeader1.matchedLength() << endl;
			kdDebug(8101) << "Matched string Header1 = " << m_contextDiffHeader1.cap( 0 ) << endl;
			kdDebug(8101) << "First capture  Header1 = " << m_contextDiffHeader1.cap( 1 ) << endl;
			kdDebug(8101) << "Second capture Header1 = " << m_contextDiffHeader1.cap( 2 ) << endl;

			m_currentModel = new DiffModel();
			QRegExp sourceFileRE     ( "^([^#]+)#(.*)$" );
			QRegExp destinationFileRE( "^([^#]+)#?(.*)$" );
			sourceFileRE.exactMatch( m_contextDiffHeader1.cap( 1 ) );
			destinationFileRE.exactMatch( m_contextDiffHeader1.cap( 2 ) );
			kdDebug(8101) << "Source File      : " << sourceFileRE.cap( 1 ) << endl;
			kdDebug(8101) << "Destination File : " << destinationFileRE.cap( 1 ) << endl;
			m_currentModel->setSourceFile          ( sourceFileRE.cap( 1 ) );
			m_currentModel->setSourceTimestamp     ( m_contextDiffHeader1.cap( 2 ) );
			m_currentModel->setSourceRevision      ( m_contextDiffHeader1.cap( 4 ) );
			m_currentModel->setDestinationFile     ( destinationFileRE.cap( 1 ) );
			m_currentModel->setDestinationTimestamp( m_contextDiffHeader2.cap( 2 ) );
			m_currentModel->setDestinationRevision ( m_contextDiffHeader2.cap( 4 ) );

			result = true;

			break;
		}
	}

	return result;
}

bool PerforceParser::parseRCSDiffHeader()
{
	return false;
}

bool PerforceParser::parseUnifiedDiffHeader()
{
	return false;
}

