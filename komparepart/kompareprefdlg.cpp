/***************************************************************************
                                kompareprefdlg.cpp
                                ------------------
        begin                   : Sun Mar 4 2001
        Copyright 2001-2009 Otto Bruggeman <bruggie@gmail.com>
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

#include "kompareprefdlg.h"

#include <QTabWidget>
#include <QPushButton>

#include <klocalizedstring.h>
#include <khelpclient.h>
#include <kstandardguiitem.h>

#include "diffpage.h"
#include "viewpage.h"

// implementation

KomparePrefDlg::KomparePrefDlg( ViewSettings* viewSets, DiffSettings* diffSets ) : KPageDialog( 0 )
{
	setFaceType( KPageDialog::List );
	setWindowTitle( i18n( "Preferences" ) );
	setStandardButtons( QDialogButtonBox::Help|QDialogButtonBox::Reset|QDialogButtonBox::Ok|QDialogButtonBox::Apply|QDialogButtonBox::Cancel );
	setModal( true );

	KGuiItem::assign( button( QDialogButtonBox::Reset ), KStandardGuiItem::defaults() );

	// ok i need some stuff in that pref dlg...
	//setIconListAllVisible(true);

	m_viewPage = new ViewPage();
	KPageWidgetItem *item = addPage( m_viewPage, i18n( "View" ) );
	item->setIcon( QIcon::fromTheme( "preferences-desktop-theme" ) );
	item->setHeader( i18n( "View Settings" ) );
	m_viewPage->setSettings( viewSets );

	m_diffPage = new DiffPage();
	item = addPage( m_diffPage, i18n( "Diff" ) );
	item->setIcon( QIcon::fromTheme( "text-x-patch" ) );
	item->setHeader( i18n( "Diff Settings" ) );
	m_diffPage->setSettings( diffSets );

//	frame = addVBoxPage( i18n( "" ), i18n( "" ), UserIcon( "" ) );

	connect( button( QDialogButtonBox::Reset ), &QPushButton::clicked, this, &KomparePrefDlg::slotDefault );
	connect( button( QDialogButtonBox::Help ), &QPushButton::clicked, this, &KomparePrefDlg::slotHelp );
	connect( button( QDialogButtonBox::Apply ), &QPushButton::clicked, this, &KomparePrefDlg::slotApply );
	connect( button( QDialogButtonBox::Ok ), &QPushButton::clicked, this, &KomparePrefDlg::slotOk );
	connect( button( QDialogButtonBox::Cancel ), &QPushButton::clicked, this, &KomparePrefDlg::slotCancel );

	adjustSize();
}

KomparePrefDlg::~KomparePrefDlg()
{

}

/** No descriptions */
void KomparePrefDlg::slotDefault()
{
	// restore all defaults in the options...
	m_viewPage->setDefaults();
	m_diffPage->setDefaults();
}

/** No descriptions */
void KomparePrefDlg::slotHelp()
{
	// figure out the current active page
	QWidget* currentpage = currentPage()->widget();
	if ( dynamic_cast<ViewPage*>(currentpage) )
	{
		// figure out the active tab
		int currentTab = static_cast<ViewPage*>(currentpage)->m_tabWidget->currentIndex();
		switch ( currentTab )
		{
		case 0:
			KHelpClient::invokeHelp( "appearance" );
			break;
		case 1:
			KHelpClient::invokeHelp( "fonts" );
			break;
		default:
			KHelpClient::invokeHelp( "view-settings" );
		}
	}
	else if ( dynamic_cast<DiffPage*>(currentpage) )
	{
		// figure out the active tab
		int currentTab = static_cast<DiffPage*>(currentpage)->m_tabWidget->currentIndex();
		switch ( currentTab )
		{
		case 0:
			KHelpClient::invokeHelp( "diff" );
			break;
		case 1:
			KHelpClient::invokeHelp( "diff-format" );
			break;
		case 2:
			KHelpClient::invokeHelp( "options" );
			break;
		case 3:
			KHelpClient::invokeHelp( "exclude" );
			break;
		default:
			KHelpClient::invokeHelp( "diff-settings" );
		}
	}
	else // Fallback since we had not added the code for the page/tab or forgotten about it
		KHelpClient::invokeHelp( "configure-preferences" );
}

/** No descriptions */
void KomparePrefDlg::slotApply()
{
	// well apply the settings that are currently selected
	m_viewPage->apply();
	m_diffPage->apply();

	emit configChanged();
}

/** No descriptions */
void KomparePrefDlg::slotOk()
{
	// Apply the settings that are currently selected
	m_viewPage->apply();
	m_diffPage->apply();

	//accept();
}

/** No descriptions */
void KomparePrefDlg::slotCancel()
{
	// discard the current settings and use the present ones
	m_viewPage->restore();
	m_diffPage->restore();

	//reject();
}
