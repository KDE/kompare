/***************************************************************************
                                kompareprocess.h  -  description
                                -------------------
        begin                   : Sun Mar 4 2001
        copyright               : (C) 2001-2004 by Otto Bruggeman
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

#ifndef KOMPAREPROCESS_H
#define KOMPAREPROCESS_H

#include <kprocess.h>

class QTextCodec;

class KompareProcess : public KProcess
{
	Q_OBJECT

public:
	KompareProcess( const QStringList& arguments, const QString& encoding = QString::null );
	~KompareProcess();

	bool start();

	QString diffOutput() { return m_stdout; }
	QString stdOut()     { return m_stdout; }
	QString stdErr()     { return m_stderr; }

	void setEncoding( const QString& encoding );

signals:
	void diffHasFinished( bool finishedNormally );

protected slots:
	void slotReceivedStdout( KProcess*, char*, int );
	void slotReceivedStderr( KProcess*, char*, int );
	void slotProcessExited ( KProcess* );

private:
	QString       m_stdout;
	QString       m_stderr;
	QTextDecoder* m_textDecoder;
	QStringList   m_arguments;
};

#endif
