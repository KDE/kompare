/***************************************************************************
                                viewprefs.cpp
                                -------------
        begin                   : Sun Mar 4 2001
        Copyright 2001-2002 Otto Bruggeman <otto.bruggeman@home.nl>
        Copyright 2001-2002 John Firebaugh <jfirebaugh@kde.org>
        Copyright 2007-2011 Kevin Kofler   <kevin.kofler@chello.at>
****************************************************************************/

/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***************************************************************************/

#include "viewpage.h"

#include <QtGui/QCheckBox>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QFontComboBox>

#include <kapplication.h>
#include <kcolorbutton.h>
#include <kdialog.h>
#include <klocale.h>
#include <kglobal.h>
#include <ktabwidget.h>

#include "viewsettings.h"

ViewPage::ViewPage() : PageBase()
{
	QWidget*     page;
	QVBoxLayout* layout;
	QGridLayout* gridLayout;
	QGroupBox*   colorGroupBox;
	QGroupBox*   snolGroupBox;
	QGroupBox*   tabGroupBox;
	QLabel*      label;

	m_tabWidget = new KTabWidget( this );
	page   = new QWidget( m_tabWidget );
	layout = new QVBoxLayout( page );
	layout->setSpacing( KDialog::spacingHint() );
	layout->setMargin( KDialog::marginHint() );

	// add a groupbox
	colorGroupBox = new QGroupBox( page );
	colorGroupBox->setTitle( i18n( "Colors" ) );
	layout->addWidget( colorGroupBox );
	//colorGroupBox->setMargin( KDialog::marginHint() );
	gridLayout = new QGridLayout( colorGroupBox );

	// add the removeColor
	label = new QLabel( i18n( "Removed color:" ), colorGroupBox );
	m_removedColorButton = new KColorButton( colorGroupBox );
	label->setBuddy( m_removedColorButton );
	gridLayout->addWidget( label, 0, 0 );
	gridLayout->addWidget( m_removedColorButton, 0, 1 );

	// add the changeColor
	label = new QLabel( i18n( "Changed color:" ), colorGroupBox );
	m_changedColorButton = new KColorButton( colorGroupBox );
	label->setBuddy( m_changedColorButton );
	gridLayout->addWidget( label, 1, 0 );
	gridLayout->addWidget( m_changedColorButton, 1, 1 );

	// add the addColor
	label = new QLabel( i18n( "Added color:" ), colorGroupBox );
	m_addedColorButton = new KColorButton( colorGroupBox );
	label->setBuddy( m_addedColorButton );
	gridLayout->addWidget( label, 2, 0 );
	gridLayout->addWidget( m_addedColorButton, 2, 1 );

	// add the appliedColor
	label = new QLabel( i18n( "Applied color:" ), colorGroupBox );
	m_appliedColorButton = new KColorButton( colorGroupBox );
	label->setBuddy( m_appliedColorButton );
	gridLayout->addWidget( label, 3, 0 );
	gridLayout->addWidget( m_appliedColorButton, 3, 1 );

	// scroll number of lines (snol)
	snolGroupBox = new QGroupBox( page );
        QHBoxLayout *snolLayout = new QHBoxLayout;
        snolGroupBox->setLayout( snolLayout );
	snolGroupBox->setTitle( i18n( "Mouse Wheel" ) );
	layout->addWidget( snolGroupBox );
	//snolGroupBox->setMargin( KDialog::marginHint() );

	label            = new QLabel( i18n( "Number of lines:" ) );
        snolLayout->addWidget( label );
	m_snolSpinBox    = new QSpinBox( snolGroupBox );
	m_snolSpinBox->setRange( 0, 50 );
        snolLayout->addWidget( m_snolSpinBox );
	label->setBuddy( m_snolSpinBox );

	// Temporarily here for testing...
	// number of spaces for a tab character stuff
	tabGroupBox = new QGroupBox( page );
        QHBoxLayout *tabLayout = new QHBoxLayout;
        tabGroupBox->setLayout( tabLayout );
	tabGroupBox->setTitle( i18n( "Tabs to Spaces" ) );
	layout->addWidget( tabGroupBox );
	//tabGroupBox->setMargin( KDialog::marginHint() );

	label = new QLabel( i18n( "Number of spaces to convert a tab character to:" ) );
        tabLayout->addWidget( label );
	m_tabSpinBox = new QSpinBox( tabGroupBox );
	m_tabSpinBox->setRange( 1, 16 );
        tabLayout->addWidget( m_tabSpinBox );
	label->setBuddy( m_tabSpinBox );

	layout->addStretch( 1 );
	page->setMinimumSize( sizeHintForWidget( page ) );

	m_tabWidget->addTab( page, i18n( "Appearance" ) );

	page   = new QWidget( m_tabWidget );
	layout = new QVBoxLayout( page );
	layout->setSpacing( KDialog::spacingHint() );
	layout->setMargin( KDialog::marginHint() );

	QGroupBox* gb = new QGroupBox( page );
        QHBoxLayout *layfont = new QHBoxLayout;
        gb->setLayout( layfont );
	gb->setTitle( i18n( "Text Font" ) );
	layout->addWidget( gb );
	//gb->setMargin( KDialog::marginHint() );

	label = new QLabel( i18n( "Font:" ) );
	m_fontCombo = new QFontComboBox;
        layfont->addWidget( label );
        layfont->addWidget( m_fontCombo );
	m_fontCombo->setObjectName( "fontcombo" );
	label->setBuddy( m_fontCombo );

	label = new QLabel( i18n( "Size:" ) );
        layfont->addWidget( label );
	m_fontSizeSpinBox = new QSpinBox( gb );
	m_fontSizeSpinBox->setRange( 6, 24 );
        layfont->addWidget( m_fontSizeSpinBox );
	label->setBuddy( m_fontSizeSpinBox );

	layout->addStretch( 1 );
	page->setMinimumSize( sizeHintForWidget( page ) );

	m_tabWidget->addTab( page, i18n( "Fonts" ) );
}

