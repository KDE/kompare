/***************************************************************************
                                comparedialog.cpp  -  description
                                -------------------
        begin                   : Sun Mar 4 2001
        copyright               : (C) 2001-2004 Otto Bruggeman
                                  (C) 2001-2003 John Firebaugh
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

#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kcharsets.h>
#include <klocale.h>
#include <kurlcombobox.h>
#include <kurlrequester.h>

#include "kompareurldialog.h"

KompareURLDialog::KompareURLDialog( const KURL* firstURL, const KURL* secondURL,
                                    QWidget *parent, const char *name )
        : KDialogBase( Plain, "", Ok|Cancel, Ok, parent, name ),
		m_config( 0 )
{
	QVBoxLayout* topLayout = new QVBoxLayout( plainPage(), 0,
	          spacingHint() );

	m_firstGB = new QGroupBox( "You have to set this moron :)", plainPage() );
	m_firstGB->setColumnLayout(0, Qt::Vertical );
	m_firstGB->layout()->setSpacing( 0 );
	m_firstGB->layout()->setMargin( 0 );
	QHBoxLayout* firstGBLayout = new QHBoxLayout( m_firstGB->layout() );
	firstGBLayout->setAlignment( Qt::AlignVCenter );
	firstGBLayout->setSpacing( 6 );
	firstGBLayout->setMargin( 11 );

	m_firstURLComboBox = new KURLComboBox( KURLComboBox::Both, true, this, "SourceURLComboBox" );

	if( firstURL ) {
		m_firstURLComboBox->setURL( *firstURL );
	}

	m_firstURLRequester = new KURLRequester( m_firstURLComboBox, m_firstGB );
	m_firstURLRequester->setFocus();

	firstGBLayout->addWidget( m_firstURLRequester );
	topLayout->addWidget( m_firstGB );

	m_secondGB = new QGroupBox( "This too moron !", plainPage() );
	m_secondGB->setColumnLayout(0, Qt::Vertical );
	m_secondGB->layout()->setSpacing( 0 );
	m_secondGB->layout()->setMargin( 0 );
	QHBoxLayout* secondGBLayout = new QHBoxLayout( m_secondGB->layout() );
	secondGBLayout->setAlignment( Qt::AlignVCenter );
	secondGBLayout->setSpacing( 6 );
	secondGBLayout->setMargin( 11 );

	m_secondURLComboBox = new KURLComboBox( KURLComboBox::Both, true, this, "DestURLComboBox" );

	if( secondURL ) {
		m_secondURLComboBox->setURL( *secondURL );
	}

	m_secondURLRequester = new KURLRequester( m_secondURLComboBox, m_secondGB );

	secondGBLayout->addWidget( m_secondURLRequester );
	topLayout->addWidget( m_secondGB );

	m_firstURLRequester->setMinimumWidth( 400 );
	m_secondURLRequester->setMinimumWidth( 400 );

	connect( m_firstURLRequester, SIGNAL( textChanged( const QString& ) ),
	         this, SLOT( slotEnableOk() ) );
	connect( m_secondURLRequester, SIGNAL( textChanged( const QString& ) ),
	         this, SLOT( slotEnableOk() ) );

	enableButtonSeparator( true );

	slotEnableOk();
}

KompareURLDialog::~KompareURLDialog()
{
	if ( !m_config )
		m_config = kapp->config();

	m_config->setGroup( m_configGroupName );
//	kdDebug(8100) << "Recent Sources     : " << m_firstURLComboBox->urls().join( " " ) << endl;
//	kdDebug(8100) << "Recent Destinations: " << m_secondURLComboBox->urls().join( " " ) << endl;
	// Keeping these names so i wont have to write more kconfupdate scripts
	m_config->writeEntry( "Recent Sources", m_firstURLComboBox->urls() );
	m_config->writeEntry( "Recent Destinations", m_secondURLComboBox->urls() );
	m_config->writeEntry( "LastChosenSourceListEntry", m_firstURLComboBox->currentText() );
	m_config->writeEntry( "LastChosenDestinationListEntry", m_secondURLComboBox->currentText() );
	m_config->sync();
}

void KompareURLDialog::slotOk()
{
	m_firstURLComboBox->setURL( KURL( m_firstURLComboBox->currentText() ) );
	m_secondURLComboBox->setURL( KURL( m_secondURLComboBox->currentText() ) );

	KDialogBase::slotOk();
}

void KompareURLDialog::slotEnableOk()
{
	enableButtonOK( !m_firstURLRequester->url().isEmpty() &&
	                !m_secondURLRequester->url().isEmpty() );
}

KURL KompareURLDialog::getFirstURL() const
{
	if ( result() == QDialog::Accepted )
		return KURL( m_firstURLRequester->url() );
	else
		return KURL();
}

KURL KompareURLDialog::getSecondURL() const
{
	if ( result() == QDialog::Accepted )
		return KURL( m_secondURLRequester->url() );
	else
		return KURL();
}

void KompareURLDialog::setFirstGroupBoxTitle( const QString& title )
{
	m_firstGB->setTitle( title );
}

void KompareURLDialog::setSecondGroupBoxTitle( const QString& title )
{
	m_secondGB->setTitle( title );
}

void KompareURLDialog::setGroup( const QString& groupName )
{
	QStringList urlList;
	QString lastChosenEntry;

	if ( !m_config )
		m_config = kapp->config();

	m_configGroupName = groupName;

	kdDebug() << "Groupname: " << groupName << endl;
	m_config->setGroup( groupName );

	urlList = m_config->readListEntry( "Recent Sources" );
	lastChosenEntry = m_config->readEntry( "LastChosenSourceListEntry", "" );
	kdDebug() << "Recent Sources: " << urlList.join("\n") << endl;
	kdDebug() << "LastChosenSourceListEntry: " << lastChosenEntry << endl;
	m_firstURLComboBox->setURLs( urlList );
	m_firstURLComboBox->setURL( KURL( lastChosenEntry ) );

	urlList = m_config->readListEntry( "Recent Destinations" );
	lastChosenEntry = m_config->readEntry( "LastChosenDestinationListEntry", "" );
	kdDebug() << "Recent Destinations: " << urlList.join("\n") << endl;
	kdDebug() << "LastChosenDestinationListEntry: " << lastChosenEntry << endl;
	m_secondURLComboBox->setURLs( urlList );
	m_secondURLComboBox->setURL( KURL( lastChosenEntry ) );
}

void KompareURLDialog::setFirstURLRequesterMode( unsigned int mode )
{
	m_firstURLRequester->setMode( mode );
}

void KompareURLDialog::setSecondURLRequesterMode( unsigned int mode )
{
	m_secondURLRequester->setMode( mode );
}

#include "kompareurldialog.moc"
