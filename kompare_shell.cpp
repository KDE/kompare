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

#include <klocale.h>
#include <kiconloader.h>
#include <kstatusbar.h>
#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <kstdaction.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <ktrader.h>
#include <kdebug.h>

#include "kompare_part.h"
#include "komparenavtreepart.h"
#include "kcomparedialog.h"

#include "kompare_shell.h"

#define ID_N_OF_N_DIFFERENCES      1
#define ID_N_OF_N_FILES            2
#define ID_GENERAL                 3

#define kdDebug() kdDebug(8100)

KompareShell::KompareShell()
	: KParts::DockMainWindow( 0L, "KompareShell" ),
	m_textViewPart( 0 ),
	m_textViewWidget( 0 )
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

	KTrader::OfferList offers = KTrader::self()->query( "text/x-diff",
	    "KompareViewPart", QString::null, QString::null );
#ifdef NDEBUG
	for( int i = 0; i < offers.count(); i++ )
	{
		kdDebug() << "one kservicetype checked..." << endl;
		KService::Ptr ptr2 = *(offers.at( i ));
		QStringList list = ptr2->serviceTypes();
		for ( QStringList::Iterator it2 = list.begin(); it2 != list.end(); ++it2 )
			kdDebug() << *it2 << endl;
	}
#endif
	KService::Ptr ptr = offers.first();
	if ( offers.count() == 0 )
	{
		KMessageBox::error(this, i18n( "Could not find our Part!" ) );
		exit(1);
	}
	KLibFactory *mainViewFactory = KLibLoader::self()->factory( ptr->library().ascii() );
	if (mainViewFactory)
	{
		m_mainViewDock = createDockWidget( "Kompare", kapp->icon() );
		// now that the Part is loaded, we cast it to a Part to get
		// our hands on it
		m_viewPart = static_cast<KomparePart*>(mainViewFactory->create(m_mainViewDock,
		              "kompare_part", "KParts::ReadWritePart" ));

		if ( m_viewPart )
		{
			m_mainViewDock->setWidget( m_viewPart->widget() );
			setView( m_mainViewDock );
			setMainDockWidget( m_mainViewDock );

			connect( m_viewPart, SIGNAL(selectionChanged(const DiffModel*,const Difference*)), 
			         this, SLOT(updateStatusBar()));
			connect( m_viewPart, SIGNAL(appliedChanged()), 
			         this, SLOT(updateStatusBar()));
			// and integrate the part's GUI with the shell's
			createGUI(m_viewPart);
		}
	}
	else
	{
		// if we couldn't find our Part, we exit since the Shell by
		// itself can't do anything useful
		KMessageBox::error(this, i18n( "Could not find our Part!" ) );
		exit(2);
	}

	offers = KTrader::self()->query( "text/x-diff",
	    "KompareNavigationPart", QString::null, QString::null );
	ptr = offers.first();
	if ( offers.count() == 0 )
	{
		KMessageBox::error(this, i18n( "Could not find our Part!" ) );
		exit(3);
	}
	KLibFactory *navTreeFactory = KLibLoader::self()->factory( ptr->library().ascii() );
	if (navTreeFactory)
	{
		m_navTreeDock = createDockWidget( "Differences", kapp->icon() );

		m_navTreePart = static_cast<KompareNavTreePart*>(navTreeFactory->create(m_navTreeDock,
		                 "komparenavtreepart", "KParts::ReadOnlyPart" ));

		if ( m_navTreePart )
		{
			m_navTreeDock->setWidget( m_navTreePart->widget() );
			m_navTreeDock->manualDock( m_mainViewDock, KDockWidget::DockTop, 20 );
			
			connect( m_viewPart->model(), SIGNAL( modelsChanged(const QPtrList<DiffModel>*) ),
			         m_navTreePart, SLOT( slotModelsChanged( const QPtrList<DiffModel>*) ) );
			// still need to connect the setSelection signals and the slotSelectionChanged slots in this part
			connect( m_navTreePart, SIGNAL( selectionChanged(const DiffModel*, const Difference*) ),
			         m_viewPart->model(), SLOT( slotSelectionChanged(const DiffModel*, const Difference*) ) );
			connect( m_viewPart->model(), SIGNAL( setSelection(const DiffModel*, const Difference*) ),
			         m_navTreePart, SLOT( slotSetSelection(const DiffModel*, const Difference*) ) );

			connect( m_navTreePart, SIGNAL( selectionChanged(const Difference*) ),
			         m_viewPart->model(), SLOT( slotSelectionChanged(const Difference*) ) );
			connect( m_viewPart->model(), SIGNAL( setSelection(const Difference*) ),
			         m_navTreePart, SLOT( slotSetSelection(const Difference*) ) );
			// connect the apply signals...
			connect( m_viewPart->model(), SIGNAL(applyDifference(bool)),
			         m_navTreePart, SLOT(slotApplyDifference(bool)) );
			connect( m_viewPart->model(), SIGNAL(applyAllDifferences(bool)),
			         m_navTreePart, SLOT(slotApplyAllDifferences(bool)) );
			connect( m_viewPart->model(), SIGNAL(applyDifference(const Difference*, bool)),
			         m_navTreePart, SLOT(slotApplyDifference(const Difference*, bool)) );
		}
	}
	else
	{
		// if we couldn't find our Part, we exit since the Shell by
		// itself can't do anything useful
		KMessageBox::error(this, i18n( "Could not find our Part!" ) );
		exit(4);
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
	return m_viewPart->askSaveChanges();
}

