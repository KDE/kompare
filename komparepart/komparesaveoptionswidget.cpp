/***************************************************************************
                                komparesaveoptionswidget.cpp
                                ----------------------------
        begin                   : Sun Mar 4 2001
        Copyright 2001-2003 Otto Bruggeman <otto.bruggeman@home.nl>
        Copyright 2001-2003 John Firebaugh <jfirebaugh@kde.org>
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

#include "komparesaveoptionswidget.h"

#include <QCheckBox>
#include <QLabel>
#include <QRadioButton>
#include <QSpinBox>

#include <kio/global.h>
#include <kurlrequester.h>

#include <libkomparediff2/diffsettings.h>

KompareSaveOptionsWidget::KompareSaveOptionsWidget( QString source, QString destination,
           DiffSettings * settings, QWidget * parent )
	: KompareSaveOptionsBase( parent )
	, m_source( source )
	, m_destination( destination )
	, m_FormatBG( new QButtonGroup(this) )
{
	setObjectName("save options");

	m_settings = settings;

	m_directoryRequester->setMode(
	    KFile::ExistingOnly | KFile::Directory | KFile::LocalOnly );

	QUrl sourceURL;
	QUrl destinationURL;
	sourceURL.setPath( source );
	destinationURL.setPath( destination );

	// Find a common root.
	QUrl root( sourceURL );
	while( root.isValid() && !root.isParentOf( destinationURL ) && root != KIO::upUrl(root) ) {
		root = KIO::upUrl(root);
	}

	// If we found a common root, change to that directory and
	// strip the common part from source and destination.
	if( root.isValid() && root != KIO::upUrl(root) ) {
		m_directoryRequester->setUrl(root);
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

	m_FormatBG->setExclusive(true);
	m_FormatBG->addButton(m_ContextRB, Kompare::Context);
	m_FormatBG->addButton(m_EdRB, Kompare::Ed);
	m_FormatBG->addButton(m_NormalRB, Kompare::Normal);
	m_FormatBG->addButton(m_UnifiedRB, Kompare::Unified);
	m_FormatBG->addButton(m_RCSRB, Kompare::RCS);
	m_FormatBG->addButton(m_SideBySideRB, Kompare::SideBySide);

	loadOptions();
}

KompareSaveOptionsWidget::~KompareSaveOptionsWidget()
{

}

QString KompareSaveOptionsWidget::directory() const
{
	return QUrl( m_directoryRequester->url() ).toLocalFile();
}

void KompareSaveOptionsWidget::updateCommandLine()
{
	QString cmdLine = "diff";

	QString options = "";

	switch( static_cast<Kompare::Format>( m_FormatBG->checkedId() ) ) {
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

	QDir dir( directory() );
	cmdLine += " -- ";
	cmdLine += dir.relativeFilePath( m_source );
	cmdLine += ' ';
	cmdLine += dir.relativeFilePath( m_destination );

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

	m_FormatBG->button(m_settings->m_format)->setChecked(true);

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

	m_settings->m_format = static_cast<Kompare::Format>( m_FormatBG->checkedId() );

}
