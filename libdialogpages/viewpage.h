/***************************************************************************
                                generalprefs.h
                                --------------
        begin                   : Sun Mar 4 2001
        Copyright 2001-2003 Otto Bruggeman <otto.bruggeman@home.nl>
        Copyright 2001-2003 John Firebaugh <jfirebaugh@kde.org>
        Copyright 2007      Kevin Kofler   <kevin.kofler@chello.at>
****************************************************************************/

/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***************************************************************************/

#ifndef VIEWPAGE_H
#define VIEWPAGE_H

#include "pagebase.h"
#include "dialogpagesexport.h"

class QFontComboBox;
class QSpinBox;

class KColorButton;
class KTabWidget;

class ViewSettings;

class DIALOGPAGES_EXPORT ViewPage : public PageBase
{
Q_OBJECT
public:
	ViewPage();
	~ViewPage();

public:
	void setSettings( ViewSettings* );
	ViewSettings* settings( void );

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
	KTabWidget*   m_tabWidget;
};

#endif
