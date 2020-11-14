/*
    SPDX-FileCopyrightText: 2001-2004 Otto Bruggeman <otto.bruggeman@home.nl>
    SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2007 Kevin Kofler <kevin.kofler@chello.at>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DIFFPAGE_H
#define DIFFPAGE_H

#include <QFrame>

#include "dialogpages_export.h"

class QCheckBox;
class QDialog;
class QSpinBox;
class QStringList;
class QButtonGroup;
class QGroupBox;
class QTabWidget;

class KLineEdit;
class KEditListWidget;
class KUrlComboBox;
class KUrlRequester;

class DiffSettings;

class DIALOGPAGES_EXPORT DiffPage : public QFrame
{
    Q_OBJECT
public:
    DiffPage();
    ~DiffPage() override;

public:
    void setSettings(DiffSettings*);
    DiffSettings* settings();

public:
    virtual void restore();
    virtual void apply();
    virtual void setDefaults();

protected Q_SLOTS:
    void slotExcludeFilePatternToggled(bool);
    void slotExcludeFileToggled(bool);

private:
    void addDiffTab();
    void addFormatTab();
    void addOptionsTab();
    void addExcludeTab();

public:
    DiffSettings*  m_settings;

    KUrlRequester* m_diffURLRequester;

    QCheckBox*     m_newFilesCheckBox;
    QCheckBox*     m_smallerCheckBox;
    QCheckBox*     m_largerCheckBox;
    QCheckBox*     m_tabsCheckBox;
    QCheckBox*     m_caseCheckBox;
    QCheckBox*     m_linesCheckBox;
    QCheckBox*     m_whitespaceCheckBox;
    QCheckBox*     m_allWhitespaceCheckBox;
    QCheckBox*     m_ignoreTabExpansionCheckBox;

    QCheckBox*     m_ignoreRegExpCheckBox;
    KLineEdit*     m_ignoreRegExpEdit;
    QStringList*   m_ignoreRegExpEditHistory;
    QDialog*       m_ignoreRegExpDialog;

    QGroupBox*     m_excludeFilePatternGroupBox;
    KEditListWidget*  m_excludeFilePatternEditListBox;

    QGroupBox*     m_excludeFileNameGroupBox;
    KUrlComboBox*  m_excludeFileURLComboBox;
    KUrlRequester* m_excludeFileURLRequester;

    // loc == lines of context
    QSpinBox*      m_locSpinBox;

    QButtonGroup*  m_modeButtonGroup;
    QGroupBox*     m_diffProgramGroup;

    QTabWidget*    m_tabWidget;
};

#endif
