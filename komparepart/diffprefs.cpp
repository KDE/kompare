/***************************************************************************
				diffprefs.cpp  -  description
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
***************************************************************************/

#include <qhgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>

#include <kdialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <ktrader.h>

#include <kparts/componentfactory.h>
#include <kregexpeditorinterface.h>

#include "diffprefs.h"

DiffPrefs::DiffPrefs( QWidget* parent ) : PrefsBase( parent ),
	m_ignoreRegExpDialog( 0 )
{
	QWidget*       page;
	QVBoxLayout*   layout;
	QVButtonGroup* optionButtonGroup;
	QVButtonGroup* moreOptionButtonGroup;
	QHGroupBox*    groupBox;
	QLabel*        label;
	QRadioButton*  radioButton;

	page   = new QWidget( this );
	layout = new QVBoxLayout( page );
	layout->setSpacing( KDialog::spacingHint() );
	layout->setMargin( KDialog::marginHint() );

	// add diff program selector
	m_diffProgramGroup = new QVButtonGroup( i18n( "Diff Program" ), page );
	layout->addWidget( m_diffProgramGroup );
	m_diffProgramGroup->setMargin( KDialog::marginHint() );

	m_urlRequester = new KURLRequester( m_diffProgramGroup, "urlRequester" );

	layout->addStretch( 1 );
	page->setMinimumSize( sizeHintForWidget( page ) );

	addTab( page, i18n( "&Diff" ) );

	page   = new QWidget( this );
	layout = new QVBoxLayout( page );
	layout->setSpacing( KDialog::spacingHint() );
	layout->setMargin( KDialog::marginHint() );

	// add diff modes
	m_modeButtonGroup = new QVButtonGroup( i18n( "Output Format" ), page );
	layout->addWidget( m_modeButtonGroup );
	m_modeButtonGroup->setMargin( KDialog::marginHint() );

	radioButton = new QRadioButton( i18n( "Context" ), m_modeButtonGroup );
	radioButton = new QRadioButton( i18n( "Ed" ),      m_modeButtonGroup );
	radioButton = new QRadioButton( i18n( "Normal" ),  m_modeButtonGroup );
	radioButton = new QRadioButton( i18n( "RCS" ),     m_modeButtonGroup );
	radioButton = new QRadioButton( i18n( "Unified" ), m_modeButtonGroup );

	// #lines of context (loc)
	groupBox = new QHGroupBox( i18n( "Lines of Context" ), page );
	layout->addWidget( groupBox );
	groupBox->setMargin( KDialog::marginHint() );

	label           = new QLabel( i18n( "Number of context lines:" ), groupBox );
	m_locSpinBox    = new QSpinBox( 0, 100, 1, groupBox );
	label->setBuddy( m_locSpinBox );

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

	QHBoxLayout* groupLayout = new QHBoxLayout( layout, -1, "regexp_horizontal_layout" );
	groupLayout->setMargin( KDialog::marginHint() );

	m_ignoreRegExpCheckBox = new QCheckBox( i18n( "Ignore regexp:" ), page );
	groupLayout->addWidget( m_ignoreRegExpCheckBox );
	m_ignoreRegExpEdit = new KLineEdit( QString::null, page, "regexplineedit" );
	groupLayout->addWidget( m_ignoreRegExpEdit );

	if ( !KTrader::self()->query("KRegExpEditor/KRegExpEditor").isEmpty() )
	{
		// Ok editor is available, use it
		QButton* ignoreRegExpEditButton = new QPushButton( i18n( "&Edit..." ), page, "regexp_editor_button" );
		groupLayout->addWidget( ignoreRegExpEditButton );
		connect( ignoreRegExpEditButton, SIGNAL( clicked() ), this, SLOT( slotShowRegExpEditor() ) );
	}

	moreOptionButtonGroup = new QVButtonGroup( i18n( "Whitespace" ), page );
	layout->addWidget( moreOptionButtonGroup );
	moreOptionButtonGroup->setMargin( KDialog::marginHint() );

	m_tabsCheckBox        = new QCheckBox( i18n( "&Expand tabs to spaces in output" ), moreOptionButtonGroup );
	m_linesCheckBox       = new QCheckBox( i18n( "I&gnore added or removed empty lines" ), moreOptionButtonGroup );
	m_whitespaceCheckBox  = new QCheckBox( i18n( "Ig&nore changes in the amount of whitespace" ), moreOptionButtonGroup );

	layout->addStretch( 1 );
	page->setMinimumSize( sizeHintForWidget( page ) );

	addTab( page, i18n( "O&ptions" ) );
}

