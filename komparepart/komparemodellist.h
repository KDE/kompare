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

class KDiffModelList : public QObject {
	Q_OBJECT
	
public:
	KDiffModelList();
	~KDiffModelList();
	void addModel( DiffModel* model );
	int modelCount() { return m_models.count(); };
	DiffModel* modelAt( int i ) { return m_models.at( i ); };
	const KURL& sourceBaseURL() const { return m_sourceBaseURL; };
	const KURL& destinationBaseURL() const { return m_destinationBaseURL; };
	void setSourceBaseURL( const KURL& sourceBaseURL );
	void setDestinationBaseURL( const KURL& destiationBaseURL );
	
signals:
	void modelAdded( DiffModel* );
	
private:
	QList<DiffModel>  m_models;
	KURL              m_sourceBaseURL;
	KURL              m_destinationBaseURL;
	
};

#endif
