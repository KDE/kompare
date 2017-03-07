/***************************************************************************
                                kompareurldialog.cpp
                                --------------------
        begin                   : Sun Mar 4 2001
        Copyright 2001-2005,2009 Otto Bruggeman <bruggie@gmail.com>
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
#include "kompareurldialog.h"
#include <QShowEvent>

#include <QGroupBox>

#include <klocalizedstring.h>
#include <kmessagebox.h>
#include <ksharedconfig.h>
#include <kurlcombobox.h>
#include <kurlrequester.h>

#include <libkomparediff2/diffsettings.h>

#include "diffpage.h"
#include "filespage.h"
#include "filessettings.h"
#include "viewpage.h"
#include "viewsettings.h"

Q_DECLARE_LOGGING_CATEGORY(KOMPARESHELL)

KompareURLDialog::KompareURLDialog( QWidget *parent, Qt::WindowFlags flags )
        : KPageDialog( parent, flags )
{
	setFaceType( List );
	KSharedConfig::Ptr cfg = KSharedConfig::openConfig();

	m_filesPage = new FilesPage();
	KPageWidgetItem *filesItem = addPage( m_filesPage, i18n( "Files" ) );
	filesItem->setIcon( QIcon::fromTheme( "text-plain" ) );
	filesItem->setHeader( i18n( "Here you can enter the files you want to compare." ) );
	m_filesSettings = new FilesSettings( this );
	m_filesSettings->loadSettings( cfg.data() );
	m_filesPage->setSettings( m_filesSettings );

	m_diffPage = new DiffPage();
	KPageWidgetItem *diffItem = addPage( m_diffPage, i18n( "Diff" ) );
	diffItem->setIcon( QIcon::fromTheme( "text-x-patch" ) );
	diffItem->setHeader( i18n( "Here you can change the options for comparing the files." ) );
	m_diffSettings = new DiffSettings( this );
	m_diffSettings->loadSettings( cfg.data() );
	m_diffPage->setSettings( m_diffSettings );

	m_viewPage = new ViewPage();
	KPageWidgetItem *viewItem = addPage( m_viewPage, i18n( "Appearance" ) );
	viewItem->setIcon( QIcon::fromTheme( "preferences-desktop-theme" ) );
	viewItem->setHeader( i18n( "Here you can change the options for the view." ) );
	m_viewSettings = new ViewSettings( this );
	m_viewSettings->loadSettings( cfg.data() );
	m_viewPage->setSettings( m_viewSettings );

	adjustSize();

	connect( m_filesPage->firstURLRequester(), SIGNAL( textChanged( const QString& ) ),
	         this, SLOT( slotEnableOk() ) );
	connect( m_filesPage->secondURLRequester(), SIGNAL( textChanged( const QString& ) ),
	         this, SLOT( slotEnableOk() ) );

}

KompareURLDialog::~KompareURLDialog()
{
}

void KompareURLDialog::showEvent ( QShowEvent * event )
{
    if ( !event->spontaneous () )
    {
        slotEnableOk();
    }
}

void KompareURLDialog::slotButtonClicked( int button )
{
	if ( button == QDialogButtonBox::Cancel )
	{
		reject();
		return;
	}
	// BUG: 124121 File with filenames to be excluded does not exist so diff complains and no diffs are generated
	qCDebug(KOMPARESHELL) << "Test to see if the file is an actual file that is given in the file with filenames to exclude field" ;
	if ( m_diffPage->m_excludeFileNameGroupBox->isChecked() )
	{
		qCDebug(KOMPARESHELL) << "Ok the checkbox is active..." ;
		if ( QFileInfo( m_diffPage->m_excludeFileURLComboBox->currentText() ).isDir() )
		{
			qCDebug(KOMPARESHELL) << "Don't enter directory names where filenames are expected..." ;
			KMessageBox::sorry( this, i18n( "File used for excluding files cannot be found, please specify another file." ) );
			reject();
			return;
		}
	}
	// Room for more checks for invalid input

	m_filesPage->setURLsInComboBoxes();

	KSharedConfig::Ptr cfg = KSharedConfig::openConfig();

	m_filesPage->apply();
	m_diffPage->apply();
	m_viewPage->apply();

	m_filesSettings->saveSettings( cfg.data() );
	m_diffSettings->saveSettings( cfg.data() );
	m_viewSettings->saveSettings( cfg.data() );

	cfg->sync();

	accept();
}

void KompareURLDialog::slotEnableOk()
{
	button(QDialogButtonBox::Ok)->setEnabled( !m_filesPage->firstURLRequester()->url().isEmpty() &&
	                !m_filesPage->secondURLRequester()->url().isEmpty() );
}

/**
 * Returns the first URL, which was entered.
 * @return first URL
 */
QUrl KompareURLDialog::getFirstURL() const
{
	return m_filesPage->firstURLRequester()->url();
}

/**
 * Returns the second URL, which was entered.
 * @return second URL
 */
QUrl KompareURLDialog::getSecondURL() const
{
	return m_filesPage->secondURLRequester()->url();
}

/**
 * Returns the encoding.
 * @return encoding
 */
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
	m_filesSettings->loadSettings( KSharedConfig::openConfig().data() );
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
