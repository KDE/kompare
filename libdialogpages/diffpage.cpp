/***************************************************************************
                                diffprefs.cpp
                                -------------
        begin                   : Sun Mar 4 2001
        Copyright 2001-2004 Otto Bruggeman <otto.bruggeman@home.nl>
        Copyright 2007      Kevin Kofler   <kevin.kofler@chello.at>
****************************************************************************/

/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***************************************************************************/

#include "diffpage.h"

#include <QtGui/QCheckBox>
#include <Q3HGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QRadioButton>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QTabWidget>
#include <QtGui/QToolTip>

#include <kbuttongroup.h>
#include <kapplication.h>
#include <kcombobox.h>
#include <kdialog.h>
#include <keditlistbox.h>
#include <klineedit.h>
#include <klocale.h>
#include <kurlcombobox.h>
#include <kurlrequester.h>
#include <kservicetypetrader.h>

#include <kparts/componentfactory.h>
#include <kregexpeditorinterface.h>
#include <kglobal.h>

#include "diffsettings.h"

DiffPage::DiffPage() : PageBase(), m_ignoreRegExpDialog( 0 )
{
	m_tabWidget = new QTabWidget( this );

	addDiffTab();

	addFormatTab();

	addOptionsTab();

	addExcludeTab();
}

DiffPage::~DiffPage()
{
	m_settings = 0;
}

void DiffPage::setSettings( DiffSettings* setts )
{
	m_settings = setts;

	m_diffURLRequester->setUrl( m_settings->m_diffProgram );

	m_smallerCheckBox->setChecked           ( m_settings->m_createSmallerDiff );
	m_largerCheckBox->setChecked            ( m_settings->m_largeFiles );
	m_tabsCheckBox->setChecked              ( m_settings->m_convertTabsToSpaces );
	m_caseCheckBox->setChecked              ( m_settings->m_ignoreChangesInCase );
	m_linesCheckBox->setChecked             ( m_settings->m_ignoreEmptyLines );
	m_whitespaceCheckBox->setChecked        ( m_settings->m_ignoreWhiteSpace );
	m_allWhitespaceCheckBox->setChecked     ( m_settings->m_ignoreAllWhiteSpace );
	m_ignoreTabExpansionCheckBox->setChecked( m_settings->m_ignoreChangesDueToTabExpansion );

	m_ignoreRegExpCheckBox->setChecked   ( m_settings->m_ignoreRegExp );
	m_ignoreRegExpEdit->setCompletedItems( m_settings->m_ignoreRegExpTextHistory );
	m_ignoreRegExpEdit->setText          ( m_settings->m_ignoreRegExpText );

	m_locSpinBox->setValue( m_settings->m_linesOfContext );

	m_modeButtonGroup->setSelected( m_settings->m_format );

	m_excludeFilePatternCheckBox->setChecked         ( m_settings->m_excludeFilePattern );
	slotExcludeFilePatternToggled                    ( m_settings->m_excludeFilePattern );
	m_excludeFilePatternEditListBox->insertStringList( m_settings->m_excludeFilePatternList );

	m_excludeFileCheckBox->setChecked( m_settings->m_excludeFilesFile );
	slotExcludeFileToggled           ( m_settings->m_excludeFilesFile );
	m_excludeFileURLComboBox->setUrls( m_settings->m_excludeFilesFileHistoryList );
	m_excludeFileURLComboBox->setUrl ( KUrl( m_settings->m_excludeFilesFileURL ) );
}

DiffSettings* DiffPage::settings( void )
{
	return m_settings;
}

void DiffPage::restore()
{
	// this shouldn't do a thing...
}

