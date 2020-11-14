/*
    SPDX-FileCopyrightText: 2001-2003 Otto Bruggeman <otto.bruggeman@home.nl>
    SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2007 Kevin Kofler <kevin.kofler@chello.at>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "viewsettings.h"

#include <QFont>
#include <QFontDatabase>

#include <KConfig>
#include <KConfigGroup>

using namespace Diff2;

const QColor ViewSettings::default_removeColor (190, 237, 190);
const QColor ViewSettings::default_changeColor (237, 190, 190);
const QColor ViewSettings::default_addColor    (190, 190, 237);
const QColor ViewSettings::default_appliedColor(237, 237, 190);

ViewSettings::ViewSettings(QWidget* parent)
    : SettingsBase(parent),
      m_removeColor(0, 0, 0),
      m_changeColor(0, 0, 0),
      m_addColor(0, 0, 0),
      m_appliedColor(0, 0, 0),
      m_scrollNoOfLines(0),
      m_tabToNumberOfSpaces(0)
{
}

ViewSettings::~ViewSettings()
{
}

void ViewSettings::loadSettings(KConfig* config)
{
    KConfigGroup cfg(config, "View Options");
    m_removeColor         = cfg.readEntry("RemoveColor",         default_removeColor);
    m_changeColor         = cfg.readEntry("ChangeColor",         default_changeColor);
    m_addColor            = cfg.readEntry("AddColor",            default_addColor);
    m_appliedColor        = cfg.readEntry("AppliedColor",        default_appliedColor);
    m_scrollNoOfLines     = cfg.readEntry("ScrollNoOfLines",     3);
    m_tabToNumberOfSpaces = cfg.readEntry("TabToNumberOfSpaces", 4);

    QFont stdFixed = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    stdFixed.setPointSize(10);
    m_font                = cfg.readEntry("TextFont", stdFixed);
}

void ViewSettings::saveSettings(KConfig* config)
{
    KConfigGroup cfg(config, "View Options");
    cfg.writeEntry("RemoveColor",         m_removeColor);
    cfg.writeEntry("ChangeColor",         m_changeColor);
    cfg.writeEntry("AddColor",            m_addColor);
    cfg.writeEntry("AppliedColor",        m_appliedColor);
    cfg.writeEntry("ScrollNoOfLines",     m_scrollNoOfLines);
    cfg.writeEntry("TabToNumberOfSpaces", m_tabToNumberOfSpaces);

    cfg.writeEntry("TextFont",            m_font);
}

QColor ViewSettings::colorForDifferenceType(int type, bool selected, bool applied)
{
    // FIXME: does not belong here
    QColor color;
    if (applied)
        color = m_appliedColor;
    else
    {
        type = type & 0xFFFFFFEF; // remove the AppliedByBlend
        switch (type) {
        case Difference::Unchanged: color = Qt::white; break;
        case Difference::Change:    color = m_changeColor; break;
        case Difference::Insert:    color = m_addColor; break;
        case Difference::Delete:    color = m_removeColor; break;
        default: break;
        }
    }

    if (selected)
        color = color.lighter(110);

    return color;
}
