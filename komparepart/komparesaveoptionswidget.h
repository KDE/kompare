/***************************************************************************
                                kdiffsaveoptionswidget.h  -  description
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

#ifndef KDIFFSAVEOPTIONSWIDGET_H
#define KDIFFSAVEOPTIONSWIDGET_H

#include <kurl.h>

#include "kdiffsaveoptionsbase.h"
#include "kdiff.h"

class DiffSettings;

class KDiffSaveOptionsWidget : public KDiffSaveOptionsBase, KDiff
{
Q_OBJECT
public:
	KDiffSaveOptionsWidget( QString source, QString destination, DiffSettings* settings, QWidget* parent );
	~KDiffSaveOptionsWidget();
	
	void      saveOptions();
	QString   directory() const;
	
protected slots:
	void      updateCommandLine();
	
private:
	void      loadOptions();
	
	DiffSettings*   m_settings;
	QString         m_source;
	QString         m_destination;
};

#endif
