/***************************************************************************
                                kdiffprocess.h  -  description
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

#ifndef KDIFFPROCESS_H
#define KDIFFPROCESS_H

#include <qobject.h>
#include <qstringlist.h>

#include <kprocess.h>
#include <kurl.h>

#include "diffsettings.h"

/**
  *@author Otto Bruggeman
  */

class KDiffProcess : public QObject
{
Q_OBJECT

public:	// methods
	KDiffProcess( DiffSettings*, const KURL& sourceURL, const KURL& destinationURL );
	~KDiffProcess();
	/** No descriptions */
	bool start();
	/** No descriptions */
	const QStringList getDiffOutput();
	QString getStdout() { return m_stdout; };
	QString getStderr() { return m_stderr; };

private:
	QString m_stdout;
	QString m_stderr;

public: // variables
	KProcess* m_diffProcess;

	KURL m_leftURL;
	KURL m_rightURL;

public slots:	// slots
	void receivedStdout( KProcess*, char*, int );
	void receivedStderr( KProcess*, char*, int );
	/** No descriptions */
	void processExited( KProcess* proc );
signals: // Signals
	/** No descriptions */
	void diffHasFinished( bool finishedNormally );
};

#endif
