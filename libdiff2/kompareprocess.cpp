/***************************************************************************
                                kompareprocess.cpp
                                ------------------
        begin                   : Sun Mar 4 2001
        Copyright 2001-2005,2009 Otto Bruggeman <bruggie@gmail.com>
        Copyright 2001-2003 John Firebaugh <jfirebaugh@kde.org>
        Copyright 2007-2008 Kevin Kofler   <kevin.kofler@chello.at>
****************************************************************************/

/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***************************************************************************/

#include "kompareprocess.h"

#include <QtCore/QDir>
#include <QtCore/QStringList>
#include <QtCore/QTextCodec>
#include <Q3ValueList>
#include <Q3CString>

#include <kcharsets.h>
#include <kdebug.h>
#include <kglobal.h>

#include "diffsettings.h"

KompareProcess::KompareProcess( DiffSettings* diffSettings, enum Kompare::DiffMode mode, QString source, QString destination, QString dir )
	: KProcess(),
	m_diffSettings( diffSettings ),
	m_mode( mode ),
	m_textDecoder( 0 )
{
	// connect the signal that indicates that the proces has exited
	connect( this, SIGNAL( finished( int, QProcess::ExitStatus ) ),
	         SLOT  ( slotFinished( int, QProcess::ExitStatus ) ) );

	setEnv( "LANG", "C" );

	// Write command and options
	if( m_mode == Kompare::Default )
	{
		writeDefaultCommandLine();
	}
	else
	{
		writeCommandLine();
	}

	if( !dir.isEmpty() ) {
		setWorkingDirectory( dir );
	}

	// Write file names
	*this << "--";
	*this << constructRelativePath( dir, source );
	*this << constructRelativePath( dir, destination );
}

void KompareProcess::writeDefaultCommandLine()
{
	if ( !m_diffSettings || m_diffSettings->m_diffProgram.isEmpty() )
	{
		*this << "diff" << "-dr";
	}
	else
	{
		*this << m_diffSettings->m_diffProgram << "-dr";
	}

	*this << "-U" << QString::number( m_diffSettings->m_linesOfContext );
}

void KompareProcess::writeCommandLine()
{
	// load the executable into the K3Process
	if ( m_diffSettings->m_diffProgram.isEmpty() )
	{
		kDebug(8101) << "Using the first diff in the path..." << endl;
		*this << "diff";
	}
	else
	{
		kDebug(8101) << "Using a user specified diff, namely: " << m_diffSettings->m_diffProgram << endl;
		*this << m_diffSettings->m_diffProgram;
	}

	switch( m_diffSettings->m_format ) {
	case Kompare::Unified :
		*this << "-U" << QString::number( m_diffSettings->m_linesOfContext );
		break;
	case Kompare::Context :
		*this << "-C" << QString::number( m_diffSettings->m_linesOfContext );
		break;
	case Kompare::RCS :
		*this << "-n";
		break;
	case Kompare::Ed :
		*this << "-e";
		break;
	case Kompare::SideBySide:
		*this << "-y";
		break;
	case Kompare::Normal :
	case Kompare::UnknownFormat :
	default:
		break;
	}

	if ( m_diffSettings->m_largeFiles 
// default diff does not have -H on OpenBSD
// so don't pass this option unless the user overrode the default program
#if defined(__OpenBSD__)
		&& !m_diffSettings->m_diffProgram.isEmpty() 
#endif
	   )
	{
		*this << "-H";
	}

	if ( m_diffSettings->m_ignoreWhiteSpace )
	{
		*this << "-b";
	}

	if ( m_diffSettings->m_ignoreAllWhiteSpace )
	{
		*this << "-w";
	}

	if ( m_diffSettings->m_ignoreEmptyLines )
	{
		*this << "-B";
	}

	if ( m_diffSettings->m_ignoreChangesDueToTabExpansion )
	{
		*this << "-E";
	}

	if ( m_diffSettings->m_createSmallerDiff )
	{
		*this << "-d";
	}

	if ( m_diffSettings->m_ignoreChangesInCase )
	{
		*this << "-i";
	}

	if ( m_diffSettings->m_ignoreRegExp && !m_diffSettings->m_ignoreRegExpText.isEmpty() )
	{
		*this << "-I " << m_diffSettings->m_ignoreRegExpText;
	}

	if ( m_diffSettings->m_showCFunctionChange )
	{
		*this << "-p";
	}

	if ( m_diffSettings->m_convertTabsToSpaces )
	{
		*this << "-t";
	}

	if ( m_diffSettings->m_recursive )
	{
		*this << "-r";
	}

	if ( m_diffSettings->m_newFiles )
	{
		*this << "-N";
	}

// This option is more trouble than it is worth... please do not ever enable it unless you want really weird crashes
//	if ( m_diffSettings->m_allText )
//	{
//		*this << "-a";
//	}

	if ( m_diffSettings->m_excludeFilePattern )
	{
		QStringList::ConstIterator it = m_diffSettings->m_excludeFilePatternList.begin();
		QStringList::ConstIterator end = m_diffSettings->m_excludeFilePatternList.end();
		for ( ; it != end; ++it )
		{
			*this << "-x" << *it;
		}
	}

	if ( m_diffSettings->m_excludeFilesFile && !m_diffSettings->m_excludeFilesFileURL.isEmpty() )
	{
		*this << "-X" << m_diffSettings->m_excludeFilesFileURL;
	}
}

KompareProcess::~KompareProcess()
{
	delete m_textDecoder;
}

void KompareProcess::setEncoding( const QString& encoding )
{
	if ( !encoding.compare( "default", Qt::CaseInsensitive ) )
	{
		m_textDecoder = QTextCodec::codecForLocale()->makeDecoder();
	}
	else
	{
		QTextCodec* textCodec = KGlobal::charsets()->codecForName( encoding.latin1() );
		if ( textCodec )
			m_textDecoder = textCodec->makeDecoder();
		else
		{
			kDebug(8101) << "Using locale codec as backup..." << endl;
			textCodec = QTextCodec::codecForLocale();
			m_textDecoder = textCodec->makeDecoder();
		}
	}
}

void KompareProcess::start()
{
#ifndef NDEBUG
	QString cmdLine;
	QStringList program = KProcess::program();
	QStringList::ConstIterator it = program.begin();
	for (; it != program.end(); ++it )
	    cmdLine += "\"" + (*it) + "\" ";
	kDebug(8101) << cmdLine << endl;
#endif
	setOutputChannelMode( SeparateChannels );
	setNextOpenMode( ReadOnly );
	KProcess::start();
}

void KompareProcess::slotFinished( int exitCode, QProcess::ExitStatus exitStatus )
{
	// add all output to m_stdout/m_stderr
	if ( m_textDecoder )
	{
		m_stdout = m_textDecoder->toUnicode( readAllStandardOutput() );
		m_stderr = m_textDecoder->toUnicode( readAllStandardError() );
	}
	else
		kDebug(8101) << "KompareProcess::slotFinished : No decoder !!!" << endl;

	// exit code of 0: no differences
	//              1: some differences
	//              2: error but there may be differences !
	kDebug(8101) << "Exited with exit code : " << exitCode << endl;
	emit diffHasFinished( exitStatus == NormalExit && exitCode != 0 );
}

#include "kompareprocess.moc"

