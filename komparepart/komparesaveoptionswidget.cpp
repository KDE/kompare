/***************************************************************************
                                komparesaveoptionswidget.cpp
                                ----------------------------
        begin                   : Sun Mar 4 2001
        Copyright 2001-2003 Otto Bruggeman <otto.bruggeman@home.nl>
        Copyright 2001-2003 John Firebaugh <jfirebaugh@kde.org>
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

#include "komparesaveoptionswidget.h"

#include <q3buttongroup.h>
#include <QtGui/QCheckBox>
#include <QtGui/QLabel>
#include <QtGui/QRadioButton>
#include <QtGui/QSpinBox>

#include <kdebug.h>
#include <kfiledialog.h>
#include <kurlrequester.h>

#include "diffsettings.h"

KompareSaveOptionsWidget::KompareSaveOptionsWidget( QString source, QString destination,
           DiffSettings * settings, QWidget * parent )
	: KompareSaveOptionsBase( parent, "save options" )
	, m_source( source )
	, m_destination( destination )
{
	m_settings = settings;

	m_directoryRequester->setMode(
	    KFile::ExistingOnly | KFile::Directory | KFile::LocalOnly );

	KUrl sourceURL;
	KUrl destinationURL;
	sourceURL.setPath( source );
	destinationURL.setPath( destination );

	// Find a common root.
	KUrl root( sourceURL );
	while( root.isValid() && !root.isParentOf( destinationURL ) ) {
		root = root.upUrl();
	}

	// If we found a common root, change to that directory and
	// strip the common part from source and destination.
	if( root.isValid() ) {
		m_directoryRequester->setUrl( root.url() );
	}

	connect( m_SmallerChangesCB,   SIGNAL(toggled(bool)), SLOT(updateCommandLine()) );
	connect( m_LargeFilesCB,       SIGNAL(toggled(bool)), SLOT(updateCommandLine()) );
	connect( m_IgnoreCaseCB,       SIGNAL(toggled(bool)), SLOT(updateCommandLine()) );
	connect( m_ExpandTabsCB,       SIGNAL(toggled(bool)), SLOT(updateCommandLine()) );
	connect( m_IgnoreEmptyLinesCB, SIGNAL(toggled(bool)), SLOT(updateCommandLine()) );
	connect( m_IgnoreWhiteSpaceCB, SIGNAL(toggled(bool)), SLOT(updateCommandLine()) );
	connect( m_FunctionNamesCB,    SIGNAL(toggled(bool)), SLOT(updateCommandLine()) );
	connect( m_RecursiveCB,        SIGNAL(toggled(bool)), SLOT(updateCommandLine()) );
	connect( m_NewFilesCB,         SIGNAL(toggled(bool)), SLOT(updateCommandLine()) );
	connect( m_ContextRB,          SIGNAL(toggled(bool)), SLOT(updateCommandLine()) );
	connect( m_EdRB,               SIGNAL(toggled(bool)), SLOT(updateCommandLine()) );
	connect( m_NormalRB,           SIGNAL(toggled(bool)), SLOT(updateCommandLine()) );
	connect( m_RCSRB,              SIGNAL(toggled(bool)), SLOT(updateCommandLine()) );
	connect( m_UnifiedRB,          SIGNAL(toggled(bool)), SLOT(updateCommandLine()) );
	connect( m_SideBySideRB,       SIGNAL(toggled(bool)), SLOT(updateCommandLine()) );
	connect( m_ContextLinesSB,     SIGNAL(valueChanged(int)), SLOT(updateCommandLine()) );
	connect( m_directoryRequester, SIGNAL(textChanged(const QString&)), SLOT(updateCommandLine()) );

	loadOptions();
}

KompareSaveOptionsWidget::~KompareSaveOptionsWidget()
{

}

QString KompareSaveOptionsWidget::directory() const
{
	return KUrl( m_directoryRequester->url() ).toLocalFile();
}

void KompareSaveOptionsWidget::updateCommandLine()
{
	QString cmdLine = "diff";

	QString options = "";

	switch( static_cast<Kompare::Format>( m_FormatBG->id( m_FormatBG->selected() ) ) ) {
	case Kompare::Unified :
		cmdLine += " -U " + QString::number( m_ContextLinesSB->value() );
		break;
	case Kompare::Context :
		cmdLine += " -C " + QString::number( m_ContextLinesSB->value() );
		break;
	case Kompare::RCS :
		options += 'n';
		break;
	case Kompare::Ed :
		options += 'e';
		break;
	case Kompare::SideBySide:
		options += 'y';
		break;
	case Kompare::Normal :
	case Kompare::UnknownFormat :
	default:
		break;
	}

	if ( m_SmallerChangesCB->isChecked() ) {
		options += 'd';
	}

	if ( m_LargeFilesCB->isChecked() ) {
		options += 'H';
	}

	if ( m_IgnoreCaseCB->isChecked() ){
		options += 'i';
	}

	if ( m_ExpandTabsCB->isChecked() ) {
		options += 't';
	}

	if ( m_IgnoreEmptyLinesCB->isChecked() ) {
		options += 'B';
	}

	if ( m_IgnoreWhiteSpaceCB->isChecked() ) {
		options += 'b';
	}

	if ( m_FunctionNamesCB->isChecked() ) {
		options += 'p';
	}

//	if (  ) {
//		cmdLine += " -w";
//	}

	if ( m_RecursiveCB->isChecked() ) {
		options += 'r';
	}

	if( m_NewFilesCB->isChecked() ) {
		options += 'N';
	}

//	if( m_AllTextCB->isChecked() ) {
//		options += 'a';
//	}

	if( options.length() > 0 ) {
		cmdLine += " -" + options;
	}

	cmdLine += " -- ";
	cmdLine += constructRelativePath( m_directoryRequester->url().pathOrUrl(), m_source );
	cmdLine += ' ';
	cmdLine += constructRelativePath( m_directoryRequester->url().pathOrUrl(), m_destination );

	m_CommandLineLabel->setText( cmdLine );
}

void KompareSaveOptionsWidget::loadOptions()
{
	m_SmallerChangesCB->setChecked  ( m_settings->m_createSmallerDiff );
	m_LargeFilesCB->setChecked      ( m_settings->m_largeFiles );
	m_IgnoreCaseCB->setChecked      ( m_settings->m_ignoreChangesInCase );
	m_ExpandTabsCB->setChecked      ( m_settings->m_convertTabsToSpaces );
	m_IgnoreEmptyLinesCB->setChecked( m_settings->m_ignoreEmptyLines );
	m_IgnoreWhiteSpaceCB->setChecked( m_settings->m_ignoreWhiteSpace );
	m_FunctionNamesCB->setChecked   ( m_settings->m_showCFunctionChange );
	m_RecursiveCB->setChecked       ( m_settings->m_recursive );
	m_NewFilesCB->setChecked        ( m_settings->m_newFiles );
//	m_AllTextCB->setChecked         ( m_settings->m_allText );

	m_ContextLinesSB->setValue      ( m_settings->m_linesOfContext );

	m_FormatBG->setButton           ( m_settings->m_format );

	updateCommandLine();
}

void KompareSaveOptionsWidget::saveOptions()
{
	m_settings->m_createSmallerDiff   = m_SmallerChangesCB->isChecked();
	m_settings->m_largeFiles          = m_LargeFilesCB->isChecked();
	m_settings->m_ignoreChangesInCase = m_IgnoreCaseCB->isChecked();
	m_settings->m_convertTabsToSpaces = m_ExpandTabsCB->isChecked();
	m_settings->m_ignoreEmptyLines    = m_IgnoreEmptyLinesCB->isChecked();
	m_settings->m_ignoreWhiteSpace    = m_IgnoreWhiteSpaceCB->isChecked();
	m_settings->m_showCFunctionChange = m_FunctionNamesCB->isChecked();
	m_settings->m_recursive           = m_RecursiveCB->isChecked();
	m_settings->m_newFiles            = m_NewFilesCB->isChecked();
//	m_settings->m_allText             = m_AllTextCB->isChecked();

	m_settings->m_linesOfContext      = m_ContextLinesSB->value();

	m_settings->m_format = static_cast<Kompare::Format>( m_FormatBG->id( m_FormatBG->selected() ) );

}

#include "komparesaveoptionswidget.moc"
