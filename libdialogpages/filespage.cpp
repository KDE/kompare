/***************************************************************************
                                filespage.cpp
                                -------------
        begin                   : Sun Apr 18 2004
        Copyright 2004 Otto Bruggeman <otto.bruggeman@home.nl>
        Copyright 2007 Kevin Kofler   <kevin.kofler@chello.at>
****************************************************************************/

/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***************************************************************************/

#include "filespage.h"

#include <QtGui/QLayout>
#include <QGroupBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>

#include <kapplication.h>
#include <kcharsets.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <kurlcombobox.h>
#include <kurlrequester.h>

#include "filessettings.h"

FilesPage::FilesPage() : PageBase()
{
	QWidget* page = new QWidget( this );
	QVBoxLayout* layout = new QVBoxLayout( page );
	layout->setSpacing( KDialog::spacingHint() );
	layout->setMargin( KDialog::marginHint() );

	m_firstGB = new QGroupBox( "You have to set this moron :)", page );
	layout->addWidget( m_firstGB );
	QHBoxLayout* gb1Layout = new QHBoxLayout( m_firstGB );
	m_firstURLComboBox = new KUrlComboBox( KUrlComboBox::Both, true, m_firstGB );
	m_firstURLComboBox->setObjectName( "SourceURLComboBox" );
	m_firstURLRequester = new KUrlRequester( m_firstURLComboBox, m_firstGB );
	gb1Layout->addWidget( m_firstURLRequester );
	m_firstURLRequester->setFocus();

	m_secondGB = new QGroupBox( "This too moron !", page );
	layout->addWidget( m_secondGB );
	QHBoxLayout* gb2Layout = new QHBoxLayout( m_secondGB );
	m_secondURLComboBox = new KUrlComboBox( KUrlComboBox::Both, true, m_secondGB );
	m_secondURLComboBox->setObjectName( "DestURLComboBox" );
	m_secondURLRequester = new KUrlRequester( m_secondURLComboBox, m_secondGB );
	gb2Layout->addWidget( m_secondURLRequester );

	m_thirdGB = new QGroupBox( i18n( "Encoding" ), page );
	layout->addWidget( m_thirdGB );
	QHBoxLayout* gb3Layout = new QHBoxLayout( m_thirdGB );
	m_encodingComboBox = new KComboBox( false, m_thirdGB );
	m_encodingComboBox->setObjectName( "encoding_combobox" );
	m_encodingComboBox->insertStringList( KGlobal::charsets()->availableEncodingNames() );
	gb3Layout->addWidget( m_encodingComboBox );

	layout->addWidget( m_firstGB );
	layout->addWidget( m_secondGB );
	layout->addWidget( m_thirdGB );

	layout->addStretch( 1 );
	page->setMinimumSize( sizeHintForWidget( page ) );

	//addTab( page, i18n( "&Files" ) );
}

FilesPage::~FilesPage()
{
	m_settings = 0;
}

KUrlRequester* FilesPage::firstURLRequester() const
{
	return m_firstURLRequester;
}

KUrlRequester* FilesPage::secondURLRequester() const
{
	return m_secondURLRequester;
}

QString FilesPage::encoding() const
{
	return m_encodingComboBox->currentText();
}

void FilesPage::setFirstGroupBoxTitle( const QString& title )
{
	m_firstGB->setTitle( title );
}

void FilesPage::setSecondGroupBoxTitle( const QString& title )
{
	m_secondGB->setTitle( title );
}

void FilesPage::setURLsInComboBoxes()
{
//	kDebug() << "first : " << m_firstURLComboBox->currentText() << endl;
//	kDebug() << "second: " << m_secondURLComboBox->currentText() << endl;
	m_firstURLComboBox->setUrl( KUrl( m_firstURLComboBox->currentText() ) );
	m_secondURLComboBox->setUrl( KUrl( m_secondURLComboBox->currentText() ) );
}


void FilesPage::setFirstURLRequesterMode( unsigned int mode )
{
	m_firstURLRequester->setMode( (KFile::Mode) mode );
}

void FilesPage::setSecondURLRequesterMode( unsigned int mode )
{
	m_secondURLRequester->setMode( (KFile::Mode) mode );
}

void FilesPage::setSettings( FilesSettings* settings )
{
	m_settings = settings;

	m_firstURLComboBox->setUrls( m_settings->m_recentSources );
	m_firstURLComboBox->setUrl( KUrl( m_settings->m_lastChosenSourceURL ) );
	m_secondURLComboBox->setUrls( m_settings->m_recentDestinations );
	m_secondURLComboBox->setUrl( KUrl( m_settings->m_lastChosenDestinationURL ) );
	m_encodingComboBox->setCurrentText( m_settings->m_encoding );
}

void FilesPage::restore()
{
	// this shouldn't do a thing...
}

void FilesPage::apply()
{
	m_settings->m_recentSources            = m_firstURLComboBox->urls();
	m_settings->m_lastChosenSourceURL      = m_firstURLComboBox->currentText();
	m_settings->m_recentDestinations       = m_secondURLComboBox->urls();
	m_settings->m_lastChosenDestinationURL = m_secondURLComboBox->currentText();
	m_settings->m_encoding                 = m_encodingComboBox->currentText();
}

void FilesPage::setDefaults()
{
	m_firstURLComboBox->setUrls( QStringList() );
	m_firstURLComboBox->setUrl( KUrl( "" ) );
	m_secondURLComboBox->setUrls( QStringList() );
	m_secondURLComboBox->setUrl( KUrl( "" ) );
	m_encodingComboBox->setCurrentText( "Default" );
}

#include "filespage.moc"
