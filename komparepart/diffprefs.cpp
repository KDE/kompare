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

#include <qcheckbox.h>
#include <qhgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qvbuttongroup.h>

#include <kcombobox.h>
#include <kdialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <ktrader.h>
#include <kurlcombobox.h>
#include <kurlrequester.h>

#include <kparts/componentfactory.h>
#include <kregexpeditorinterface.h>

#include "diffsettings.h"

#include "diffprefs.h"

DiffPrefs::DiffPrefs( QWidget* parent ) : PrefsBase( parent ),
	m_ignoreRegExpDialog( 0 )
{
	addDiffTab();

	addFormatTab();

	addOptionsTab();

	addExcludeTab();
}

DiffPrefs::~DiffPrefs()
{
	m_settings = NULL;
}

void DiffPrefs::setSettings( DiffSettings* setts )
{
	m_settings = setts;

	m_diffURLRequester->setURL( m_settings->m_diffProgram );

	m_smallerCheckBox->setChecked           ( m_settings->m_createSmallerDiff );
	m_largerCheckBox->setChecked            ( m_settings->m_largeFiles );
	m_tabsCheckBox->setChecked              ( m_settings->m_convertTabsToSpaces );
	m_caseCheckBox->setChecked              ( m_settings->m_ignoreChangesInCase );
	m_linesCheckBox->setChecked             ( m_settings->m_ignoreEmptyLines );
	m_whitespaceCheckBox->setChecked        ( m_settings->m_ignoreWhiteSpace );

	m_ignoreRegExpCheckBox->setChecked   ( m_settings->m_ignoreRegExp );
	m_ignoreRegExpEdit->setCompletedItems( m_settings->m_ignoreRegExpTextHistory );
	m_ignoreRegExpEdit->setText          ( m_settings->m_ignoreRegExpText );

	m_locSpinBox->setValue( m_settings->m_linesOfContext );

	m_modeButtonGroup->setButton( m_settings->m_format );

	m_excludeFilePatternCheckBox->setChecked       ( m_settings->m_excludeFilePattern );
	slotExcludeFilePatternToggled                  ( m_settings->m_excludeFilePattern );
	m_excludeFilePatternComboBox->setCompletedItems( m_settings->m_excludeFilePatternHistoryList );
	m_excludeFilePatternComboBox->setCurrentText   ( m_settings->m_excludeFilePatternText );

	m_excludeFileCheckBox->setChecked( m_settings->m_excludeFilesFile );
	slotExcludeFileToggled           ( m_settings->m_excludeFilesFile );
	m_excludeFileURLComboBox->setURLs( m_settings->m_excludeFilesFileHistoryList );
	m_excludeFileURLComboBox->setURL ( KURL( m_settings->m_excludeFilesFileURL ) );
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

	setts->m_diffProgram                    = m_diffURLRequester->url();

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

	setts->m_format = static_cast<Kompare::Format>( m_modeButtonGroup->selectedId() );

	setts->m_excludeFilePattern            = m_excludeFilePatternCheckBox->isChecked();
	setts->m_excludeFilePatternText        = m_excludeFilePatternComboBox->currentText();
	setts->m_excludeFilePatternHistoryList.clear();
	QString entry;
	for ( int i = 0; i < m_excludeFilePatternComboBox->count(); i++ )
	{
		entry = m_excludeFilePatternComboBox->text( i );
		if ( !entry.isEmpty() )
		{
			setts->m_excludeFilePatternHistoryList.append( entry );
		}
	}

	setts->m_excludeFilesFile            = m_excludeFileCheckBox->isChecked();
	setts->m_excludeFilesFileURL         = m_excludeFileURLComboBox->currentText();
	setts->m_excludeFilesFileHistoryList = m_excludeFileURLComboBox->urls();
}

void DiffPrefs::setDefaults()
{
	m_diffURLRequester->setURL( "diff" );
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

	m_excludeFilePatternCheckBox->setChecked( false );

	m_excludeFileCheckBox->setChecked( false );
}

void DiffPrefs::slotShowRegExpEditor()
{
	if ( ! m_ignoreRegExpDialog )
		m_ignoreRegExpDialog = KParts::ComponentFactory::createInstanceFromQuery<QDialog>( "KRegExpEditor/KRegExpEditor", QString::null, this );

	KRegExpEditorInterface *iface = static_cast<KRegExpEditorInterface *>( m_ignoreRegExpDialog->qt_cast( "KRegExpEditorInterface" ) );

	if ( !iface )
		return;

	iface->setRegExp( m_ignoreRegExpEdit->text() );
	bool ok = m_ignoreRegExpDialog->exec();

	if ( ok )
		m_ignoreRegExpEdit->setText( iface->regExp() );
}

void DiffPrefs::slotExcludeFilePatternToggled( bool on )
{
	if ( !on )
	{
		m_excludeFilePatternComboBox->setEnabled( false );
	}
	else
	{
		m_excludeFilePatternComboBox->setEnabled( true );
	}
}

void DiffPrefs::slotExcludeFileToggled( bool on )
{
	if ( !on )
	{
		m_excludeFileURLComboBox->setEnabled( false );
		m_excludeFileURLRequester->setEnabled( false );
	}
	else
	{
		m_excludeFileURLComboBox->setEnabled( true );
		m_excludeFileURLRequester->setEnabled( true );
	}
}