void DiffPage::apply()
{
	m_settings->m_diffProgram                    = m_diffURLRequester->url().pathOrUrl();

	m_settings->m_largeFiles                     = m_largerCheckBox->isChecked();
	m_settings->m_createSmallerDiff              = m_smallerCheckBox->isChecked();
	m_settings->m_convertTabsToSpaces            = m_tabsCheckBox->isChecked();
	m_settings->m_ignoreChangesInCase            = m_caseCheckBox->isChecked();
	m_settings->m_ignoreEmptyLines               = m_linesCheckBox->isChecked();
	m_settings->m_ignoreWhiteSpace               = m_whitespaceCheckBox->isChecked();
	m_settings->m_ignoreAllWhiteSpace            = m_allWhitespaceCheckBox->isChecked();
	m_settings->m_ignoreChangesDueToTabExpansion = m_ignoreTabExpansionCheckBox->isChecked();

	m_settings->m_ignoreRegExp                   = m_ignoreRegExpCheckBox->isChecked();
	m_settings->m_ignoreRegExpText               = m_ignoreRegExpEdit->text();
	m_settings->m_ignoreRegExpTextHistory        = m_ignoreRegExpEdit->completionObject()->items();

	m_settings->m_linesOfContext                 = m_locSpinBox->value();

	m_settings->m_format = static_cast<Kompare::Format>( m_modeButtonGroup->selected() );

	m_settings->m_excludeFilePattern             = m_excludeFilePatternCheckBox->isChecked();
	m_settings->m_excludeFilePatternList         = m_excludeFilePatternEditListBox->items();

	m_settings->m_excludeFilesFile               = m_excludeFileCheckBox->isChecked();
	m_settings->m_excludeFilesFileURL            = m_excludeFileURLComboBox->currentText();
	m_settings->m_excludeFilesFileHistoryList    = m_excludeFileURLComboBox->urls();

	m_settings->saveSettings( KGlobal::config().data() );
}

void DiffPage::setDefaults()
{
	m_diffURLRequester->setUrl( KUrl( "diff" ) );
	m_smallerCheckBox->setChecked( true );
	m_largerCheckBox->setChecked( true );
	m_tabsCheckBox->setChecked( false );
	m_caseCheckBox->setChecked( false );
	m_linesCheckBox->setChecked( false );
	m_whitespaceCheckBox->setChecked( false );
	m_allWhitespaceCheckBox->setChecked( false );
	m_ignoreTabExpansionCheckBox->setChecked( false );
	m_ignoreRegExpCheckBox->setChecked( false );

	m_ignoreRegExpEdit->setText( QString() );

	m_locSpinBox->setValue( 3 );

	m_modeButtonGroup->setSelected( Kompare::Unified );

	m_excludeFilePatternCheckBox->setChecked( false );

	m_excludeFileCheckBox->setChecked( false );
}

void DiffPage::slotShowRegExpEditor()
{
	if ( ! m_ignoreRegExpDialog )
		m_ignoreRegExpDialog = KServiceTypeTrader::createInstanceFromQuery<QDialog>( "KRegExpEditor/KRegExpEditor", QString(), this );

	KRegExpEditorInterface *iface = qobject_cast<KRegExpEditorInterface *>( m_ignoreRegExpDialog );

	if ( !iface )
		return;

	iface->setRegExp( m_ignoreRegExpEdit->text() );
	bool ok = m_ignoreRegExpDialog->exec();

	if ( ok )
		m_ignoreRegExpEdit->setText( iface->regExp() );
}

void DiffPage::slotExcludeFilePatternToggled( bool on )
{
	if ( !on )
	{
		m_excludeFilePatternEditListBox->setEnabled( false );
	}
	else
	{
		m_excludeFilePatternEditListBox->setEnabled( true );
	}
}

void DiffPage::slotExcludeFileToggled( bool on )
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

