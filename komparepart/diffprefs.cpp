/***************************************************************************
				diffprefs.cpp  -  description
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
***************************************************************************/

#include <qhgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>

#include <kdialog.h>
#include <klocale.h>

#include "diffprefs.h"

DiffPrefs::DiffPrefs( QWidget* parent ) : PrefsBase( parent )
{
	QWidget*       page;
	QVBoxLayout*   layout;
	QVButtonGroup* optionButtonGroup;
	QVButtonGroup* moreOptionButtonGroup;
	QHGroupBox*    locGroupBox;
	QLabel*        label;
	QRadioButton*  radioButton;

	page   = new QWidget( this );
	layout = new QVBoxLayout( page );
	layout->setSpacing( KDialog::spacingHint() );
	layout->setMargin( KDialog::marginHint() );

	// add diff modes
	m_modeButtonGroup = new QVButtonGroup( i18n( "Output format" ), page );
	layout->addWidget( m_modeButtonGroup );
	m_modeButtonGroup->setMargin( KDialog::marginHint() );	

	radioButton = new QRadioButton( i18n( "Context" ), m_modeButtonGroup );
	radioButton = new QRadioButton( i18n( "Ed" ),      m_modeButtonGroup );
	radioButton = new QRadioButton( i18n( "Normal" ),  m_modeButtonGroup );
	radioButton = new QRadioButton( i18n( "RCS" ),     m_modeButtonGroup );
	radioButton = new QRadioButton( i18n( "Unified" ), m_modeButtonGroup );

	// #lines of context (loc)
	locGroupBox     = new QHGroupBox( i18n( "Lines of context" ), page );
	layout->addWidget( locGroupBox );
	locGroupBox->setMargin( KDialog::marginHint() );

	label           = new QLabel( i18n( "Number of contextlines:" ), locGroupBox );
	m_locSpinButton = new QSpinBox( 0, 100, 1, locGroupBox );
	label->setBuddy( m_locSpinButton );

	layout->addStretch( 1 );
	page->setMinimumSize( sizeHintForWidget( page ) );

	addTab( page, i18n( "&Format" ) );

	page   = new QWidget( this );
	layout = new QVBoxLayout( page );
	layout->setSpacing( KDialog::spacingHint() );
	layout->setMargin( KDialog::marginHint() );

	// add diff options
	optionButtonGroup     = new QVButtonGroup( i18n( "General" ), page );
	layout->addWidget( optionButtonGroup );
	optionButtonGroup->setMargin( KDialog::marginHint() );

	m_smallerCheckBox     = new QCheckBox( i18n( "&Look for smaller changes" ), optionButtonGroup );
	m_largerCheckBox      = new QCheckBox( i18n( "O&ptimize for large files" ), optionButtonGroup );
	m_caseCheckBox        = new QCheckBox( i18n( "&Ignore changes in case" ), optionButtonGroup );

	moreOptionButtonGroup = new QVButtonGroup( i18n( "Whitespace" ), page );
	layout->addWidget( moreOptionButtonGroup );
	moreOptionButtonGroup->setMargin( KDialog::marginHint() );

	m_tabsCheckBox        = new QCheckBox( i18n( "&Expand tabs to spaces in output" ), moreOptionButtonGroup );
	m_linesCheckBox       = new QCheckBox( i18n( "I&gnore added or removed empty lines" ), moreOptionButtonGroup );
	m_whitespaceCheckBox  = new QCheckBox( i18n( "Ig&nore changes in number of whitespace on a line" ), moreOptionButtonGroup );

	layout->addStretch( 1 );
	page->setMinimumSize( sizeHintForWidget( page ) );

	addTab( page, i18n( "&Options" ) );
};

DiffPrefs::~DiffPrefs()
{
	m_settings = NULL;
};

void DiffPrefs::setSettings( DiffSettings* setts )
{
	m_settings = setts;

	m_smallerCheckBox->setChecked   ( m_settings->m_createSmallerDiff );
	m_largerCheckBox->setChecked    ( m_settings->m_largeFiles );
	m_tabsCheckBox->setChecked      ( m_settings->m_convertTabsToSpaces );
	m_caseCheckBox->setChecked      ( m_settings->m_ignoreChangesInCase );
	m_linesCheckBox->setChecked     ( m_settings->m_ignoreWhitespaceComparingLines );
	m_whitespaceCheckBox->setChecked( m_settings->m_ignoreWhiteSpace );

	m_locSpinButton->setValue       ( m_settings->m_linesOfContext );

	m_modeButtonGroup->setButton    ( m_settings->m_format );
};

DiffSettings* DiffPrefs::getSettings( void )
{
	return m_settings;
}

void DiffPrefs::restore()
{
	// this shouldn't do a thing...
};

void DiffPrefs::apply()
{
	DiffSettings* setts;
	setts = (DiffSettings*)getSettings();
	
	setts->m_largeFiles                     = m_largerCheckBox->isChecked();
	setts->m_createSmallerDiff              = m_smallerCheckBox->isChecked();
	setts->m_convertTabsToSpaces            = m_tabsCheckBox->isChecked();
	setts->m_ignoreChangesInCase            = m_caseCheckBox->isChecked();
	setts->m_ignoreWhitespaceComparingLines = m_linesCheckBox->isChecked();
	setts->m_ignoreWhiteSpace               = m_whitespaceCheckBox->isChecked();

	setts->m_linesOfContext                 = m_locSpinButton->value();

	setts->m_format = static_cast<Kompare::Format>( m_modeButtonGroup->id( m_modeButtonGroup->selected() ) );
};

void DiffPrefs::setDefaults()
{
	m_smallerCheckBox->setChecked( true );
	m_largerCheckBox->setChecked( true );
	m_tabsCheckBox->setChecked( false );
	m_caseCheckBox->setChecked( false );
	m_linesCheckBox->setChecked( false );
	m_whitespaceCheckBox->setChecked( false );
	
	m_locSpinButton->setValue( 3 );
	
	m_modeButtonGroup->setButton( Unified );
};

#include "diffprefs.moc"
