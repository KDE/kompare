/***************************************************************************
                                viewprefs.cpp  -  description
                                -------------------
        begin                   : Sun Mar 4 2001
        copyright               : (C) 2001-2002 by Otto Bruggeman
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

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qhgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qspinbox.h>

#include <kcolorbutton.h>
#include <kdialog.h>
#include <klocale.h>

#include "viewprefs.h"
#include "viewsettings.h"

ViewPrefs::ViewPrefs( QWidget* parent ) : PrefsBase( parent )
{
	QWidget*     page;
	QVBoxLayout* layout;
	QGroupBox*   colorGroupBox;
	QHGroupBox*  snolGroupBox;
	QHGroupBox*  tabGroupBox;
	QLabel*      label;

	page   = new QWidget( this );
	layout = new QVBoxLayout( page );
	layout->setSpacing( KDialog::spacingHint() );
	layout->setMargin( KDialog::marginHint() );

	// add a groupbox
	colorGroupBox = new QGroupBox( 2, Qt::Horizontal, i18n( "Colors" ), page );
	layout->addWidget( colorGroupBox );
	colorGroupBox->setMargin( KDialog::marginHint() );

	// add the removeColor
	label = new QLabel( i18n( "Removed color:" ), colorGroupBox );
	m_removedColorButton = new KColorButton( colorGroupBox );
	label->setBuddy( m_removedColorButton );

	// add the changeColor
	label = new QLabel( i18n( "Changed color:" ), colorGroupBox );
	m_changedColorButton = new KColorButton( colorGroupBox );
	label->setBuddy( m_changedColorButton );

	// add the addColor
	label = new QLabel( i18n( "Added color:" ), colorGroupBox );
	m_addedColorButton = new KColorButton( colorGroupBox );
	label->setBuddy( m_addedColorButton );

	// add the appliedColor
	label = new QLabel( i18n( "Applied color:" ), colorGroupBox );
	m_appliedColorButton = new KColorButton( colorGroupBox );
	label->setBuddy( m_appliedColorButton );

	// add the show entire file checkbox
	m_showEntireFile = new QCheckBox( i18n( "Show entire file when comparing" ), page );
	layout->addWidget( m_showEntireFile );

	// scroll number of lines (snol)
	snolGroupBox = new QHGroupBox( i18n( "Mouse Wheel" ), page );
	layout->addWidget( snolGroupBox );
	snolGroupBox->setMargin( KDialog::marginHint() );

	label            = new QLabel( i18n( "Number of lines:" ), snolGroupBox );
	m_snolSpinBox    = new QSpinBox( 0, 50, 1, snolGroupBox );
	label->setBuddy( m_snolSpinBox );

	// Temporarily here for testing...
	// number of spaces for a tab character stuff
	tabGroupBox = new QHGroupBox( i18n( "Tabs To Spaces" ), page );
	layout->addWidget( tabGroupBox );
	tabGroupBox->setMargin( KDialog::marginHint() );

	label = new QLabel( i18n( "Number of spaces to convert a tab character to:" ), tabGroupBox );
	m_tabSpinBox = new QSpinBox( 1, 16, 1, tabGroupBox );
	label->setBuddy( m_tabSpinBox );

	layout->addStretch( 1 );
	page->setMinimumSize( sizeHintForWidget( page ) );

	addTab( page, i18n( "A&ppearance" ) );
}

ViewPrefs::~ViewPrefs()
{

}

void ViewPrefs::setSettings( ViewSettings* setts )
{
	m_settings = setts;

	m_showEntireFile->setChecked  ( m_settings->m_showEntireFile );
	m_addedColorButton->setColor  ( m_settings->m_addColor );
	m_changedColorButton->setColor( m_settings->m_changeColor );
	m_removedColorButton->setColor( m_settings->m_removeColor );
	m_appliedColorButton->setColor( m_settings->m_appliedColor );
	m_snolSpinBox->setValue       ( m_settings->m_scrollNoOfLines );
	m_tabSpinBox->setValue        ( m_settings->m_tabToNumberOfSpaces );
}

ViewSettings* ViewPrefs::settings( void )
{
	return m_settings;
}

void ViewPrefs::restore()
{
}

void ViewPrefs::apply()
{
	ViewSettings* setts;
	setts = settings();

	setts->m_showEntireFile      = m_showEntireFile->isChecked();
	setts->m_addColor            = m_addedColorButton->color();
	setts->m_changeColor         = m_changedColorButton->color();
	setts->m_removeColor         = m_removedColorButton->color();
	setts->m_appliedColor        = m_appliedColorButton->color();
	setts->m_scrollNoOfLines     = m_snolSpinBox->value();
	setts->m_tabToNumberOfSpaces = m_tabSpinBox->value();

	setts->emitSettingsChanged();
}

void ViewPrefs::setDefaults()
{
	m_showEntireFile->setChecked  ( true );
	m_addedColorButton->setColor  ( ViewSettings::default_addColor );
	m_changedColorButton->setColor( ViewSettings::default_changeColor );
	m_removedColorButton->setColor( ViewSettings::default_removeColor );
	m_appliedColorButton->setColor( ViewSettings::default_appliedColor );
	m_snolSpinBox->setValue       ( 3 );
}

#include "viewprefs.moc"