void DiffPage::addDiffTab()
{
	QWidget* page   = new QWidget( this );
	QVBoxLayout* layout = new QVBoxLayout( page );
	layout->setSpacing( KDialog::spacingHint() );
	layout->setMargin( KDialog::marginHint() );

	// add diff program selector
	m_diffProgramGroup = new KButtonGroup( page );
	layout->addWidget( m_diffProgramGroup );
	QVBoxLayout* bgLayout = new QVBoxLayout( m_diffProgramGroup );
	m_diffProgramGroup->setTitle( i18n( "Diff Program" ) );
	//m_diffProgramGroup->setMargin( KDialog::marginHint() );

	m_diffURLRequester = new KUrlRequester( m_diffProgramGroup);
	m_diffURLRequester->setObjectName("diffURLRequester" );
	m_diffURLRequester->setWhatsThis( i18n( "You can select a different diff program here. On Solaris the standard diff program does not support all the options that the GNU version does. This way you can select that version." ) );
	bgLayout->addWidget( m_diffURLRequester );

	layout->addStretch( 1 );
	page->setMinimumSize( sizeHintForWidget( page ) );

	m_tabWidget->addTab( page, i18n( "&Diff" ) );
}

void DiffPage::addFormatTab()
{
	QWidget* page   = new QWidget( this );
	QVBoxLayout* layout = new QVBoxLayout( page );
	layout->setSpacing( KDialog::spacingHint() );
	layout->setMargin( KDialog::marginHint() );

	// add diff modes
	m_modeButtonGroup = new KButtonGroup( page );
	m_modeButtonGroup->setWhatsThis( i18n( "Select the format of the output generated by diff. Unified is the one that is used most frequently because it is very readable. The KDE developers like this format the best so use it for sending patches." ) );
	layout->addWidget( m_modeButtonGroup );
	QVBoxLayout* bgLayout = new QVBoxLayout( m_modeButtonGroup );
	m_modeButtonGroup->setTitle( i18n( "Output Format" ) );
	//m_modeButtonGroup->setMargin( KDialog::marginHint() );

	QRadioButton* radioButton;
	radioButton = new QRadioButton( i18n( "Context" ), m_modeButtonGroup );
	bgLayout->addWidget( radioButton );
	radioButton = new QRadioButton( i18n( "Ed" ),      m_modeButtonGroup );
	radioButton->setEnabled( false );
	bgLayout->addWidget( radioButton );
	radioButton = new QRadioButton( i18n( "Normal" ),  m_modeButtonGroup );
	bgLayout->addWidget( radioButton );
	radioButton = new QRadioButton( i18n( "RCS" ),     m_modeButtonGroup );
	radioButton->setEnabled( false );
	bgLayout->addWidget( radioButton );
	radioButton = new QRadioButton( i18n( "Unified" ), m_modeButtonGroup );
	bgLayout->addWidget( radioButton );

	// #lines of context (loc)
	Q3HGroupBox* groupBox = new Q3HGroupBox( page );
	layout->addWidget( groupBox );
	groupBox->setTitle( i18n( "Lines of Context" ) );
	//groupBox->setMargin( KDialog::marginHint() );

	QLabel* label = new QLabel( i18n( "Number of context lines:" ), groupBox );
	m_locSpinBox = new QSpinBox( 0, 100, 1, groupBox );
	m_locSpinBox->setWhatsThis( i18n( "The number of context lines is normally 2 or 3. This makes the diff readable and applicable in most cases. More than 3 lines will only bloat the diff unnecessarily." ) );
	label->setBuddy( m_locSpinBox );

	layout->addStretch( 1 );
	page->setMinimumSize( sizeHintForWidget( page ) );

	m_tabWidget->addTab( page, i18n( "&Format" ) );
}

