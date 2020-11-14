/*
    SPDX-FileCopyrightText: 2001-2003 Otto Bruggeman <otto.bruggeman@home.nl>
    SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef VIEWSETTINGS_H
#define VIEWSETTINGS_H

#include <QColor>
#include <QWidget>

#include <libkomparediff2/difference.h>
#include <libkomparediff2/settingsbase.h>

#include "dialogpages_export.h"

class DIALOGPAGES_EXPORT ViewSettings : public SettingsBase
{
    Q_OBJECT
public:
    static const QColor default_removeColor;
    static const QColor default_changeColor;
    static const QColor default_addColor;
    static const QColor default_appliedColor;

    explicit ViewSettings(QWidget* parent);
    ~ViewSettings() override;
public:
    // some virtual functions that will be overloaded from the base class
    void loadSettings(KConfig* config) override;
    void saveSettings(KConfig* config) override;
    QColor colorForDifferenceType(int type, bool selected = false, bool applied = false);

public:
    QColor m_removeColor;
    QColor m_changeColor;
    QColor m_addColor;
    QColor m_appliedColor;
    QColor m_selectedRemoveColor;
    QColor m_selectedChangeColor;
    QColor m_selectedAddColor;
    QColor m_selectedAppliedColor;
    int    m_scrollNoOfLines;
    int    m_tabToNumberOfSpaces;

    QFont  m_font;
};

#endif // VIEWSETTINGS_H
