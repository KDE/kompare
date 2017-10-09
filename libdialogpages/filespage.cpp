/***************************************************************************
                                filespage.cpp
                                -------------
        begin                   : Sun Apr 18 2004
        Copyright 2004      Otto Bruggeman <otto.bruggeman@home.nl>
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

#include "filespage.h"

#include <QLayout>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <kcharsets.h>
#include <kconfig.h>
#include <klocalizedstring.h>
#include <kurlcombobox.h>
#include <kurlrequester.h>

#include "filessettings.h"

QUrl urlFromArg(const QString &arg);

FilesPage::FilesPage() : QFrame()
{
	QVBoxLayout* layout = new QVBoxLayout( this );

	m_firstGB = new QGroupBox( "You have to set this moron :)", this );
	layout->addWidget( m_firstGB );
	QHBoxLayout* gb1Layout = new QHBoxLayout( m_firstGB );
	m_firstURLComboBox = new KUrlComboBox( KUrlComboBox::Both, true, m_firstGB );
	m_firstURLComboBox->setMaxItems( 10 );
	m_firstURLComboBox->setObjectName( "SourceURLComboBox" );
	m_firstURLRequester = new KUrlRequester(m_firstURLComboBox, nullptr);
	gb1Layout->addWidget( m_firstURLComboBox );
	m_firstURLComboBox->setFocus();

	QPushButton* button = new QPushButton( "", this );
	button->setIcon( QIcon::fromTheme("document-open") );
	button->setToolTip( i18n( "Select File" ) );
	button->setProperty( "combobox", "SourceURLComboBox" );
	button->setProperty( "folder", false );
	connect( button, SIGNAL(clicked()), this, SLOT(open()) );
	gb1Layout->addWidget( button );

	button = new QPushButton( "", this );
	button->setIcon( QIcon::fromTheme("folder-open") );
	QSizePolicy sp = button->sizePolicy();
	sp.setRetainSizeWhenHidden( true );
	button->setSizePolicy( sp );
	button->setObjectName( "firstURLOpenFolder" );
	button->setToolTip( i18n( "Select Folder" ) );
	button->setProperty( "combobox", "SourceURLComboBox" );
	button->setProperty( "folder", true );
	connect( button, SIGNAL(clicked()), this, SLOT(open()) );
	gb1Layout->addWidget( button );

	m_secondGB = new QGroupBox( "This too moron !", this );
	layout->addWidget( m_secondGB );
	QHBoxLayout* gb2Layout = new QHBoxLayout( m_secondGB );
	m_secondURLComboBox = new KUrlComboBox( KUrlComboBox::Both, true, m_secondGB );
	m_secondURLComboBox->setMaxItems( 10 );
	m_secondURLComboBox->setObjectName( "DestURLComboBox" );
	m_secondURLRequester = new KUrlRequester(m_secondURLComboBox, nullptr);
	gb2Layout->addWidget( m_secondURLComboBox );

	button = new QPushButton( "", this );
	button->setIcon( QIcon::fromTheme("document-open") );
	button->setToolTip( i18n( "Select File" ) );
	button->setProperty( "combobox", "DestURLComboBox" );
	button->setProperty( "folder", false );
	connect( button, SIGNAL(clicked()), this, SLOT(open()) );
	gb2Layout->addWidget( button );

	button = new QPushButton( "", this );
	button->setIcon( QIcon::fromTheme("folder-open") );
	button->setObjectName( "secondURLOpenFolder" );
	sp = button->sizePolicy();
	sp.setRetainSizeWhenHidden( true );
	button->setSizePolicy( sp );
	button->setToolTip( i18n( "Select Folder" ) );
	button->setProperty( "combobox", "DestURLComboBox" );
	button->setProperty( "folder", true );
	connect( button, SIGNAL(clicked()), this, SLOT(open()) );
	gb2Layout->addWidget( button );


	m_thirdGB = new QGroupBox( i18n( "Encoding" ), this );
	layout->addWidget( m_thirdGB );
	QHBoxLayout* gb3Layout = new QHBoxLayout( m_thirdGB );
	m_encodingComboBox = new KComboBox( false, m_thirdGB );
	m_encodingComboBox->setObjectName( "encoding_combobox" );
	m_encodingComboBox->insertItem( 0, i18n( "Default" ) );
	m_encodingComboBox->insertItems( 1, KCharsets::charsets()->availableEncodingNames() );
	gb3Layout->addWidget( m_encodingComboBox );

	layout->addWidget( m_firstGB );
	layout->addWidget( m_secondGB );
	layout->addWidget( m_thirdGB );

	layout->addStretch( 1 );
}

FilesPage::~FilesPage()
{
	delete m_firstURLRequester;
	m_firstURLRequester = nullptr;
	delete m_secondURLRequester;
	m_secondURLRequester = nullptr;
	m_settings = nullptr;
}

void FilesPage::open()
{
	QPushButton* button = ( QPushButton* ) sender();
	bool selectFolders = button->property( "folder" ).toBool();
	KUrlComboBox* urlComboBox = findChild<KUrlComboBox*>( button->property( "combobox" ).toString() );

	open( urlComboBox, selectFolders );
}

void FilesPage::open( KUrlComboBox *urlComboBox, bool selectFolders )
{
	QUrl currentUrl = QUrl::fromUserInput( urlComboBox->currentText() );

	QUrl newUrl = selectFolders ? QFileDialog::getExistingDirectoryUrl( this,
																	i18n( "Select Folder" ),
																	currentUrl,
																	QFileDialog::ReadOnly )
								: QFileDialog::getOpenFileUrl( this,
															   i18n( "Select File" ),
															   currentUrl );
	if ( !newUrl.isEmpty() )
	{
		// remove newUrl if already exists and add it as the first item
		urlComboBox->setUrl( newUrl );
	}

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
//	qDebug() << "first : " << m_firstURLComboBox->currentText() ;
//	qDebug() << "second: " << m_secondURLComboBox->currentText() ;
	m_firstURLComboBox->setUrl( urlFromArg( m_firstURLComboBox->currentText() ) );
	m_secondURLComboBox->setUrl( urlFromArg( m_secondURLComboBox->currentText() ) );
}


void FilesPage::setFirstURLRequesterMode( unsigned int mode )
{
	m_firstURLRequester->setMode( (KFile::Mode) mode );
    if ( (mode & KFile::Directory) == 0 )
	{
		QPushButton *button = findChild<QPushButton *>( "firstURLOpenFolder" );
		button->setVisible( false );
	}
}

void FilesPage::setSecondURLRequesterMode( unsigned int mode )
{
	m_secondURLRequester->setMode( (KFile::Mode) mode );
    if ( (mode & KFile::Directory) == 0 )
	{
		QPushButton *button = findChild<QPushButton *>( "secondURLOpenFolder" );
		button->setVisible( false );
	}
}

void FilesPage::setSettings( FilesSettings* settings )
{
	m_settings = settings;

	m_firstURLComboBox->setUrls( m_settings->m_recentSources );
	m_firstURLComboBox->setUrl( urlFromArg( m_settings->m_lastChosenSourceURL ) );
	m_secondURLComboBox->setUrls( m_settings->m_recentDestinations );
	m_secondURLComboBox->setUrl( urlFromArg( m_settings->m_lastChosenDestinationURL ) );
	m_encodingComboBox->setCurrentIndex( m_encodingComboBox->findText( m_settings->m_encoding, Qt::MatchFixedString ) );
}

void FilesPage::restore()
{
	// this shouldn't do a thing...
}

void FilesPage::apply()
{
	// set the current items as the first ones
	m_firstURLComboBox->insertUrl( 0, QUrl( m_firstURLComboBox->currentText() ) );
	m_secondURLComboBox->insertUrl( 0, QUrl( m_secondURLComboBox->currentText() ) );

	m_settings->m_recentSources            = m_firstURLComboBox->urls();
	m_settings->m_lastChosenSourceURL      = m_firstURLComboBox->currentText();
	m_settings->m_recentDestinations       = m_secondURLComboBox->urls();
	m_settings->m_lastChosenDestinationURL = m_secondURLComboBox->currentText();
	m_settings->m_encoding                 = m_encodingComboBox->currentText();
}

void FilesPage::setDefaults()
{
	m_firstURLComboBox->setUrls( QStringList() );
	m_firstURLComboBox->setUrl( QUrl() );
	m_secondURLComboBox->setUrls( QStringList() );
	m_secondURLComboBox->setUrl( QUrl() );
	m_encodingComboBox->setCurrentIndex( 0 ); // "Default"
}
