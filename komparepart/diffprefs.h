/***************************************************************************
                                diffprefs.h  -  description
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

#ifndef DIFFPREFS_H
#define DIFFPREFS_H

#include <qcheckbox.h>
#include <qspinbox.h>
#include <qvbuttongroup.h>
#include <qwidget.h>

#include "diffsettings.h"
#include "prefsbase.h"
#include "settingsbase.h"

class DiffPrefs : public PrefsBase
{
Q_OBJECT
public:
	DiffPrefs( QWidget* );
	~DiffPrefs();

public:
	DiffSettings* m_settings;

public:
	void setSettings( DiffSettings* );
	DiffSettings* settings( void );

public:
	virtual void restore();
	virtual void apply();
	virtual void setDefaults();

public:
	// loc == lines of context
	QSpinBox* m_locSpinBox;

	QCheckBox* m_smallerCheckBox;
	QCheckBox* m_largerCheckBox;
	QCheckBox* m_tabsCheckBox;
	QCheckBox* m_caseCheckBox;
	QCheckBox* m_linesCheckBox;
	QCheckBox* m_whitespaceCheckBox;

	QVButtonGroup* m_modeButtonGroup;
};

#endif
