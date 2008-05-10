/***************************************************************************
                                filessettings.cpp
                                -----------------
        begin                   : Sun Apr 18 2004
        Copyright 2004 Otto Bruggeman <otto.bruggeman@home.nl>
        Copyright 2007 Kevin Kofler   <kevin.kofler@chello.at>
****************************************************************************/

/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
****************************************************************************/

#include "filessettings.h"

#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>

FilesSettings::FilesSettings( QWidget* parent )
	: SettingsBase( parent )
{
}

FilesSettings::~FilesSettings()
{
}

void FilesSettings::loadSettings( KConfig* config )
{
	KConfigGroup group( config, m_configGroupName );

	m_recentSources            = group.readEntry( "Recent Sources", QStringList() );
	m_lastChosenSourceURL      = group.readEntry    ( "LastChosenSourceListEntry", "" );
	m_recentDestinations       = group.readEntry( "Recent Destinations", QStringList() );
	m_lastChosenDestinationURL = group.readEntry    ( "LastChosenDestinationListEntry", "" );
	m_encoding                 = group.readEntry    ( "Encoding", "default" );
}

void FilesSettings::saveSettings( KConfig* config )
{
	KConfigGroup group( config, m_configGroupName );
	group.writeEntry( "Recent Sources", m_recentSources );
	group.writeEntry( "Recent Destinations", m_recentDestinations );
	group.writeEntry( "LastChosenSourceListEntry", m_lastChosenSourceURL );
	group.writeEntry( "LastChosenDestinationListEntry", m_lastChosenDestinationURL );
	group.writeEntry( "Encoding", m_encoding );
	config->sync();
}

void FilesSettings::setGroup( const QString& groupName )
{
	m_configGroupName = groupName;
}

#include "filessettings.moc"
