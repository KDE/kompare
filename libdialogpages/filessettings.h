/***************************************************************************
				filessettings.h
				---------------
	begin			: Sun Apr 18 2004
	Copyright 2004 Otto Bruggeman <otto.bruggeman@home.nl>
****************************************************************************/

/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
****************************************************************************/

#ifndef FILESSETTINGS_H
#define FILESSETTINGS_H

#include <QString>
#include <QStringList>

#include <libkomparediff2/settingsbase.h>

#include "dialogpages_export.h"

class KConfig;

class DIALOGPAGES_EXPORT FilesSettings : public SettingsBase
{
Q_OBJECT
public:
	FilesSettings( QWidget* parent );
	~FilesSettings() override;

public:
	// some virtual functions that will be overloaded from the base class
	void loadSettings( KConfig* config ) override;
	void saveSettings( KConfig* config ) override;

	void setGroup( const QString& groupName );

public:
	QString     m_configGroupName;

	QStringList m_recentSources;
	QString     m_lastChosenSourceURL;
	QStringList m_recentDestinations;
	QString     m_lastChosenDestinationURL;
	QString     m_encoding;
};

#endif // FILESSETTINGS_H