void DiffPage::addOptionsTab()
{
	QWidget* page   = new QWidget( this );
	QVBoxLayout* layout = new QVBoxLayout( page );
	layout->setSpacing( KDialog::spacingHint() );
	layout->setMargin( KDialog::marginHint() );

	// add diff options
	KButtonGroup* optionButtonGroup = new KButtonGroup( page );
	layout->addWidget( optionButtonGroup );
	QVBoxLayout* bgLayout = new QVBoxLayout( optionButtonGroup );
	optionButtonGroup->setTitle( i18n( "General" ) );
	//optionButtonGroup->setMargin( KDialog::marginHint() );

	m_smallerCheckBox     = new QCheckBox( i18n( "&Look for smaller changes" ), optionButtonGroup );
	QToolTip::add( m_smallerCheckBox, i18n( "This corresponds to the -d diff option." ) );
	bgLayout->addWidget( m_smallerCheckBox );
	m_largerCheckBox      = new QCheckBox( i18n( "O&ptimize for large files" ), optionButtonGroup );
	QToolTip::add( m_largerCheckBox, i18n( "This corresponds to the -H diff option." ) );
	bgLayout->addWidget( m_largerCheckBox );
	m_caseCheckBox        = new QCheckBox( i18n( "&Ignore changes in case" ), optionButtonGroup );
	QToolTip::add( m_caseCheckBox, i18n( "This corresponds to the -i diff option." ) );
	bgLayout->addWidget( m_caseCheckBox );

	QHBoxLayout* groupLayout = new QHBoxLayout( layout );
	groupLayout->setObjectName( "regexp_horizontal_layout" );
	groupLayout->setSpacing( -1 );
	groupLayout->setMargin( KDialog::marginHint() );

	m_ignoreRegExpCheckBox = new QCheckBox( i18n( "Ignore regexp:" ), page );
	QToolTip::add( m_ignoreRegExpCheckBox, i18n( "This option corresponds to the -I diff option." ) );
	groupLayout->addWidget( m_ignoreRegExpCheckBox );
	m_ignoreRegExpEdit = new KLineEdit( QString::null, page);	//krazy:exclude=nullstrassign for old broken gcc
	m_ignoreRegExpEdit->setObjectName("regexplineedit" );
	QToolTip::add( m_ignoreRegExpEdit, i18n( "Add the regular expression here that you want to use\nto ignore lines that match it." ) );
	groupLayout->addWidget( m_ignoreRegExpEdit );

	if ( !KServiceTypeTrader::self()->query("KRegExpEditor/KRegExpEditor").isEmpty() )
	{
		// Ok editor is available, use it
		QPushButton* ignoreRegExpEditButton = new QPushButton( i18n( "&Edit..." ), page, "regexp_editor_button" );
		QToolTip::add( ignoreRegExpEditButton, i18n( "Clicking this will open a regular expression dialog where\nyou can graphically create regular expressions." ) );
		groupLayout->addWidget( ignoreRegExpEditButton );
		connect( ignoreRegExpEditButton, SIGNAL( clicked() ), this, SLOT( slotShowRegExpEditor() ) );
	}

	KButtonGroup* moreOptionButtonGroup = new KButtonGroup( page );
	layout->addWidget( moreOptionButtonGroup );
	bgLayout = new QVBoxLayout( moreOptionButtonGroup );
	moreOptionButtonGroup->setTitle( i18n( "Whitespace" ) );
	//moreOptionButtonGroup->setMargin( KDialog::marginHint() );

	m_tabsCheckBox        = new QCheckBox( i18n( "E&xpand tabs to spaces in output" ), moreOptionButtonGroup );
	QToolTip::add( m_tabsCheckBox, i18n( "This option corresponds to the -t diff option." ) );
	bgLayout->addWidget( m_tabsCheckBox );
	m_linesCheckBox       = new QCheckBox( i18n( "I&gnore added or removed empty lines" ), moreOptionButtonGroup );
	QToolTip::add( m_linesCheckBox, i18n( "This option corresponds to the -B diff option." ) );
	bgLayout->addWidget( m_linesCheckBox );
	m_whitespaceCheckBox  = new QCheckBox( i18n( "Ig&nore changes in the amount of whitespace" ), moreOptionButtonGroup );
	QToolTip::add( m_whitespaceCheckBox, i18n( "This option corresponds to the -b diff option." ) );
	bgLayout->addWidget( m_whitespaceCheckBox );
	m_allWhitespaceCheckBox = new QCheckBox( i18n( "Ign&ore all whitespace" ), moreOptionButtonGroup );
	QToolTip::add( m_allWhitespaceCheckBox, i18n( "This option corresponds to the -w diff option." ) );
	bgLayout->addWidget( m_allWhitespaceCheckBox );
	m_ignoreTabExpansionCheckBox = new QCheckBox( i18n( "Igno&re changes due to tab expansion" ), moreOptionButtonGroup );
	QToolTip::add( m_ignoreTabExpansionCheckBox, i18n( "This option corresponds to the -E diff option." ) );
	bgLayout->addWidget( m_ignoreTabExpansionCheckBox );

	layout->addStretch( 1 );
	page->setMinimumSize( sizeHintForWidget( page ) );

	m_tabWidget->addTab( page, i18n( "O&ptions" ) );
}

