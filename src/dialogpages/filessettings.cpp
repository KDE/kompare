/*
    SPDX-FileCopyrightText: 2004 Otto Bruggeman <otto.bruggeman@home.nl>
    SPDX-FileCopyrightText: 2007 Kevin Kofler <kevin.kofler@chello.at>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "filessettings.h"

#include <KConfig>
#include <KConfigGroup>

FilesSettings::FilesSettings(QWidget* parent)
    : SettingsBase(parent)
{
}

FilesSettings::~FilesSettings()
{
}

void FilesSettings::loadSettings(KConfig* config)
{
    KConfigGroup group(config, m_configGroupName);

    m_recentSources            = group.readEntry("Recent Sources", QStringList());
    m_lastChosenSourceURL      = group.readEntry("LastChosenSourceListEntry", QString());
    m_recentDestinations       = group.readEntry("Recent Destinations", QStringList());
    m_lastChosenDestinationURL = group.readEntry("LastChosenDestinationListEntry", QString());
    m_encoding                 = group.readEntry("Encoding", "default");
}

void FilesSettings::saveSettings(KConfig* config)
{
    KConfigGroup group(config, m_configGroupName);
    group.writeEntry("Recent Sources", m_recentSources);
    group.writeEntry("Recent Destinations", m_recentDestinations);
    group.writeEntry("LastChosenSourceListEntry", m_lastChosenSourceURL);
    group.writeEntry("LastChosenDestinationListEntry", m_lastChosenDestinationURL);
    group.writeEntry("Encoding", m_encoding);
    config->sync();
}

void FilesSettings::setGroup(const QString& groupName)
{
    m_configGroupName = groupName;
}
