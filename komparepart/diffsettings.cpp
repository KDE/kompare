/***************************************************************************
				diffsettings.cpp  -  description
				-------------------
	begin			: Sun Mar 4 2001
	copyright		: (C) 2001-2004 Otto Bruggeman
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

#include <kconfig.h>
#include "diffsettings.h"

DiffSettings::DiffSettings( QWidget* parent )
	: SettingsBase( parent ),
	m_diffProgram( "" ),
	m_linesOfContext( 0 ),
	m_format( Kompare::Unified ),
	m_largeFiles( false ),
	m_ignoreWhiteSpace( false ),
	m_ignoreAllWhiteSpace( false ),
	m_ignoreEmptyLines( false ),
	m_ignoreChangesDueToTabExpansion( false ),
	m_createSmallerDiff( false ),
	m_ignoreChangesInCase( false ),
	m_showCFunctionChange( false ),
	m_convertTabsToSpaces( false ),
	m_ignoreRegExp( false ),
	m_ignoreRegExpText( "" ),
	m_ignoreRegExpTextHistory( "" ),
	m_recursive( false ),
	m_newFiles( false ),
	m_excludeFilePattern( false ),
	m_excludeFilePatternList( "" ),
	m_excludeFilesFile( false ),
	m_excludeFilesFileURL( "" ),
	m_excludeFilesFileHistoryList( "" )
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
	m_ignoreAllWhiteSpace            = config->readBoolEntry( "IgnoreAllWhiteSpace", false );
	m_ignoreEmptyLines               = config->readBoolEntry( "IgnoreEmptyLines", false );
	m_ignoreChangesDueToTabExpansion = config->readBoolEntry( "IgnoreChangesDueToTabExpansion", false );
	m_ignoreChangesInCase            = config->readBoolEntry( "IgnoreChangesInCase", false );
	m_ignoreRegExp                   = config->readBoolEntry( "IgnoreRegExp", false );
	m_ignoreRegExpText               = config->readEntry    ( "IgnoreRegExpText", "" );
	m_ignoreRegExpTextHistory        = config->readListEntry( "IgnoreRegExpTextHistory" );
	m_createSmallerDiff              = config->readBoolEntry( "CreateSmallerDiff", true );
	m_convertTabsToSpaces            = config->readBoolEntry( "ConvertTabsToSpaces", false );
	m_showCFunctionChange            = config->readBoolEntry( "ShowCFunctionChange", false );
	m_recursive                      = config->readBoolEntry( "CompareRecursively", true );
	m_newFiles                       = config->readBoolEntry( "NewFiles", true );

	m_format = static_cast<Kompare::Format>( config->readNumEntry( "Format", Kompare::Unified ) );

	config->setGroup( "Exclude File Options" );
	m_excludeFilePattern             = config->readBoolEntry( "Pattern", false );
	m_excludeFilePatternList         = config->readListEntry( "PatternList" );
	m_excludeFilesFile               = config->readBoolEntry( "File", false );
	m_excludeFilesFileURL            = config->readEntry( "FileURL", "" );
	m_excludeFilesFileHistoryList    = config->readListEntry( "FileHistoryList" );
}

void DiffSettings::saveSettings( KConfig* config )
{
	config->setGroup( "Diff Options" );
	config->writeEntry( "DiffProgram",                    m_diffProgram );
	config->writeEntry( "LinesOfContext",                 m_linesOfContext );
	config->writeEntry( "Format",                         m_format );
	config->writeEntry( "LargeFiles",                     m_largeFiles );
	config->writeEntry( "IgnoreWhiteSpace",               m_ignoreWhiteSpace );
	config->writeEntry( "IgnoreAllWhiteSpace",            m_ignoreAllWhiteSpace );
	config->writeEntry( "IgnoreEmptyLines",               m_ignoreEmptyLines );
	config->writeEntry( "IgnoreChangesInCase",            m_ignoreChangesInCase );
	config->writeEntry( "IgnoreChangesDueToTabExpansion", m_ignoreChangesDueToTabExpansion );
	config->writeEntry( "IgnoreRegExp",                   m_ignoreRegExp );
	config->writeEntry( "IgnoreRegExpText",               m_ignoreRegExpText );
	config->writeEntry( "IgnoreRegExpTextHistory",        m_ignoreRegExpTextHistory );
	config->writeEntry( "CreateSmallerDiff",              m_createSmallerDiff );
	config->writeEntry( "ConvertTabsToSpaces",            m_convertTabsToSpaces );
	config->writeEntry( "ShowCFunctionChange",            m_showCFunctionChange );
	config->writeEntry( "CompareRecursively",             m_recursive );
	config->writeEntry( "NewFiles",                       m_newFiles );

	config->setGroup( "Exclude File Options" );
	config->writeEntry( "Pattern",            m_excludeFilePattern );
	config->writeEntry( "PatternList",        m_excludeFilePatternList );
	config->writeEntry( "File",               m_excludeFilesFile );
	config->writeEntry( "FileURL",            m_excludeFilesFileURL );
	config->writeEntry( "FileHistoryList",    m_excludeFilesFileHistoryList );
}

#include "diffsettings.moc"
