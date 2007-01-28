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
#include <kapplication.h>
#include <klocale.h>
#include <kurlrequester.h>
#include <kglobal.h>

#include "diffpage.h"
#include "diffsettings.h"
#include "filespage.h"
#include "filessettings.h"
#include "viewpage.h"
#include "viewsettings.h"

#include "kompareurldialog.h"

KompareURLDialog::KompareURLDialog( QWidget *parent, Qt::WFlags flags )
        : KPageDialog( parent, flags )
{
    setFaceType( List );
	KSharedConfig::Ptr cfg = KGlobal::config();

// 	QVBox* filesBox = addVBoxPage( i18n( "Files" ), i18n( "Here you can enter the files you want to compare." ) );
	m_filesPage = new FilesPage( 0 );
	m_filesSettings = new FilesSettings( this );
	m_filesSettings->loadSettings( cfg );
	m_filesPage->setSettings( m_filesSettings );
   	KPageWidgetItem *filesItem = new KPageWidgetItem( m_filesPage, i18n( "My Second Test Page" ) );
	addPage( filesItem );

// 	QVBox* diffBox = addVBoxPage( i18n( "Diff" ), i18n( "Here you can change the options for comparing the files." ) );
// 	m_diffPage = new DiffPage( diffBox );
// 	m_diffSettings = new DiffSettings( this );
// 	m_diffSettings->loadSettings( cfg );
// 	m_diffPage->setSettings( m_diffSettings );
//
// 	QVBox* viewBox = addVBoxPage( i18n( "Appearance" ), i18n( "Here you can change the options for the view." ) );
// 	m_viewPage = new ViewPage( viewBox );
// 	m_viewSettings = new ViewSettings( this );
// 	m_viewSettings->loadSettings( cfg );
// 	m_viewPage->setSettings( m_viewSettings );

	adjustSize();

	showButtonSeparator( true );

	connect( m_filesPage->firstURLRequester(), SIGNAL( textChanged( const QString& ) ),
	         this, SLOT( slotEnableOk() ) );
	connect( m_filesPage->secondURLRequester(), SIGNAL( textChanged( const QString& ) ),
	         this, SLOT( slotEnableOk() ) );

	slotEnableOk();
}

KompareURLDialog::~KompareURLDialog()
{
}

void KompareURLDialog::slotOk()
{
	m_filesPage->setURLsInComboBoxes();

	KSharedConfig::Ptr cfg = KGlobal::config();

	m_filesPage->apply();
	m_diffPage->apply();
	m_viewPage->apply();

	m_filesSettings->saveSettings( cfg );
	m_diffSettings->saveSettings( cfg );
	m_viewSettings->saveSettings( cfg );

	cfg->sync();

/*	KDialogBase::slotOk();*/
}

void KompareURLDialog::slotEnableOk()
{
	enableButtonOk( !m_filesPage->firstURLRequester()->url().isEmpty() &&
	                !m_filesPage->secondURLRequester()->url().isEmpty() );
}

KUrl KompareURLDialog::getFirstURL() const
{
	return KUrl( m_filesPage->firstURLRequester()->url() );
}

KUrl KompareURLDialog::getSecondURL() const
{
	return KUrl( m_filesPage->secondURLRequester()->url() );
}

QString KompareURLDialog::encoding() const
{
	return m_filesPage->encoding();
}

void KompareURLDialog::setFirstGroupBoxTitle( const QString& title )
{
	m_filesPage->setFirstGroupBoxTitle( title );
}

void KompareURLDialog::setSecondGroupBoxTitle( const QString& title )
{
	m_filesPage->setSecondGroupBoxTitle( title );
}

void KompareURLDialog::setGroup( const QString& groupName )
{
	m_filesSettings->setGroup( groupName );
	m_filesSettings->loadSettings( KGlobal::config() );
	m_filesPage->setSettings( m_filesSettings );
}

void KompareURLDialog::setFirstURLRequesterMode( unsigned int mode )
{
	m_filesPage->setFirstURLRequesterMode( mode );
}

void KompareURLDialog::setSecondURLRequesterMode( unsigned int mode )
{
	m_filesPage->setSecondURLRequesterMode( mode );
}

#include "kompareurldialog.moc"