void DiffPage::addExcludeTab()
{
	QWidget* page = new QWidget( this );
	QVBoxLayout* layout = new QVBoxLayout( page );
	layout->setSpacing( KDialog::spacingHint() );
	layout->setMargin( KDialog::marginHint() );

	Q3HGroupBox* excludeFilePatternGroupBox = new Q3HGroupBox( page );
	excludeFilePatternGroupBox->setTitle( i18n( "File Pattern to Exclude" ) );
	m_excludeFilePatternCheckBox = new QCheckBox( "", excludeFilePatternGroupBox );
	QToolTip::add( m_excludeFilePatternCheckBox, i18n( "If this is checked you can enter a shell pattern in the text box on the right or select entries from the list." ) );
	m_excludeFilePatternEditListBox = new KEditListBox( excludeFilePatternGroupBox, "exclude_file_pattern_editlistbox", false, KEditListBox::Add|KEditListBox::Remove );
	QToolTip::add( m_excludeFilePatternEditListBox, i18n( "Here you can enter or remove a shell pattern or select one or more entries from the list." ) );
	layout->addWidget( excludeFilePatternGroupBox );


	connect( m_excludeFilePatternCheckBox, SIGNAL(toggled(bool)), this, SLOT(slotExcludeFilePatternToggled(bool)));

	Q3HGroupBox* excludeFileNameGroupBox = new Q3HGroupBox( page );
	excludeFileNameGroupBox->setTitle( i18n( "File with Filenames to Exclude" ) );
	m_excludeFileCheckBox     = new QCheckBox( "", excludeFileNameGroupBox );
	QToolTip::add( m_excludeFileCheckBox, i18n( "If this is checked you can enter a filename in the combo box on the right." ) );
	m_excludeFileURLComboBox  = new KUrlComboBox( KUrlComboBox::Files, true, excludeFileNameGroupBox );
	m_excludeFileURLComboBox->setObjectName( "exclude_file_urlcombo" );
	QToolTip::add( m_excludeFileURLComboBox, i18n( "Here you can enter the URL of a file with shell patterns to ignore during the comparison of the folders." ) );
	m_excludeFileURLRequester = new KUrlRequester( m_excludeFileURLComboBox, excludeFileNameGroupBox);
	m_excludeFileURLRequester->setObjectName("exclude_file_name_urlrequester" );
	QToolTip::add( m_excludeFileURLRequester, i18n( "Any file you select in the dialog that pops up when you click it will be put in the dialog to the left of this button." ) );
	layout->addWidget( excludeFileNameGroupBox );

	connect( m_excludeFileCheckBox, SIGNAL(toggled(bool)), this, SLOT(slotExcludeFileToggled(bool)));

	layout->addStretch( 1 );
	page->setMinimumSize( sizeHintForWidget( page ) );

	m_tabWidget->addTab( page, i18n( "&Exclude" ) );
}

#include "diffpage.moc"
