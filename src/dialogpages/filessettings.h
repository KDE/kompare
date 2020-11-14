/*
    SPDX-FileCopyrightText: 2004 Otto Bruggeman <otto.bruggeman@home.nl>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
    explicit FilesSettings(QWidget* parent);
    ~FilesSettings() override;

public:
    // some virtual functions that will be overloaded from the base class
    void loadSettings(KConfig* config) override;
    void saveSettings(KConfig* config) override;

    void setGroup(const QString& groupName);

public:
    QString     m_configGroupName;

    QStringList m_recentSources;
    QString     m_lastChosenSourceURL;
    QStringList m_recentDestinations;
    QString     m_lastChosenDestinationURL;
    QString     m_encoding;
};

#endif // FILESSETTINGS_H

