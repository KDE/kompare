/***************************************************************************
                                kompareprocess.cpp  -  description
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

#include <qdir.h>

#include <kdebug.h>

#include "diffsettings.h"
#include "kompareprocess.h"

KompareProcess::KompareProcess( QString source, QString destination, QString dir, DiffSettings* diffSettings )
	: KShellProcess()
{
	// connect the stdout and stderr signals
	connect( this, SIGNAL( receivedStdout( KProcess*, char*, int ) ),
	         this, SLOT  ( receivedStdout( KProcess*, char*, int ) ) );
	connect( this, SIGNAL( receivedStderr( KProcess*, char*, int ) ),
	         this, SLOT  ( receivedStderr( KProcess*, char*, int ) ) );

	// connect the signal that indicates that the proces has exited
	connect( this, SIGNAL( processExited( KProcess* ) ),
	         this, SLOT  ( processExited( KProcess* ) ) );
	
	// Write command and options
	if( diffSettings ) {
		writeCommandLine( diffSettings );
	} else {
		writeDefaultCommandLine();
	}
	
	if( !dir.isEmpty() ) {
		QDir::setCurrent( dir );
	}
	
	// Write file names
	*this << "--";
	*this << constructRelativePath( dir, source );
	*this << constructRelativePath( dir, destination );
}

void KompareProcess::writeDefaultCommandLine()
{
	*this << "diff" << "-U65535" << "-dr";
}

void KompareProcess::writeCommandLine( DiffSettings* diffSettings )
{
	// load the executable into the KProcess
	*this << "diff";
	
	switch( diffSettings->m_format ) {
	case Unified :
		*this << "-U" << QString::number( diffSettings->m_linesOfContext );
		break;
	case Context :
		*this << "-C" << QString::number( diffSettings->m_linesOfContext );
		break;
	case RCS :
		*this << "-n";
		break;
	case Ed :
		*this << "-e";
		break;
	case SideBySide:
		*this << "-y";
		break;
	case Normal :
	case Unknown :
	default:
		break;
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
	
	if ( diffSettings->m_recursive )
	{
		*this << "-r";
	}
	
	if ( diffSettings->m_newFiles )
	{
		*this << "-N";
	}
	
	if ( diffSettings->m_allText )
	{
		*this << "-a";
	}
}

KompareProcess::~KompareProcess()
{}

void KompareProcess::receivedStdout( KProcess* /* process */, char* buffer, int length )
{
	// add all output to m_stdout
	m_stdout += QString::fromLatin1( buffer, length );
}

void KompareProcess::receivedStderr( KProcess* /* process */, char* buffer, int length )
{
	// add all output to m_stderr
	m_stderr += QString::fromLatin1( buffer, length );
}

bool KompareProcess::start()
{
#ifndef NDEBUG
	QString cmdLine;
	for( QStrListIterator i( *this->args() ); i.current(); ++i ) {
		cmdLine += i.current();
		cmdLine += " ";
	}
	kdDebug() << cmdLine << endl;
#endif
	return( KShellProcess::start( KProcess::NotifyOnExit, KProcess::AllOutput ) );
}

void KompareProcess::processExited( KProcess* /* proc */ )
{
	// exit status of 0: no differences
	//                1: some differences
	//                2: error
	emit diffHasFinished( normalExit() && exitStatus() == 1 );
}

const QStringList KompareProcess::getDiffOutput()
{
	return QStringList::split( "\n", m_stdout );
}

#include "kompareprocess.moc"
