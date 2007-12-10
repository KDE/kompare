/***************************************************************************
                                diffprefs.h  -  description
                                -------------------
        begin                   : Sun Mar 4 2001
        copyright               : (C) 2001-2004 Otto Bruggeman
                                  (C) 2001-2003 John Firebaugh
        email                   : otto.bruggeman@home.nl
                                  jfirebaugh@kde.org
****************************************************************************/

/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***************************************************************************/

#ifndef DIFFPAGE_H
#define DIFFPAGE_H

#include "pagebase.h"

class QCheckBox;
class QDialog;
class QSpinBox;
class QStringList;
class KButtonGroup;
class QWidget;
class QTabWidget;

class KLineEdit;
class KComboBox;
class KEditListBox;
class KUrlComboBox;
class KUrlRequester;

class DiffSettings;

class DiffPage : public PageBase
{
Q_OBJECT
public:
	DiffPage();
	~DiffPage();

public:
	void setSettings( DiffSettings* );
	DiffSettings* settings( void );

public:
	virtual void restore();
	virtual void apply();
	virtual void setDefaults();

protected slots:
	void slotShowRegExpEditor();
	void slotExcludeFilePatternToggled( bool );
	void slotExcludeFileToggled( bool );

private:
	void addDiffTab();
	void addFormatTab();
	void addOptionsTab();
	void addExcludeTab();

public:
	DiffSettings*  m_settings;

	KUrlRequester* m_diffURLRequester;

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

	QCheckBox*     m_excludeFilePatternCheckBox;
	KEditListBox*  m_excludeFilePatternEditListBox;
	
	QCheckBox*     m_excludeFileCheckBox;
	KUrlComboBox*  m_excludeFileURLComboBox;
	KUrlRequester* m_excludeFileURLRequester;

	// loc == lines of context
	QSpinBox*      m_locSpinBox;

	KButtonGroup* m_modeButtonGroup;
	KButtonGroup* m_diffProgramGroup;

	QTabWidget*      m_tabWidget;
};

#endif
