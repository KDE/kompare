/***************************************************************************
				generalsettings.cpp  -  description
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

#include "generalsettings.h"

#include "generalsettings.moc"

GeneralSettings::GeneralSettings( QWidget* parent ) : SettingsBase( parent )
{

};

GeneralSettings::~GeneralSettings()
{

};

void GeneralSettings::loadSettings( KConfig* config )
{
	m_removeColor = config->readColorEntry( "RemoveColor", new QColor(190, 237, 190) );
	m_changeColor = config->readColorEntry( "ChangeColor", new QColor(237, 190, 190) );
	m_addColor = config->readColorEntry( "AddColor", new QColor(190, 190, 237) );
};

void GeneralSettings::saveSettings( KConfig* config )
{
	config->writeEntry( "RemoveColor", m_removeColor );
	config->writeEntry( "ChangeColor", m_changeColor );
	config->writeEntry( "AddColor", m_addColor );
};
