/***************************************************************************
                          miscsettings.cpp  -  description
                             -------------------
    begin                : Sun Mar 4 2001
    copyright            : (C) 2001 by Otto Bruggeman
    email                : otto.bruggeman@home.nl
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "miscsettings.h"

#include "miscsettings.moc"

MiscSettings::MiscSettings( QWidget* parent ) : SettingsBase( parent )
{

};

MiscSettings::~MiscSettings()
{

};

void MiscSettings::loadSettings( KConfig* config )
{
config = config;
};

void MiscSettings::saveSettings( KConfig* config )
{
config = config;
};
