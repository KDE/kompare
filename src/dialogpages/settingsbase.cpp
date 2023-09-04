/*
    SPDX-FileCopyrightText: 2001 Otto Bruggeman <otto.bruggeman@home.nl>
    SPDX-FileCopyrightText: 2001 John Firebaugh <jfirebaugh@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "settingsbase.h"

// Qt
#include <QWidget>

SettingsBase::SettingsBase(QWidget* parent) : QObject(parent)
{

}

SettingsBase::~SettingsBase()
{

}

void SettingsBase::loadSettings(KConfig* /* config */)
{
}

void SettingsBase::saveSettings(KConfig* /* config */)
{
}

#include "moc_settingsbase.cpp"
