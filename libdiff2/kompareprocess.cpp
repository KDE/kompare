/***************************************************************************
                                kompareprocess.cpp  -  description
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

#include <qdir.h>
#include <qstringlist.h>
#include <qtextcodec.h>

#include <kcharsets.h>
#include <kdebug.h>
#include <kglobal.h>

#include "diffsettings.h"
#include "kompareprocess.h"

KompareProcess::KompareProcess( const QStringList& arguments, const QString& encoding )
	: KProcess(),
	m_textDecoder( 0 ),
	m_arguments( arguments )
{
	setUseShell( true );

	// connect the stdout and stderr signals
	connect( this, SIGNAL( receivedStdout( KProcess*, char*, int ) ),
	         SLOT  ( slotReceivedStdout( KProcess*, char*, int ) ) );
	connect( this, SIGNAL( receivedStderr( KProcess*, char*, int ) ),
	         SLOT  ( slotReceivedStderr( KProcess*, char*, int ) ) );

	// connect the signal that indicates that the proces has exited
	connect( this, SIGNAL( processExited( KProcess* ) ),
	         SLOT  ( slotProcessExited( KProcess* ) ) );

	setEncoding( encoding );

	*this << "LANG=C";

	*this << arguments;
}

KompareProcess::~KompareProcess()
{
}

void KompareProcess::setEncoding( const QString& encoding )
{
	QTextCodec* textCodec = KGlobal::charsets()->codecForName( encoding.latin1() );

	if ( textCodec )
	{
		m_textDecoder = textCodec->makeDecoder();
	}
	else
	{
		kdDebug(8101) << "Using locale codec as backup..." << endl;
		textCodec = QTextCodec::codecForLocale();
		m_textDecoder = textCodec->makeDecoder();
	}
}

void KompareProcess::slotReceivedStdout( KProcess* /* process */, char* buffer, int length )
{
	// add all output to m_stdout
	if ( m_textDecoder )
		m_stdout += m_textDecoder->toUnicode( buffer, length );
	else
		kdDebug(8101) << "KompareProcess::slotReceivedStdout : No decoder !!!" << endl;
}

void KompareProcess::slotReceivedStderr( KProcess* /* process */, char* buffer, int length )
{
	// add all output to m_stderr
	if ( m_textDecoder )
		m_stderr += m_textDecoder->toUnicode( buffer, length );
	else
		kdDebug(8101) << "KompareProcess::slotReceivedStderr : No decoder !!!" << endl;
}

bool KompareProcess::start()
{
	return( KProcess::start( KProcess::NotifyOnExit, KProcess::AllOutput ) );
}

void KompareProcess::slotProcessExited( KProcess* /* proc */ )
{
	// exit status of 0: no differences
	//                1: some differences
	//                2: error but there may be differences !
	kdDebug(8101) << "Exited with exit status : " << exitStatus() << endl;

	emit diffHasFinished( normalExit() && exitStatus() != 0 );
}

#include "kompareprocess.moc"