void KompareShell::load(const KURL& url)
{
	m_diffURL = url;
	m_viewPart->openURL( url );
}

void KompareShell::compare(const KURL& source,const KURL& destination )
{
	m_sourceURL = source;
	m_destinationURL = destination;
	m_viewPart->compare( source, destination );
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
	kdDebug() << "KompareShell::updateStatusBar()" << endl;
	
	QString fileStr;
	QString diffStr;
	int modelIndex = m_viewPart->selectedModelIndex();
	int modelCount = m_viewPart->modelCount();
	if (modelIndex >= 0) {
		fileStr = i18n( " %1 of %n file ", " %1 of %n files ", modelCount ).arg(modelIndex+1);
		int diffIndex = m_viewPart->selectedDifferenceIndex();
		int diffCount = m_viewPart->differenceCount();
		int appliedCount = m_viewPart->appliedCount();
		if (diffIndex >= 0)
			diffStr = i18n(" %1 of %n difference, %2 applied ", " %1 of %n differences, %2 applied ", diffCount )
			          .arg(diffIndex+1).arg(appliedCount);
		else
			diffStr = i18n(" %n difference ", " %n differences ", diffCount );
	} else {
		fileStr = i18n( " %n file ", " %n files ", modelCount );
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
	KParts::DockMainWindow::setCaption( caption, m_viewPart->isModified() );
}

void KompareShell::saveProperties(KConfig* config)
{
	// The 'config' object points to the session managed
	// config file.  Anything you write here will be available
	// later when this app is restored
	if ( m_mode == Kompare::Compare )
	{
		config->writeEntry( "Mode", "Compare" );
		config->writeEntry( "SourceUrl", m_sourceURL.url() );
		config->writeEntry( "DestinationUrl", m_destinationURL.url() );
	}
	else if ( m_mode == Kompare::Diff )
	{
		config->writeEntry( "Mode", "Diff" );
		config->writeEntry( "DiffUrl", m_diffURL.url() );
	}

	m_viewPart->saveSettings( config );
}

void KompareShell::readProperties(KConfig* config)
{
	// The 'config' object points to the session managed
	// config file. This function is automatically called whenever
	// the app is being restored. Read in here whatever you wrote
	// in 'saveProperties'

	QString mode = config->readEntry( "Mode", "Compare" );
	if ( mode == "Compare" )
	{
		m_mode  = Kompare::Compare;
		m_sourceURL  = config->readEntry( "SourceUrl", "" );
		m_destinationURL = config->readEntry( "DestinationFile", "" );

		m_viewPart->loadSettings( config );

		m_viewPart->compare( m_sourceURL, m_destinationURL );
	}
	else if ( mode == "Diff" )
	{
		m_mode = Kompare::Diff;
		m_diffURL = config->readEntry( "DiffUrl", "" );

		m_viewPart->loadSettings( config );

		m_viewPart->openURL( m_diffURL );
	}
	else
	{ // just in case something weird has happened, dont restore the diff then
		m_viewPart->loadSettings( config );
	}
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
	KCompareDialog* dialog = new KCompareDialog( &m_sourceURL, &m_destinationURL, this );
	if( dialog->exec() == QDialog::Accepted ) {
		KURL m_sourceURL = dialog->getSourceURL();
		KURL m_destinationURL = dialog->getDestinationURL();
		KompareShell* shell = new KompareShell();
		shell->show();
		shell->compare( m_sourceURL, m_destinationURL );
	}
	kdDebug() << "Deleting dialog" << endl;
	delete dialog;
	kdDebug() << "Dialog deleted" << endl;
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
			m_textViewPart->openURL( m_viewPart->diffURL() );
			connect( m_viewPart, SIGNAL( diffURLChanged() ), SLOT( slotDiffURLChanged() ) );
		}
	}

	m_textViewWidget->manualDock( m_mainViewDock, KDockWidget:: DockCenter );
}

void KompareShell::slotDiffURLChanged()
{
	if( m_textViewPart ) {
		m_textViewPart->openURL( m_viewPart->diffURL() );
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
		createGUI(m_viewPart);
	}
}

#include "kompare_shell.moc"