ViewPage::~ViewPage()
{

}

void ViewPage::setSettings( ViewSettings* setts )
{
	m_settings = setts;

	m_addedColorButton->setColor  ( m_settings->m_addColor );
	m_changedColorButton->setColor( m_settings->m_changeColor );
	m_removedColorButton->setColor( m_settings->m_removeColor );
	m_appliedColorButton->setColor( m_settings->m_appliedColor );
	m_snolSpinBox->setValue       ( m_settings->m_scrollNoOfLines );
	m_tabSpinBox->setValue        ( m_settings->m_tabToNumberOfSpaces );

	m_fontCombo->setCurrentFont   ( m_settings->m_font.family() );
	m_fontSizeSpinBox->setValue   ( m_settings->m_font.pointSize() );
}

ViewSettings* ViewPage::settings( void )
{
	return m_settings;
}

void ViewPage::restore()
{
}

void ViewPage::apply()
{
	m_settings->m_addColor            = m_addedColorButton->color();
	m_settings->m_changeColor         = m_changedColorButton->color();
	m_settings->m_removeColor         = m_removedColorButton->color();
	m_settings->m_appliedColor        = m_appliedColorButton->color();
	m_settings->m_scrollNoOfLines     = m_snolSpinBox->value();
	m_settings->m_tabToNumberOfSpaces = m_tabSpinBox->value();

	m_settings->m_font                = QFont( m_fontCombo->currentFont() );
	m_settings->m_font.setPointSize( m_fontSizeSpinBox->value() );

	m_settings->saveSettings( KGlobal::config().data() );
}

void ViewPage::setDefaults()
{
	m_addedColorButton->setColor  ( ViewSettings::default_addColor );
	m_changedColorButton->setColor( ViewSettings::default_changeColor );
	m_removedColorButton->setColor( ViewSettings::default_removeColor );
	m_appliedColorButton->setColor( ViewSettings::default_appliedColor );
	m_snolSpinBox->setValue       ( 3 );
        m_tabSpinBox->setValue        ( 4 );

	// TODO: port
	// m_fontCombo->setCurrentFont   ( KGlobalSettings::fixedFont().family() );
        m_fontSizeSpinBox->setValue   ( 10 );
}

#include "viewpage.moc"
