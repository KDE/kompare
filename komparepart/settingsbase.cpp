/***************************************************************************
				settingsbase.cpp  -  description
				-------------------
	begin			: Sun Mar 4 2001
	copyright			: (C) 2001 by Otto Bruggeman
	email			: otto.bruggeman@home.nl
****************************************************************************/

/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
****************************************************************************/

#include "settingsbase.h"

#include "settingsbase.moc"

SettingsBase::SettingsBase( QWidget* parent ) : QObject( parent )
{

};

SettingsBase::~SettingsBase()
{

};

void SettingsBase::loadSettings( KConfig* config )
{
	// nonsens statement to get rid of a compiler warning...
	config = config;
};

void SettingsBase::saveSettings( KConfig* config )
{
	// nonsens statement to get rid of a compiler warning...
	config = config;
};
