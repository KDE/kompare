/***************************************************************************
                                generalprefs.h  -  description
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

#ifndef GENERALPREFS_H
#define GENERALPREFS_H

#include <qwidget.h>

#include <kcolorbutton.h>

#include "prefsbase.h"
#include "settingsbase.h"
#include "generalsettings.h"

class GeneralPrefs : public PrefsBase
{
Q_OBJECT
public:
	GeneralPrefs( QWidget* );
	~GeneralPrefs();

public:
	void setSettings( GeneralSettings* );
	GeneralSettings* getSettings( void );

public:
	GeneralSettings* m_settings;

public:
	virtual void restore();
	virtual void apply();
	virtual void setDefaults();

public:
	KColorButton* m_removedColorButton;
	KColorButton* m_changedColorButton;
	KColorButton* m_addedColorButton;
};

#endif
