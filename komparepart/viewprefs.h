/***************************************************************************
                                generalprefs.h  -  description
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

#ifndef VIEWPREFS_H
#define VIEWPREFS_H

#include "prefsbase.h"

class QCheckBox;
class KColorButton;
class QSpinBox;

class ViewSettings;

class ViewPrefs : public PrefsBase
{
Q_OBJECT
public:
	ViewPrefs( QWidget* );
	~ViewPrefs();

public:
	void setSettings( ViewSettings* );
	ViewSettings* settings( void );

public:
	ViewSettings* m_settings;

public:
	virtual void restore();
	virtual void apply();
	virtual void setDefaults();

public:
	KColorButton* m_removedColorButton;
	KColorButton* m_changedColorButton;
	KColorButton* m_addedColorButton;
	KColorButton* m_appliedColorButton;
	QCheckBox*    m_showEntireFile;
	// snol == scroll number of lines
	QSpinBox*     m_snolSpinBox;
	QSpinBox*     m_tabSpinBox;
};

#endif
