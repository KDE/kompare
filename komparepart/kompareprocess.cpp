/***************************************************************************
                                kdiffprocess.cpp  -  description
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

#include <qstringlist.h>

#include <kio/netaccess.h>
#include <kdebug.h>

#include "diffsettings.h"

#include "kdiffprocess.h"
#include "kdiffprocess.moc"

KDiffProcess::KDiffProcess( const KURL& source, const KURL& destination, DiffSettings* diffSettings )
	: KShellProcess(),
	m_leftURL( source ),
	m_rightURL( destination )
{
	// connect the stdout and stderr signals
	connect( this, SIGNAL( receivedStdout( KProcess*, char*, int ) ),
	         this, SLOT( receivedStdout( KProcess*, char*, int ) ) );
	connect( this, SIGNAL( receivedStderr( KProcess*, char*, int ) ),
	         this, SLOT( receivedStderr( KProcess*, char*, int ) ) );

	// connect the signal that indicates that the proces has exited
	connect( this, SIGNAL( processExited( KProcess* ) ),
	         this, SLOT( processExited( KProcess* ) ) );
	
	if( diffSettings ) {
		writeCommandLine( diffSettings );
	} else {
		writeDefaultCommandLine();
	}
}

void KDiffProcess::writeDefaultCommandLine()
{
	*this << "diff" << "-U65535" << "-d";
}

void KDiffProcess::writeCommandLine( DiffSettings* diffSettings )
{
	// load the executable into the KProcess
	*this << "diff";
	
	if ( diffSettings->m_linesOfContext != 3 ) // standard value
	{
		// if it differs then we need these:
		if ( diffSettings->m_useUnifiedDiff )
		{
			*this << "-U" << QString::number( diffSettings->m_linesOfContext );
		}
		else if ( diffSettings->m_useContextDiff )
		{
			*this << "-C" << QString::number( diffSettings->m_linesOfContext );
		}
		else if ( diffSettings->m_useNormalDiff )
		{
			// do nothing... :)
		}
	}
	else
	{
		if ( diffSettings->m_useUnifiedDiff )
		{
			*this << "-u";
		}
		else if ( diffSettings->m_useContextDiff )
		{
			*this << "-c";
		}
		else if ( diffSettings->m_useRCSDiff )
		{
			*this << "-n";
		}
		else if ( diffSettings->m_useNormalDiff )
		{
			// do nothing... :)
		}
	}

	if ( diffSettings->m_largeFiles )
	{
		*this << "-H";
	}

	if ( diffSettings->m_ignoreWhiteSpace )
	{
		*this << "-b";
	}

	if ( diffSettings->m_ignoreEmptyLines )
	{
		*this << "-B";
	}

	if ( diffSettings->m_createSmallerDiff )
	{
		*this << "-d";
	}

	if ( diffSettings->m_ignoreChangesInCase )
	{
		*this << "-i";
	}

	if ( diffSettings->m_showCFunctionChange )
	{
		*this << "-p";
	}

	if ( diffSettings->m_convertTabsToSpaces )
	{
		*this << "-t";
	}

	if ( diffSettings->m_ignoreWhitespaceComparingLines )
	{
		*this << "-w";
	}
}

KDiffProcess::~KDiffProcess()
{}

void KDiffProcess::receivedStdout( KProcess* /* process */, char* buffer, int length )
{
	// add all output to m_stdout
	m_stdout += QString::fromLatin1( buffer, length );
}

void KDiffProcess::receivedStderr( KProcess* /* process */, char* buffer, int length )
{
	// add all output to m_stderr
	m_stderr += QString::fromLatin1( buffer, length );
}

bool KDiffProcess::start()
{
	if( KIO::NetAccess::download( m_leftURL, m_leftTemp ) &&
	    KIO::NetAccess::download( m_rightURL, m_rightTemp ) ) {
		*this << "--" << m_leftTemp << m_rightTemp;
		QString cmdLine;
		for( QStrListIterator i( *this->args() ); i.current(); ++i ) {
			cmdLine += i.current();
			cmdLine += " ";
		}
		kdDebug() << cmdLine << endl;
		return( KShellProcess::start( KProcess::NotifyOnExit, KProcess::AllOutput ) );
	}
	return false;
}

void KDiffProcess::processExited( KProcess* /* proc */ )
{
	KIO::NetAccess::removeTempFile( m_leftTemp );
	KIO::NetAccess::removeTempFile( m_rightTemp );
	// exit status of 0: no differences
	//                1: some differences
	//                2: error
	emit diffHasFinished( normalExit() && exitStatus() == 1 );
}

const QStringList KDiffProcess::getDiffOutput()
{
	return QStringList::split( "\n", m_stdout );
}
