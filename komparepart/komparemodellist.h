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

#include "kdiff.h"
#include "diffmodel.h"

class QFile;

class KTempFile;

class KDiffProcess;
class DiffSettings;

class KDiffModelList : public QObject, KDiff
{
	Q_OBJECT
	
public:
	enum Mode { Compare, Diff };
	enum Status { RunningDiff, Parsing, FinishedParsing, FinishedWritingDiff };
	
	KDiffModelList();
	~KDiffModelList();
	
	bool compare( const KURL& source, const KURL& destination );
	bool saveDestination( int index );

	bool openDiff( const KURL& url );
	bool saveDiff( const KURL& url, QString directory, DiffSettings* settings );
	bool saveAll();
	
	void swap();
	
	Mode mode() { return m_mode; };
	
	const QList<DiffModel>& getModels() { return m_models; };
	int modelCount() { return m_models.count(); };
	DiffModel* modelAt( int i ) { return m_models.at( i ); };
	KURL sourceBaseURL() const;
	KURL destinationBaseURL() const;
	QString sourceTemp() const { return m_sourceTemp; };
	QString destinationTemp() const { return m_destinationTemp; };
	KURL diffURL() const { return m_diffURL; };
	bool isModified() const;
	
signals:
	void status( KDiffModelList::Status );
	void error( QString error );
	void modelsChanged();

protected slots:
	void slotDiffProcessFinished( bool success );
	void slotWriteDiffOutput( bool success );

private:
	int parseDiffs( const QStringList& list );
	void clear();
	
	Mode              m_mode;
	KDiffProcess*     m_diffProcess;
	QList<DiffModel>  m_models;

	KURL              m_sourceURL;
	KURL              m_destinationURL;
	QString           m_sourceTemp;
	QString           m_destinationTemp;

	KURL              m_diffURL;
	KTempFile*        m_diffTemp;
	
};

#endif
