/***************************************************************************
                                kompare_shell.cpp  -  description
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
* kompareshell.cpp
*
* Copyright (C) 2001  <kurt@granroth.org>
*/

#include "kompare_shell.h"

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
#include <ktrader.h>
#include <kdebug.h>

#include "kompare_part.h"
#include "kcomparedialog.h"

#define ID_N_OF_N_DIFFERENCES      1
#define ID_N_OF_N_FILES            2
#define ID_GENERAL                 3

KompareShell::KompareShell()
	: KParts::DockMainWindow( 0L, "KompareShell" )
	, m_textViewWidget( 0 )
	, m_textViewPart( 0 )
{
	if ( !initialGeometrySet() )
	resize( 800, 480 );

	// set the shell's ui resource file
	setXMLFile("kompareui.rc");

	// then, setup our actions
	setupActions();
	setupStatusBar();

	// and a status bar
	statusBar()->show();

	// this routine will find and load our Part.  it finds the Part by
	// name which is a bad idea usually.. but it's alright in this
	// case since our Part is made for this Shell
	KLibFactory *factory = KLibLoader::self()->factory("libkomparepart");
	if (factory)
	{
		KDockWidget* mainDock = createDockWidget( "Kompare", kapp->icon() );

		// now that the Part is loaded, we cast it to a Part to get
		// our hands on it
		m_part = static_cast<KomparePart *>(factory->create(mainDock,
		              "kompare_part", "KParts::ReadWritePart" ));

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
			connect( m_part, SIGNAL(appliedChanged()), this, SLOT(updateStatusBar()));

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

KompareShell::~KompareShell()
{
}

bool KompareShell::queryClose()
{
	return m_part->askSaveChanges();
}

void KompareShell::load(const KURL& url)
{
	m_part->openURL( url );
}

void KompareShell::compare(const KURL& source,const KURL& destination )
{
	m_source = source;
	m_destination = destination;
	m_part->compare( source, destination );
}

void KompareShell::setupActions()
{
	new KAction( i18n("&Compare Files..."), "fileopen", Qt::CTRL + Qt::Key_O,
	              this, SLOT(slotFileCompareFiles()),
	              actionCollection(), "file_compare_files" );
	KStdAction::open(this, SLOT(slotFileOpen()), actionCollection());
	KStdAction::quit(kapp, SLOT(quit()), actionCollection());

	m_toolbarAction = KStdAction::showToolbar(this, SLOT(optionsShowToolbar()), actionCollection());
	m_statusbarAction = KStdAction::showStatusbar(this, SLOT(optionsShowStatusbar()), actionCollection());
	m_showTextView = new KToggleAction( i18n("Show &Text View"), 0, this, SLOT(slotShowTextView()),
	                                  actionCollection(), "options_show_text_view" );
	
	KStdAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
	KStdAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());
}

void KompareShell::setupStatusBar()
{
	// Made these entries permanent so they will appear on the right side
	statusBar()->insertItem( i18n(" 0 of 0 differences "), ID_N_OF_N_DIFFERENCES, 0, true );
	statusBar()->insertItem( i18n(" 0 of 0 files "), ID_N_OF_N_FILES, 0, true );
	statusBar()->insertItem( "", ID_GENERAL, 1 );
	statusBar()->setItemAlignment( ID_GENERAL, AlignLeft );
}

void KompareShell::updateStatusBar()
{
	QString fileStr;
	QString diffStr;
	int modelIndex = m_part->getSelectedModelIndex();
	int modelCount = m_part->modelCount();
	if (modelIndex >= 0) {
		fileStr = i18n( " %1 of %2 file ", " %1 of %2 files ", modelCount )
		          .arg(modelIndex+1).arg(modelCount);
		int diffIndex = m_part->getSelectedDifferenceIndex();
		int diffCount = m_part->getSelectedModel()->differenceCount();
		int appliedCount = m_part->appliedCount();
		if (diffIndex >= 0)
			diffStr = i18n(" %1 of %2 difference, %3 applied ", " %1 of %2 differences, %3 applied ", diffCount )
			          .arg(diffIndex+1).arg(diffCount).arg(appliedCount);
		else
			diffStr = i18n(" %1 difference ", " %1 differences ", diffCount ).arg(diffCount);
	} else {
		fileStr = i18n( " %1 file ", " %1 files ", modelCount ).arg( modelCount );
	}
	statusBar()->changeItem( fileStr, ID_N_OF_N_FILES );
	statusBar()->changeItem( diffStr, ID_N_OF_N_DIFFERENCES );
}

void KompareShell::slotSetStatusBarText( const QString& text )
{
	statusBar()->changeItem( text, ID_GENERAL );
}

void KompareShell::setCaption( const QString& caption )
{
	KParts::DockMainWindow::setCaption( caption, m_part->isModified() );
}

void KompareShell::saveProperties(KConfig* /*config*/)
{
	// the 'config' object points to the session managed
	// config file.  anything you write here will be available
	// later when this app is restored
}

void KompareShell::readProperties(KConfig* /*config*/)
{
	// the 'config' object points to the session managed
	// config file.  this function is automatically called whenever
	// the app is being restored.  read in here whatever you wrote
	// in 'saveProperties'
}

void KompareShell::slotFileOpen()
{
	KURL url = KFileDialog::getOpenURL( QString::null, "text/x-diff", this );
	if( !url.isEmpty() ) {
		KompareShell* shell = new KompareShell();
		shell->show();
		shell->load( url );
	}
}

void KompareShell::slotFileCompareFiles()
{
	KCompareDialog* dialog = new KCompareDialog( &m_source, &m_destination, this );
	if( dialog->exec() == QDialog::Accepted ) {
		KURL source = dialog->getSourceURL();
		KURL destination = dialog->getDestinationURL();
		KompareShell* shell = new KompareShell();
		shell->show();
		shell->compare( source, destination );
	}
	delete dialog;
}

void KompareShell::optionsShowToolbar()
{
	// this is all very cut and paste code for showing/hiding the
	// toolbar
	if (m_toolbarAction->isChecked())
		toolBar()->show();
	else
		toolBar()->hide();
}

void KompareShell::optionsShowStatusbar()
{
	// this is all very cut and paste code for showing/hiding the
	// statusbar
	if (m_statusbarAction->isChecked())
		statusBar()->show();
	else
		statusBar()->hide();
}

void KompareShell::slotShowTextView()
{
	if( !m_textViewWidget ) {
		
		KTrader::OfferList offers = KTrader::self()->query( "text/plain",
		    "KParts/ReadOnlyPart", QString::null, QString::null );
		KService::Ptr ptr = offers.first();
		KLibFactory* factory = KLibLoader::self()->factory( ptr->library().ascii() );
		
		m_textViewWidget = createDockWidget( i18n("Text View"), ptr->pixmap( KIcon::Small ) );
		
		if( factory )
			m_textViewPart = static_cast<KParts::ReadOnlyPart *>(
			    factory->create(m_textViewWidget, ptr->name().ascii(), "KParts::ReadOnlyPart"));
		
		if( m_textViewPart ) {
			m_textViewWidget->setWidget( m_textViewPart->widget() );
			m_textViewPart->openURL( m_part->diffURL() );
			connect( m_part, SIGNAL( diffURLChanged() ), SLOT( slotDiffURLChanged() ) );
		}
	}
	
	m_textViewWidget->manualDock( getMainDockWidget(), KDockWidget:: DockCenter );
}

void KompareShell::slotDiffURLChanged()
{
	if( m_textViewPart ) {
		m_textViewPart->openURL( m_part->diffURL() );
	}
}

void KompareShell::optionsConfigureKeys()
{
	KKeyDialog::configureKeys(actionCollection(), "kompare_shell.rc");
}

void KompareShell::optionsConfigureToolbars()
{
	// use the standard toolbar editor
	KEditToolbar dlg(actionCollection());
	if (dlg.exec())
	{
		// recreate our GUI
		createGUI(m_part);
	}
}

#include "kompare_shell.moc"
