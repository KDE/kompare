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

#include <kdebug.h>

#include "kdiffprocess.h"

#include "kdiffprocess.moc"

KDiffProcess::KDiffProcess( DiffSettings* diffSettings, const KURL& source, const KURL& destination )
	: m_leftURL( source ),
	m_rightURL( destination )
{
	// create the process to run diff with
	m_diffProcess = new KProcess();

	// connect the stdout and stderr signals
	connect( m_diffProcess, SIGNAL( receivedStdout( KProcess*, char*, int ) ), this, SLOT( receivedStdout( KProcess*, char*, int ) ) );
	connect( m_diffProcess, SIGNAL( receivedStderr( KProcess*, char*, int ) ), this, SLOT( receivedStderr( KProcess*, char*, int ) ) );

	// connect the signal that indicates that the proces has exited
	connect( m_diffProcess, SIGNAL( processExited( KProcess* ) ), this, SLOT( processExited( KProcess* ) ) );

	// load the executable into the KProcess
	*m_diffProcess << "diff";

	// load all parameters into the diff for as far as we know them
/*
	-H // for faster handling of large files
//	-(integer) // aantal regels
	-B // ignore lege regels die verwijdert of toegevoegd worden
	-b // ignore changes in number of whitespace
	-d // search for smaller diff
	-i // ignore changes in case
	-n // use RCS format for diff
	-p // show which c function the change is in..
	-N // include files that are only in one directory
	-r // search recursively through directories
	-s // report when two files are identical
	-t // expands tab to spaces in output
	-u/-c // for respectively unified or context format diff
	-U/-C [number] // for respectively showing [number] of lines of context for unified or context format diff
	-w // ignore whitespace when comparing lines
*/
//	diffSettings->m_useUnifiedDiff = true;
//	diffSettings->m_linesOfContext = 3;

	if ( diffSettings->m_linesOfContext != 3 ) // standard value
	{
		// if it differs then we need these:
		if ( diffSettings->m_useUnifiedDiff )
		{
			*m_diffProcess << "-U" << QString( "%1" ).arg( diffSettings->m_linesOfContext );
		}
		else if ( diffSettings->m_useContextDiff )
		{
			*m_diffProcess << "-C" << QString( "%1" ).arg( diffSettings->m_linesOfContext );
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
			*m_diffProcess << "-u";
		}
		else if ( diffSettings->m_useContextDiff )
		{
			*m_diffProcess << "-c";
		}
		else if ( diffSettings->m_useRCSDiff )
		{
			*m_diffProcess << "-n";
		}
		else if ( diffSettings->m_useNormalDiff )
		{
			// do nothing... :)
		}
	}

	if ( diffSettings->m_largeFiles )
	{
		*m_diffProcess << "-H";
	}

	if ( diffSettings->m_ignoreWhiteSpace )
	{
		*m_diffProcess << "-b";
	}

	if ( diffSettings->m_ignoreEmptyLines )
	{
		*m_diffProcess << "-B";
	}

	if ( diffSettings->m_createSmallerDiff )
	{
		*m_diffProcess << "-d";
	}

	if ( diffSettings->m_ignoreChangesInCase )
	{
		*m_diffProcess << "-i";
	}

	if ( diffSettings->m_showCFunctionChange )
	{
		*m_diffProcess << "-p";
	}

	if ( diffSettings->m_convertTabsToSpaces )
	{
		*m_diffProcess << "-t";
	}

	if ( diffSettings->m_ignoreWhitespaceComparingLines )
	{
		*m_diffProcess << "-w";
	}
}

KDiffProcess::~KDiffProcess()
{
	delete m_diffProcess;
}

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
	*m_diffProcess << "--" << m_leftURL.path() << m_rightURL.path();
	QString cmdLine;
	for( QStrListIterator i( *m_diffProcess->args() ); i.current(); ++i ) {
		cmdLine += i.current();
		cmdLine += " ";
	}
	kdDebug() << cmdLine << endl;
	return( m_diffProcess->start( KProcess::NotifyOnExit, KProcess::AllOutput ) );
}

void KDiffProcess::processExited( KProcess* /* proc */ )
{
	// exit status of 0: no differences
	//                1: some differences
	//                2: error
	emit diffHasFinished( m_diffProcess->normalExit()
	                      && m_diffProcess->exitStatus() == 1 );
}

const QStringList KDiffProcess::getDiffOutput()
{
	return QStringList::split( "\n", m_stdout );
}
