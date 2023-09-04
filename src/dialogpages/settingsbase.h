/*
    SPDX-FileCopyrightText: 2001 Otto Bruggeman <otto.bruggeman@home.nl>
    SPDX-FileCopyrightText: 2001 John Firebaugh <jfirebaugh@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SETTINGSBASE_H
#define SETTINGSBASE_H

// lib
#include "dialogpages_export.h"
// Qt
#include <QObject>

class QWidget;
class KConfig;

class DIALOGPAGES_EXPORT SettingsBase : public QObject
{
    Q_OBJECT
public:
    explicit SettingsBase(QWidget* parent);
    ~SettingsBase() override;

public:
    virtual void loadSettings(KConfig* config);
    virtual void saveSettings(KConfig* config);
};

#endif
