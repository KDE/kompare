/***************************************************************************
                                miscsettings.h  -  description
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

#ifndef MISCSETTINGS_H
#define MISCSETTINGS_H

#include <qwidget.h>

#include "settingsbase.h"

class MiscSettings : public SettingsBase
{
Q_OBJECT
public:
	MiscSettings( QWidget* parent );
	~MiscSettings();
public:
	// some virtual functions that will be overloaded from the base class
	virtual void loadSettings( KConfig* config );
	virtual void saveSettings( KConfig* config );
};

#endif
