/***************************************************************************
				generalsettings.h  -  description
				-------------------
	begin			: Sun Mar 4 2001
	copyright			: (C) 2001 by Otto Bruggeman
	email			: otto.bruggeman@home.nl
 ***************************************************************************/

/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
****************************************************************************/

#ifndef GENERALSETTINGS_H
#define GENERALSETTINGS_H

#include <qcolor.h>
#include <qwidget.h>

#include "settingsbase.h"

class GeneralSettings : public SettingsBase
{
Q_OBJECT
public:
	static const QColor default_removeColor;
	static const QColor default_changeColor;
	static const QColor default_addColor;

	GeneralSettings( QWidget* parent );
	~GeneralSettings();
public:
	// some virtual functions that will be overloaded from the base class
	virtual void loadSettings( KConfig* config );
	virtual void saveSettings( KConfig* config );

public:
	QColor m_removeColor;
	QColor m_changeColor;
	QColor m_addColor;
};

#endif