DiffPrefs::~DiffPrefs()
{
	m_settings = NULL;
}

void DiffPrefs::setSettings( DiffSettings* setts )
{
	m_settings = setts;

	m_urlRequester->setURL            ( m_settings->m_diffProgram );

	m_smallerCheckBox->setChecked     ( m_settings->m_createSmallerDiff );
	m_largerCheckBox->setChecked      ( m_settings->m_largeFiles );
	m_tabsCheckBox->setChecked        ( m_settings->m_convertTabsToSpaces );
	m_caseCheckBox->setChecked        ( m_settings->m_ignoreChangesInCase );
	m_linesCheckBox->setChecked       ( m_settings->m_ignoreEmptyLines );
	m_whitespaceCheckBox->setChecked  ( m_settings->m_ignoreWhiteSpace );

	m_ignoreRegExpCheckBox->setChecked( m_settings->m_ignoreRegExp );
	m_ignoreRegExpEdit->setText       ( m_settings->m_ignoreRegExpText );
	m_ignoreRegExpEdit->setCompletedItems( m_settings->m_ignoreRegExpTextHistory );

	m_locSpinBox->setValue            ( m_settings->m_linesOfContext );

	m_modeButtonGroup->setButton      ( m_settings->m_format );
}

DiffSettings* DiffPrefs::settings( void )
{
	return m_settings;
}

void DiffPrefs::restore()
{
	// this shouldn't do a thing...
}

void DiffPrefs::apply()
{
	DiffSettings* setts;
	setts = (DiffSettings*)settings();

	setts->m_diffProgram                    = m_urlRequester->url();

	setts->m_largeFiles                     = m_largerCheckBox->isChecked();
	setts->m_createSmallerDiff              = m_smallerCheckBox->isChecked();
	setts->m_convertTabsToSpaces            = m_tabsCheckBox->isChecked();
	setts->m_ignoreChangesInCase            = m_caseCheckBox->isChecked();
	setts->m_ignoreEmptyLines               = m_linesCheckBox->isChecked();
	setts->m_ignoreWhiteSpace               = m_whitespaceCheckBox->isChecked();

	setts->m_ignoreRegExp                   = m_ignoreRegExpCheckBox->isChecked();
	setts->m_ignoreRegExpText               = m_ignoreRegExpEdit->text();
	setts->m_ignoreRegExpTextHistory        = m_ignoreRegExpEdit->completionObject()->items();

	setts->m_linesOfContext                 = m_locSpinBox->value();

	setts->m_format = static_cast<Kompare::Format>( m_modeButtonGroup->id( m_modeButtonGroup->selected() ) );
}

void DiffPrefs::setDefaults()
{
	m_urlRequester->setURL( "diff" );
	m_smallerCheckBox->setChecked( true );
	m_largerCheckBox->setChecked( true );
	m_tabsCheckBox->setChecked( false );
	m_caseCheckBox->setChecked( false );
	m_linesCheckBox->setChecked( false );
	m_whitespaceCheckBox->setChecked( false );
	m_ignoreRegExpCheckBox->setChecked( false );

	m_ignoreRegExpEdit->setText( QString::null );

	m_locSpinBox->setValue( 3 );

	m_modeButtonGroup->setButton( Kompare::Unified );
}

void DiffPrefs::slotShowRegExpEditor()
{
	if ( ! m_ignoreRegExpDialog )
	{
		m_ignoreRegExpDialog = KParts::ComponentFactory::createInstanceFromQuery<QDialog>( "KRegExpEditor/KRegExpEditor", QString::null, this );
	}

	KRegExpEditorInterface *iface = static_cast<KRegExpEditorInterface *>( m_ignoreRegExpDialog->qt_cast( "KRegExpEditorInterface" ) );

	if ( !iface )
	{
		return;
	}

	iface->setRegExp( m_ignoreRegExpEdit->text() );
	bool ok = m_ignoreRegExpDialog->exec();

	if ( ok )
	{
		m_ignoreRegExpEdit->setText( iface->regExp() );
	}
}

#include "diffprefs.moc"
