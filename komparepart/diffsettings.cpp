/***************************************************************************
				diffsettings.cpp  -  description
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

#include "diffsettings.h"

#include "diffsettings.moc"

DiffSettings::DiffSettings( QWidget* parent ) : SettingsBase( parent )
{

};

DiffSettings::~DiffSettings()
{
	
};

void DiffSettings::loadSettings( KConfig* config )
{
	m_linesOfContext = config->readNumEntry( "LinesOfContext", 3 );
	m_useContextDiff = config->readBoolEntry( "UseContextDiff", false );
	m_useNormalDiff = config->readBoolEntry( "UseNormalDiff", false );
	m_useUnifiedDiff = config->readBoolEntry( "UseUnifiedDiff", true );
	m_useRCSDiff = config->readBoolEntry( "UseRCSDiff", false );
	m_largeFiles = config->readBoolEntry( "LargeFiles", true );
	m_ignoreWhiteSpace = config->readBoolEntry( "IgnoreWhiteSpace", false );
	m_ignoreEmptyLines = config->readBoolEntry( "IgnoreEmptyLines", false );
	m_ignoreChangesInCase = config->readBoolEntry( "IgnoreChangesInCase", false );
	m_ignoreWhitespaceComparingLines = config->readBoolEntry( "IgnoreWhitespaceComparingLines", false );
	m_createSmallerDiff = config->readBoolEntry( "CreateSmallerDiff", true );
	m_convertTabsToSpaces = config->readBoolEntry( "ConvertTabsToSpaces", false );
	m_showCFunctionChange = config->readBoolEntry( "ShowCFunctionChange", false );
};

void DiffSettings::saveSettings( KConfig* config )
{
	config->writeEntry( "LinesOfContext", m_linesOfContext );
	config->writeEntry( "UseNormalDiff", m_useNormalDiff );
	config->writeEntry( "UseContextDiff", m_useContextDiff );
	config->writeEntry( "UseUnifiedDiff", m_useUnifiedDiff );
	config->writeEntry( "UseRCSDiff", m_useRCSDiff );
	config->writeEntry( "LargeFiles", m_largeFiles );
	config->writeEntry( "IgnoreWhiteSpace", m_ignoreWhiteSpace );
	config->writeEntry( "IgnoreEmptyLines", m_ignoreEmptyLines );
	config->writeEntry( "IgnoreChangesInCase", m_ignoreChangesInCase );
	config->writeEntry( "IgnoreWhitespacecomparingLines", m_ignoreWhitespaceComparingLines );
	config->writeEntry( "CreateSmallerDiff", m_createSmallerDiff );
	config->writeEntry( "ConvertTabsToSpaces", m_convertTabsToSpaces );
	config->writeEntry( "ShowCFunctionChange", m_showCFunctionChange );
};
