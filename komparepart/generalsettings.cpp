/***************************************************************************
                                generalsettings.cpp  -  description
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

#include "generalsettings.h"

const QColor GeneralSettings::default_removeColor (190, 237, 190);
const QColor GeneralSettings::default_changeColor (237, 190, 190);
const QColor GeneralSettings::default_addColor    (190, 190, 237);
const QColor GeneralSettings::default_appliedColor(237, 237, 190);

GeneralSettings::GeneralSettings( QWidget* parent ) : SettingsBase( parent )
{

};

GeneralSettings::~GeneralSettings()
{

};

void GeneralSettings::loadSettings( KConfig* config )
{
	m_showEntireFile  = config->readBoolEntry ( "ShowEntireFile",  true );
	m_removeColor     = config->readColorEntry( "RemoveColor",     &default_removeColor );
	m_changeColor     = config->readColorEntry( "ChangeColor",     &default_changeColor );
	m_addColor        = config->readColorEntry( "AddColor",        &default_addColor );
	m_appliedColor    = config->readColorEntry( "AppliedColor",    &default_appliedColor );
	m_scrollNoOfLines = config->readNumEntry     ( "ScrollNoOfLines", 3 );

	SettingsBase::loadSettings( config );
};

void GeneralSettings::saveSettings( KConfig* config )
{
	config->writeEntry( "ShowEntireFile",  m_showEntireFile );
	config->writeEntry( "RemoveColor",     m_removeColor );
	config->writeEntry( "ChangeColor",     m_changeColor );
	config->writeEntry( "AddColor",        m_addColor );
	config->writeEntry( "AppliedColor",    m_appliedColor );
	config->writeEntry( "ScrollNoOfLines", m_scrollNoOfLines );

	SettingsBase::saveSettings( config );
};

QColor GeneralSettings::colorForDifferenceType( Difference::Type type, bool selected, bool applied )
{
	QColor color;
	if( applied ) {
		color = m_appliedColor;
	} else {
		switch( type ) {
			case Difference::Unchanged: color = white; break;
			case Difference::Change:    color = m_changeColor; break;
			case Difference::Insert:    color = m_addColor; break;
			case Difference::Delete:    color = m_removeColor; break;
			default: break;
		}
	}
	
	if( selected ) {
		color = color.light( 105 );
	}

	return color;
};

#include "generalsettings.moc"
