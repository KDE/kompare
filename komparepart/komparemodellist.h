/***************************************************************************
                          kdiffmodellist.h  -  description
                             -------------------
    begin                : Tue Jun 26 2001
    copyright            : (C) 2001 by John Firebaugh
    email                : jfirebaugh@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDIFFMODELLIST_H
#define KDIFFMODELLIST_H

#include <qobject.h>
#include <qlist.h>

#include <kurl.h>

#include "diffmodel.h"

class QFile;

class KDiffProcess;
class DiffSettings;

class KDiffModelList : public QObject {
	Q_OBJECT
	
public:
	enum Mode { Compare, Diff };
	enum Status { RunningDiff, Parsing, FinishedParsing, FinishedWritingDiff };
	
	KDiffModelList();
	~KDiffModelList();
	
	void compare( const KURL& source, const KURL& destination );
	void readDiffFile( QFile& file );
	void writeDiffFile( QString file, DiffSettings* settings );
	void saveDestination( int index );
	
	int parseDiffs( const QStringList& list );
	
	Mode mode() { return m_mode; };
	
	void addModel( DiffModel* model );
	int modelCount() { return m_models.count(); };
	DiffModel* modelAt( int i ) { return m_models.at( i ); };
	KURL sourceBaseURL() const { return m_sourceURL.upURL(); };
	KURL destinationBaseURL() const { return m_destinationURL.upURL(); };
	
signals:
	void status( KDiffModelList::Status );
	void error( QString error );
	void modelAdded( DiffModel* );

protected slots:
	void slotDiffProcessFinished( bool success );
	void slotWriteDiffOutput( bool success );

private:
	Mode              m_mode;
	KDiffProcess*     m_diffProcess;
	QList<DiffModel>  m_models;
	KURL              m_sourceURL;
	KURL              m_destinationURL;
	QString           m_diffFile;
	
};

#endif