void DiffPrefs::addDiffTab()
{
	QWidget* page   = new QWidget( this );
	QVBoxLayout* layout = new QVBoxLayout( page );
	layout->setSpacing( KDialog::spacingHint() );
	layout->setMargin( KDialog::marginHint() );

	// add diff program selector
	m_diffProgramGroup = new QVButtonGroup( i18n( "Diff Program" ), page );
	layout->addWidget( m_diffProgramGroup );
	m_diffProgramGroup->setMargin( KDialog::marginHint() );

	m_diffURLRequester = new KURLRequester( m_diffProgramGroup, "diffURLRequester" );

	layout->addStretch( 1 );
	page->setMinimumSize( sizeHintForWidget( page ) );

	addTab( page, i18n( "&Diff" ) );
}

void DiffPrefs::addFormatTab()
{
	QWidget* page   = new QWidget( this );
	QVBoxLayout* layout = new QVBoxLayout( page );
	layout->setSpacing( KDialog::spacingHint() );
	layout->setMargin( KDialog::marginHint() );

	// add diff modes
	m_modeButtonGroup = new QVButtonGroup( i18n( "Output Format" ), page );
	layout->addWidget( m_modeButtonGroup );
	m_modeButtonGroup->setMargin( KDialog::marginHint() );

	QRadioButton* radioButton;
	radioButton = new QRadioButton( i18n( "Context" ), m_modeButtonGroup );
	radioButton = new QRadioButton( i18n( "Ed" ),      m_modeButtonGroup );
	radioButton->setEnabled( false );
	radioButton = new QRadioButton( i18n( "Normal" ),  m_modeButtonGroup );
	radioButton = new QRadioButton( i18n( "RCS" ),     m_modeButtonGroup );
	radioButton->setEnabled( false );
	radioButton = new QRadioButton( i18n( "Unified" ), m_modeButtonGroup );

	// #lines of context (loc)
	QHGroupBox* groupBox = new QHGroupBox( i18n( "Lines of Context" ), page );
	layout->addWidget( groupBox );
	groupBox->setMargin( KDialog::marginHint() );

	QLabel* label = new QLabel( i18n( "Number of context lines:" ), groupBox );
	m_locSpinBox = new QSpinBox( 0, 100, 1, groupBox );
	label->setBuddy( m_locSpinBox );

	layout->addStretch( 1 );
	page->setMinimumSize( sizeHintForWidget( page ) );

	addTab( page, i18n( "&Format" ) );
}

void DiffPrefs::addOptionsTab()
{
	QWidget* page   = new QWidget( this );
	QVBoxLayout* layout = new QVBoxLayout( page );
	layout->setSpacing( KDialog::spacingHint() );
	layout->setMargin( KDialog::marginHint() );

	// add diff options
	QVButtonGroup* optionButtonGroup     = new QVButtonGroup( i18n( "General" ), page );
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

	QVButtonGroup* moreOptionButtonGroup = new QVButtonGroup( i18n( "Whitespace" ), page );
	layout->addWidget( moreOptionButtonGroup );
	moreOptionButtonGroup->setMargin( KDialog::marginHint() );

	m_tabsCheckBox        = new QCheckBox( i18n( "&Expand tabs to spaces in output" ), moreOptionButtonGroup );
	m_linesCheckBox       = new QCheckBox( i18n( "I&gnore added or removed empty lines" ), moreOptionButtonGroup );
	m_whitespaceCheckBox  = new QCheckBox( i18n( "Ig&nore changes in the amount of whitespace" ), moreOptionButtonGroup );

	layout->addStretch( 1 );
	page->setMinimumSize( sizeHintForWidget( page ) );

	addTab( page, i18n( "O&ptions" ) );
}

void DiffPrefs::addExcludeTab()
{
	QWidget* page = new QWidget( this );
	QVBoxLayout* layout = new QVBoxLayout( page );
	layout->setSpacing( KDialog::spacingHint() );
	layout->setMargin( KDialog::marginHint() );

	QHGroupBox* excludeFilePatternGroupBox = new QHGroupBox( i18n( "File pattern to exclude" ), page );
	m_excludeFilePatternCheckBox = new QCheckBox( "", excludeFilePatternGroupBox );
	m_excludeFilePatternComboBox = new KComboBox( true, excludeFilePatternGroupBox, "exclude_file_pattern_combobox" );
	layout->addWidget( excludeFilePatternGroupBox );

	connect( m_excludeFilePatternCheckBox, SIGNAL(toggled(bool)), this, SLOT(slotExcludeFilePatternToggled(bool)));

	QHGroupBox* excludeFileNameGroupBox = new QHGroupBox( i18n( "File with filenames to exclude" ), page );
	m_excludeFileCheckBox     = new QCheckBox( "", excludeFileNameGroupBox );
	m_excludeFileURLComboBox  = new KURLComboBox( KURLComboBox::Files, true, excludeFileNameGroupBox, "exclude_file_urlcombo" );
	m_excludeFileURLRequester = new KURLRequester( m_excludeFileURLComboBox, excludeFileNameGroupBox, "exclude_file_name_urlrequester" );
	layout->addWidget( excludeFileNameGroupBox );

	connect( m_excludeFileCheckBox, SIGNAL(toggled(bool)), this, SLOT(slotExcludeFileToggled(bool)));

	layout->addStretch( 1 );
	page->setMinimumSize( sizeHintForWidget( page ) );

	// FIXME: restore the i18n() call, dunno how smart the extraction script is and i dont want to cause fuzzies
	addTab( page, "&Exclude" );
}

#include "diffprefs.moc"
