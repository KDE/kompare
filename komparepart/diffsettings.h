/***************************************************************************
                                diffsettings.h  -  description
                                -------------------
        begin                   : Sun Mar 4 2001
        copyright               : (C) 2001 by Otto Bruggeman
                                  and John Firebaugh
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

#ifndef DIFFSETTINGS_H
#define DIFFSETTINGS_H

#include <qwidget.h>

#include "kompare.h"
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
	Format m_format;
	bool m_largeFiles;                         // -H
	bool m_ignoreWhiteSpace;                   // -b
	bool m_ignoreEmptyLines;                   // -B
	bool m_createSmallerDiff;                  // -d
	bool m_ignoreChangesInCase;                // -i
	bool m_showCFunctionChange;                // -p
	bool m_convertTabsToSpaces;                // -t
	bool m_ignoreWhitespaceComparingLines;     // -w
	bool m_recursive;                          // -r
	bool m_newFiles;                           // -N
	bool m_allText;                            // -a
};

#endif
