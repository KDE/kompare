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

#include <qvbox.h>

#include <kapplication.h>
#include <klocale.h>
#include <kurlrequester.h>

#include "diffpage.h"
#include "diffsettings.h"
#include "filespage.h"
#include "filessettings.h"
#include "viewpage.h"
#include "viewsettings.h"

#include "kompareurldialog.h"

KompareURLDialog::KompareURLDialog( const KURL* firstURL, const KURL* secondURL,
                                    QWidget *parent, const char *name )
        : KDialogBase( IconList, "", Ok|Cancel, Ok, parent, name )
{
	setIconListAllVisible(true);

	QVBox* filesPage = addVBoxPage( i18n( "Files" ), i18n( "Enter here the files you want to compare." ) );
	m_filesPage = new FilesPage( filesPage );
	m_filesSettings = new FilesSettings( this );

	QVBox* diffPage  = addVBoxPage( i18n( "Diff" ), i18n( "Here you can change the options for comparing the files." ) );
	DiffPage* dp = new DiffPage( diffPage );
	DiffSettings* ds = new DiffSettings( this );
	ds->loadSettings( kapp->config() );
	dp->setSettings( ds );

	QVBox* viewPage  = addVBoxPage( i18n( "Appearance" ), i18n( "Here you can change the options for the view." ) );
	ViewPage* vp = new ViewPage( viewPage );
	ViewSettings* vs = new ViewSettings( this );
	vs->loadSettings( kapp->config() );
	vp->setSettings( vs );

	adjustSize();

	enableButtonSeparator( true );

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

	KDialogBase::slotOk();
}

void KompareURLDialog::slotEnableOk()
{
	enableButtonOK( !m_filesPage->firstURLRequester()->url().isEmpty() &&
	                !m_filesPage->secondURLRequester()->url().isEmpty() );
}

KURL KompareURLDialog::getFirstURL() const
{
	if ( result() == QDialog::Accepted )
		return KURL( m_filesPage->firstURLRequester()->url() );
	else
		return KURL();
}

KURL KompareURLDialog::getSecondURL() const
{
	if ( result() == QDialog::Accepted )
		return KURL( m_filesPage->secondURLRequester()->url() );
	else
		return KURL();
}

QString KompareURLDialog::encoding() const
{
	if ( result() == QDialog::Accepted )
		return m_filesPage->encoding();
		return QString( "default" );
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
	m_filesSettings->loadSettings( kapp->config() );
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

