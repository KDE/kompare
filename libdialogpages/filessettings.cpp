/***************************************************************************
				filessettings.cpp  -  description
				-------------------
	begin			: Sun Apr 18 2004
	copyright		: (C) 2004 Otto Bruggeman
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

#include <kapplication.h>
#include <kconfig.h>

#include "filessettings.h"

FilesSettings::FilesSettings( QWidget* parent )
	: SettingsBase( parent )
{
}

FilesSettings::~FilesSettings()
{
}

void FilesSettings::loadSettings( KConfig* config )
{
	config->setGroup( m_configGroupName );

	m_recentSources            = config->readListEntry( "Recent Sources" );
	m_lastChosenSourceURL      = config->readEntry    ( "LastChosenSourceListEntry", "" );
	m_recentDestinations       = config->readListEntry( "Recent Destinations" );
	m_lastChosenDestinationURL = config->readEntry    ( "LastChosenDestinationListEntry", "" );
	m_encoding                 = config->readEntry    ( "Encoding", "default" );
}

void FilesSettings::saveSettings( KConfig* config )
{
	config->setGroup( m_configGroupName );
	config->writeEntry( "Recent Sources", m_recentSources );
	config->writeEntry( "Recent Destinations", m_recentDestinations );
	config->writeEntry( "LastChosenSourceListEntry", m_lastChosenSourceURL );
	config->writeEntry( "LastChosenDestinationListEntry", m_lastChosenDestinationURL );
	config->writeEntry( "Encoding", m_encoding );
	config->sync();
}

void FilesSettings::setGroup( const QString& groupName )
{
	m_configGroupName = groupName;
}

#include "filessettings.moc"
