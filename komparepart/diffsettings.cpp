/***************************************************************************
				diffsettings.cpp  -  description
				-------------------
	begin			: Sun Mar 4 2001
	copyright		: (C) 2001-2003 by Otto Bruggeman
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

DiffSettings::DiffSettings( QWidget* parent ) : SettingsBase( parent )
{
}

DiffSettings::~DiffSettings()
{
}

void DiffSettings::loadSettings( KConfig* config )
{
	config->setGroup( "Diff Options" );
	m_diffProgram                    = config->readEntry    ( "DiffProgram", "" );
	m_linesOfContext                 = config->readNumEntry ( "LinesOfContext", 3 );
	m_largeFiles                     = config->readBoolEntry( "LargeFiles", true );
	m_ignoreWhiteSpace               = config->readBoolEntry( "IgnoreWhiteSpace", false );
	m_ignoreEmptyLines               = config->readBoolEntry( "IgnoreEmptyLines", false );
	m_ignoreChangesInCase            = config->readBoolEntry( "IgnoreChangesInCase", false );
	m_ignoreWhitespaceComparingLines = config->readBoolEntry( "IgnoreWhitespaceComparingLines", false );
	m_ignoreRegExp                   = config->readBoolEntry( "IgnoreRegExp", false );
	m_ignoreRegExpText               = config->readEntry    ( "IgnoreRegExpText", "" );
	m_ignoreRegExpTextHistory        = config->readListEntry( "IgnoreRegExpTextHistory" );
	m_createSmallerDiff              = config->readBoolEntry( "CreateSmallerDiff", true );
	m_convertTabsToSpaces            = config->readBoolEntry( "ConvertTabsToSpaces", false );
	m_showCFunctionChange            = config->readBoolEntry( "ShowCFunctionChange", false );
	m_recursive                      = config->readBoolEntry( "CompareRecursively", true );
	m_newFiles                       = config->readBoolEntry( "NewFiles", true );
	m_allText                        = config->readBoolEntry( "TreatAllFilesAsText", true );

	m_format = static_cast<Kompare::Format>( config->readNumEntry( "Format", Kompare::Unified ) );
}

void DiffSettings::saveSettings( KConfig* config )
{
	config->setGroup( "Diff Options" );
	config->writeEntry( "DiffProgram",                    m_diffProgram );
	config->writeEntry( "LinesOfContext",                 m_linesOfContext );
	config->writeEntry( "Format",                         m_format );
	config->writeEntry( "LargeFiles",                     m_largeFiles );
	config->writeEntry( "IgnoreWhiteSpace",               m_ignoreWhiteSpace );
	config->writeEntry( "IgnoreEmptyLines",               m_ignoreEmptyLines );
	config->writeEntry( "IgnoreChangesInCase",            m_ignoreChangesInCase );
	config->writeEntry( "IgnoreWhitespaceComparingLines", m_ignoreWhitespaceComparingLines );
	config->writeEntry( "IgnoreRegExp",                   m_ignoreRegExp );
	config->writeEntry( "IgnoreRegExpText",               m_ignoreRegExpText );
	config->writeEntry( "IgnoreRegExpTextHistory",        m_ignoreRegExpTextHistory );
	config->writeEntry( "CreateSmallerDiff",              m_createSmallerDiff );
	config->writeEntry( "ConvertTabsToSpaces",            m_convertTabsToSpaces );
	config->writeEntry( "ShowCFunctionChange",            m_showCFunctionChange );
	config->writeEntry( "CompareRecursively",             m_recursive );
	config->writeEntry( "NewFiles",                       m_newFiles );
	config->writeEntry( "TreatAllFilesAsText",            m_allText );
}

#include "diffsettings.moc"
