/***************************************************************************
                                kdiff_shell.cpp  -  description
                                -------------------
        begin                   : Sun Mar 4 2001
        copyright               : (C) 2001 by Otto Bruggeman
                                  and John Firebaugh
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

/*
* kdiffshell.cpp
*
* Copyright (C) 2001  <kurt@granroth.org>
*/

#include "kdiff_shell.h"

#include <kkeydialog.h>
#include <kconfig.h>
#include <kurl.h>
#include <klocale.h>
#include <kedittoolbar.h>
#include <kaction.h>
#include <kstdaction.h>
#include <klibloader.h>
#include <kmessagebox.h>
#include <kfiledialog.h>

#include "kdiff_part.h"
#include "kcomparedialog.h"

#define ID_N_OF_N      1
#define ID_GENERAL     2

KDiffShell::KDiffShell()
	: KParts::DockMainWindow( 0L, "KDiffShell" )
{
	if ( !initialGeometrySet() )
	resize( 800, 480 );

	// set the shell's ui resource file
	setXMLFile("kdiffui.rc");

	// then, setup our actions
	setupActions();
	setupStatusBar();

	// and a status bar
	statusBar()->show();

	// this routine will find and load our Part.  it finds the Part by
	// name which is a bad idea usually.. but it's alright in this
	// case since our Part is made for this Shell
	KLibFactory *factory = KLibLoader::self()->factory("libkdiffpart");
	if (factory)
	{
		KDockWidget* mainDock = createDockWidget( "KDiff", kapp->icon() );

		// now that the Part is loaded, we cast it to a Part to get
		// our hands on it
		m_part = static_cast<KDiffPart *>(factory->create(mainDock,
		              "kdiff_part", "KParts::ReadWritePart" ));

		if (m_part)
		{

			mainDock->setWidget( m_part->widget() );
			setView( mainDock );
			setMainDockWidget( mainDock );

			KDockWidget* navDock = createDockWidget( "Differences", kapp->icon() );
			QWidget* navWidget = m_part->createNavigationWidget( navDock );
			navDock->setWidget( navWidget );
			navDock->manualDock( mainDock, KDockWidget::DockTop, 20 );

			connect( m_part, SIGNAL(selectionChanged(int,int)), this, SLOT(updateStatusBar()));

			// and integrate the part's GUI with the shell's
			createGUI(m_part);
		}
	}
	else
	{
		// if we couldn't find our Part, we exit since the Shell by
		// itself can't do anything useful
		KMessageBox::error(this, "Could not find our Part!");
		kapp->quit();
	}

	// Read basic main-view settings, and set to autosave
	setAutoSaveSettings( "General Options" );

	m_toolbarAction->setChecked( !toolBar()->isHidden() );
	m_statusbarAction->setChecked( !statusBar()->isHidden() );

}

KDiffShell::~KDiffShell()
{
}

void KDiffShell::load(const KURL& url)
{
	m_part->openURL( url );
}

void KDiffShell::compare(const KURL& source,const KURL& destination )
{
	m_source = source;
	m_destination = destination;
	m_part->compare( source, destination );
}

void KDiffShell::setFormat( QCString format )
{
	m_part->setFormat( format );
}

void KDiffShell::setupActions()
{
	new KAction( i18n("&Compare Files..."), "fileopen", Qt::CTRL + Qt::Key_O,
	              this, SLOT(slotFileCompareFiles()),
	              actionCollection(), "file_compare_files" );
	KStdAction::open(this, SLOT(slotFileOpen()), actionCollection());
	KStdAction::quit(kapp, SLOT(quit()), actionCollection());

	m_toolbarAction = KStdAction::showToolbar(this, SLOT(optionsShowToolbar()), actionCollection());
	m_statusbarAction = KStdAction::showStatusbar(this, SLOT(optionsShowStatusbar()), actionCollection());

	KStdAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
	KStdAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());
}

void KDiffShell::setupStatusBar()
{
	statusBar()->insertItem( i18n(" 0 of 0 differences "), ID_N_OF_N );
	statusBar()->insertItem( "", ID_GENERAL, 1 );
}

void KDiffShell::updateStatusBar()
{
	QString str;
	int markedItem = m_part->getSelectedDifferenceIndex();
	int count = m_part->getSelectedModel()->differenceCount();
	if (markedItem >= 0)
		str = i18n(" %1 of %2 differences ").arg(markedItem+1).arg(count);
	else
		str = i18n(" %1 differences ").arg(count);
	statusBar()->changeItem( str, ID_N_OF_N );
}

void KDiffShell::saveProperties(KConfig* /*config*/)
{
	// the 'config' object points to the session managed
	// config file.  anything you write here will be available
	// later when this app is restored
}

void KDiffShell::readProperties(KConfig* /*config*/)
{
	// the 'config' object points to the session managed
	// config file.  this function is automatically called whenever
	// the app is being restored.  read in here whatever you wrote
	// in 'saveProperties'
}

void KDiffShell::slotFileOpen()
{
	KURL url = KFileDialog::getOpenURL( QString::null, "text/x-diff", this );
	if( !url.isEmpty() ) {
		KDiffShell* shell = new KDiffShell();
		shell->show();
		shell->load( url );
	}
}

void KDiffShell::slotFileCompareFiles()
{
	KCompareDialog* dialog = new KCompareDialog( &m_source, &m_destination, this );
	if( dialog->exec() == QDialog::Accepted ) {
		KURL source = dialog->getSourceURL();
		KURL destination = dialog->getDestinationURL();
		KDiffShell* shell = new KDiffShell();
		shell->show();
		shell->compare( source, destination );
	}
	delete dialog;
}

void KDiffShell::optionsShowToolbar()
{
	// this is all very cut and paste code for showing/hiding the
	// toolbar
	if (m_toolbarAction->isChecked())
		toolBar()->show();
	else
		toolBar()->hide();
}

void KDiffShell::optionsShowStatusbar()
{
	// this is all very cut and paste code for showing/hiding the
	// statusbar
	if (m_statusbarAction->isChecked())
		statusBar()->show();
	else
		statusBar()->hide();
}

void KDiffShell::optionsConfigureKeys()
{
	KKeyDialog::configureKeys(actionCollection(), "kdiff_shell.rc");
}

void KDiffShell::optionsConfigureToolbars()
{
	// use the standard toolbar editor
	KEditToolbar dlg(actionCollection());
	if (dlg.exec())
	{
		// recreate our GUI
		createGUI(m_part);
	}
}

#include "kdiff_shell.moc"
