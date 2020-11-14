/*
    SPDX-FileCopyrightText: 2001-2003 Otto Bruggeman <otto.bruggeman@home.nl>
    SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2007 Kevin Kofler <kevin.kofler@chello.at>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef VIEWPAGE_H
#define VIEWPAGE_H

#include <QFrame>

#include "dialogpages_export.h"

class QFontComboBox;
class QSpinBox;
class QTabWidget;

class KColorButton;

class ViewSettings;

class DIALOGPAGES_EXPORT ViewPage : public QFrame
{
    Q_OBJECT
public:
    ViewPage();
    ~ViewPage() override;

public:
    void setSettings(ViewSettings*);
    ViewSettings* settings();

public:
    ViewSettings* m_settings;

public:
    virtual void restore();
    virtual void apply();
    virtual void setDefaults();

public:
    KColorButton* m_removedColorButton;
    KColorButton* m_changedColorButton;
    KColorButton* m_addedColorButton;
    KColorButton* m_appliedColorButton;
    // snol == scroll number of lines
    QSpinBox*     m_snolSpinBox;
    QSpinBox*     m_tabSpinBox;
    QFontComboBox*   m_fontCombo;
    QSpinBox*     m_fontSizeSpinBox;
    QTabWidget*   m_tabWidget;
};

#endif
