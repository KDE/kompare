/***************************************************************************
                                kompareprocess.h  -  description
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

#ifndef KOMPAREPROCESS_H
#define KOMPAREPROCESS_H

#include <kprocess.h>
#include <kurl.h>

#include "kompare.h"

class DiffSettings;

class KompareProcess : public KShellProcess, Kompare
{
	Q_OBJECT
	
public:
	KompareProcess( QString source, QString destination, QString directory = QString::null, DiffSettings* settings = 0 );
	~KompareProcess();
	
	bool start();
	
	const QStringList  getDiffOutput();
	QString            getStdout()       { return m_stdout; };
	QString            getStderr()       { return m_stderr; };
	
signals:
	void diffHasFinished( bool finishedNormally );
	
protected:
	void writeDefaultCommandLine();
	void writeCommandLine( DiffSettings* settings );
	
protected slots:
	void receivedStdout( KProcess*, char*, int );
	void receivedStderr( KProcess*, char*, int );
	void processExited( KProcess* proc );
	
private:
	QString      m_stdout;
	QString      m_stderr;
};

#endif
