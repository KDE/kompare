/***************************************************************************
				diffsettings.h  -  description
				-------------------
	begin			: Sun Mar 4 2001
	copyright			: (C) 2001 by Otto Bruggeman
	email			: otto.bruggeman@home.nl
****************************************************************************/

/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
****************************************************************************/

#ifndef DIFFSETTINGS_H
#define DIFFSETTINGS_H

#include <qwidget.h>

#include "settingsbase.h"

class DiffSettings : public SettingsBase
{
Q_OBJECT
public:
	DiffSettings( QWidget* parent );
	~DiffSettings();
public:
	// some virtual functions that will be overloaded from the base class
	virtual void loadSettings( KConfig* config );
	virtual void saveSettings( KConfig* config );

public:
	int m_linesOfContext;
	bool m_useUnifiedDiff;
	bool m_useContextDiff;
	bool m_useNormalDiff;
	bool m_useRCSDiff;
	bool m_useEdDiff;
	bool m_largeFiles;
	bool m_ignoreWhiteSpace;
	bool m_ignoreEmptyLines;
	bool m_createSmallerDiff;
	bool m_ignoreChangesInCase;
	bool m_showCFunctionChange;
	bool m_convertTabsToSpaces;
	bool m_ignoreWhitespaceComparingLines;
};

#endif
