/***************************************************************************
                                kompare_shell.cpp  -  description
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

#include <klocale.h>
#include <kiconloader.h>
#include <kstatusbar.h>
#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <kstdaction.h>
#include <ksqueezedtextlabel.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kuserprofile.h>
#include <kdebug.h>
#include <ktrader.h>
#include <kparts/componentfactory.h>

#include "kompare_part.h"
#include "komparenavtreepart.h"
#include "kompareurldialog.h"

#include "kompare_shell.h"

#define ID_N_OF_N_DIFFERENCES      1
#define ID_N_OF_N_FILES            2
#define ID_GENERAL                 3

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

	KTrader::OfferList offers = KTrader::self()->query( "text/x-diff",
	    "Kompare/ViewPart", QString::null, QString::null );
#ifdef NDEBUG
	for( int i = 0; i < offers.count(); i++ )
	{
		kdDebug(8102) << "One kservicetype checked..." << endl;
		KService::Ptr ptr2 = *(offers.at( i ));
		QStringList list = ptr2->serviceTypes();
		for ( QStringList::Iterator it2 = list.begin(); it2 != list.end(); ++it2 )
			kdDebug(8102) << *it2 << endl;
	}
#endif
	if ( offers.count() == 0 )
	{
		KMessageBox::error(this, i18n( "Could not find our KompareViewPart." ) );
		exit(1);
	}

	KService::Ptr ptr = offers.first();

	KLibFactory *mainViewFactory = KLibLoader::self()->factory( ptr->library().ascii() );
	if (mainViewFactory)
	{
		m_mainViewDock = createDockWidget( "View", kapp->icon() );
		// now that the Part is loaded, we cast it to a KomparePart to get
		// our hands on it
		m_viewPart = static_cast<KomparePart*>(mainViewFactory->create(m_mainViewDock,
		              "kompare_part", "KParts::ReadWritePart" ));

		if ( m_viewPart )
		{
			m_mainViewDock->setWidget( m_viewPart->widget() );
			setView( m_mainViewDock );
			setMainDockWidget( m_mainViewDock );

			// and integrate the part's GUI with the shell's
			createGUI(m_viewPart);
		}
	}
	else
	{
		// if we couldn't load our Part, we exit since the Shell by
		// itself can't do anything useful
		KMessageBox::error(this, i18n( "Could not load our KompareViewPart." ) );
		exit(2);
	}

	offers.clear();
	offers = KTrader::self()->query( "text/x-diff", "KParts/ReadOnlyPart", "'Kompare/NavigationPart' in ServiceTypes", QString::null );
	if ( offers.count() == 0 )
	{
		KMessageBox::error(this, i18n( "Could not find our KompareNavigationPart." ) );
		exit(3);
	}

	ptr = offers.first();

	KLibFactory *navTreeFactory = KLibLoader::self()->factory( ptr->library().ascii() );
	if (navTreeFactory)
	{
		m_navTreeDock = createDockWidget( "Navigation", kapp->icon() );

		m_navTreePart = static_cast<KompareNavTreePart*>(navTreeFactory->create(m_navTreeDock,
		                 "komparenavtreepart", "KParts::ReadOnlyPart" ));

		if ( m_navTreePart )
		{
			m_navTreeDock->setWidget( m_navTreePart->widget() );
			m_navTreeDock->manualDock( m_mainViewDock, KDockWidget::DockTop, 20 );
		}
	}
	else
	{
		// if we couldn't load our Part, we exit since the Shell by
		// itself can't do anything useful
		KMessageBox::error(this, i18n( "Could not load our KompareNavigationPart." ) );
		exit(4);
	}

	// Hook up the inter part communication
	connect( m_viewPart, SIGNAL( modelsChanged(const QPtrList<Diff2::DiffModel>*) ),
	         m_navTreePart, SLOT( slotModelsChanged( const QPtrList<Diff2::DiffModel>*) ) );

	connect( m_viewPart, SIGNAL( kompareInfo(Kompare::Info*) ),
	         m_navTreePart, SLOT( slotKompareInfo(Kompare::Info*) ) );

	connect( m_navTreePart, SIGNAL( selectionChanged(const Diff2::DiffModel*, const Diff2::Difference*) ),
	         m_viewPart, SIGNAL( selectionChanged(const Diff2::DiffModel*, const Diff2::Difference*) ) );
	connect( m_viewPart, SIGNAL( setSelection(const Diff2::DiffModel*, const Diff2::Difference*) ),
	         m_navTreePart, SLOT( slotSetSelection(const Diff2::DiffModel*, const Diff2::Difference*) ) );

	connect( m_navTreePart, SIGNAL( selectionChanged(const Diff2::Difference*) ),
	         m_viewPart, SIGNAL( selectionChanged(const Diff2::Difference*) ) );
	connect( m_viewPart, SIGNAL( setSelection(const Diff2::Difference*) ),
	         m_navTreePart, SLOT( slotSetSelection(const Diff2::Difference*) ) );

	// This is the interpart interface, it is signal and slot based so no "real" nterface here
	// All you have to do is connect the parts from your application.
	// These just point to the method with the same name in the KompareModelList or get called
	// from the method with the same name in KompareModelList.

	// There is currently no applying possible from the navtreepart to the viewpart
	connect( m_viewPart, SIGNAL(applyDifference(bool)),
	         m_navTreePart, SLOT(slotApplyDifference(bool)) );
	connect( m_viewPart, SIGNAL(applyAllDifferences(bool)),
	         m_navTreePart, SLOT(slotApplyAllDifferences(bool)) );
	connect( m_viewPart, SIGNAL(applyDifference(const Diff2::Difference*, bool)),
	         m_navTreePart, SLOT(slotApplyDifference(const Diff2::Difference*, bool)) );

	// Hook up the KomparePart -> KompareShell communication
	connect( m_viewPart, SIGNAL( setStatusBarModelInfo( int, int, int, int, int ) ),
	         this, SLOT( slotUpdateStatusBar( int, int, int, int, int ) ) );
	connect( m_viewPart, SIGNAL( setStatusBarText(const QString&) ),
	         this, SLOT( slotSetStatusBarText(const QString&) ) );

	// Read basic main-view settings, and set to autosave
	setAutoSaveSettings( "General Options" );
}

KompareShell::~KompareShell()
{
}

bool KompareShell::queryClose()
{
	return m_viewPart->queryClose();
}

void KompareShell::openDiff(const KURL& url)
{
	kdDebug(8102) << "Url = " << url.prettyURL() << endl;
	m_diffURL = url;
	m_viewPart->openDiff( url );
}

void KompareShell::openStdin()
{
	kdDebug(8102) << "Using stdin to read the diff" << endl;
	QFile file;
	file.open( IO_ReadOnly, stdin );
	QTextStream stream( &file );
	QStringList diff;

	while (!stream.eof()) {
		diff.append( stream.readLine() );
	}

	m_viewPart->openDiff( diff );

}

void KompareShell::compare(const KURL& source,const KURL& destination )
{
	m_sourceURL = source;
	m_destinationURL = destination;

	m_viewPart->compare( source, destination );
}

void KompareShell::blend( const KURL& url1, const KURL& diff )
{
	m_sourceURL = url1;
	m_destinationURL = diff;

	m_viewPart->openDirAndDiff( url1, diff );
}

void KompareShell::setupActions()
{
	KAction* open = KStdAction::open(this, SLOT(slotFileOpen()), actionCollection());
	open->setText( i18n( "&Open Diff..." ) );
	new KAction( i18n("&Compare Files..."), "fileopen", Qt::CTRL + Qt::Key_C,
	              this, SLOT(slotFileCompareFiles()),
	              actionCollection(), "file_compare_files" );
	new KAction( i18n("&Blend URL with Diff..."), "fileblend", Qt::CTRL + Qt::Key_B,
	              this, SLOT(slotFileBlendURLAndDiff()),
	              actionCollection(), "file_blend_url" );
	KStdAction::quit( this, SLOT( slotFileClose() ), actionCollection() );

#if KDE_VERSION >= KDE_MAKE_VERSION(3,1,90)
	createStandardStatusBarAction();
#endif
	setStandardToolBarMenuEnabled(true);
	m_showTextView = new KToggleAction( i18n("Show T&ext View"), 0, this, SLOT(slotShowTextView()),
	                                  actionCollection(), "options_show_text_view" );

	KStdAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
	KStdAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());
}

void KompareShell::setupStatusBar()
{
	// Made these entries permanent so they will appear on the right side
	statusBar()->insertItem( i18n(" 0 of 0 differences "), ID_N_OF_N_DIFFERENCES, 0, true );
	statusBar()->insertItem( i18n(" 0 of 0 files "), ID_N_OF_N_FILES, 0, true );

	m_generalLabel = new KSqueezedTextLabel( "", 0, "general_statusbar_label" );
	statusBar()->addWidget( m_generalLabel, 1, false );
	m_generalLabel->setAlignment( Qt::AlignLeft );
}

void KompareShell::slotUpdateStatusBar( int modelIndex, int differenceIndex, int modelCount, int differenceCount, int appliedCount )
{
	kdDebug(8102) << "KompareShell::updateStatusBar()" << endl;

	QString fileStr;
	QString diffStr;

	if ( modelIndex >= 0 )
		fileStr = i18n( " %1 of %n file ", " %1 of %n files ", modelCount ).arg( modelIndex + 1 );
	else
		fileStr = i18n( " %n file ", " %n files ", modelCount );

	if ( differenceIndex >= 0 )
		diffStr = i18n( " %1 of %n difference, %2 applied ", " %1 of %n differences, %2 applied ", differenceCount )
		          .arg( differenceIndex + 1 ).arg( appliedCount );
	else
		diffStr = i18n( " %n difference ", " %n differences ", differenceCount );

	statusBar()->changeItem( fileStr, ID_N_OF_N_FILES );
	statusBar()->changeItem( diffStr, ID_N_OF_N_DIFFERENCES );
}

void KompareShell::slotSetStatusBarText( const QString& text )
{
	m_generalLabel->setText( text );
}

void KompareShell::setCaption( const QString& caption )
{
	kdDebug() << kdBacktrace();
	KParts::DockMainWindow::setCaption( caption, m_viewPart->isModified() );
}

void KompareShell::saveProperties(KConfig* config)
{
	// The 'config' object points to the session managed
	// config file.  Anything you write here will be available
	// later when this app is restored
	if ( m_mode == Kompare::ComparingFiles )
	{
		config->writeEntry( "Mode", "ComparingFiles" );
		config->writePathEntry( "SourceUrl", m_sourceURL.url() );
		config->writePathEntry( "DestinationUrl", m_destinationURL.url() );
	}
	else if ( m_mode == Kompare::ShowingDiff )
	{
		config->writeEntry( "Mode", "ShowingDiff" );
		config->writePathEntry( "DiffUrl", m_diffURL.url() );
	}

	m_viewPart->saveProperties( config );
}

void KompareShell::readProperties(KConfig* config)
{
	// The 'config' object points to the session managed
	// config file. This function is automatically called whenever
	// the app is being restored. Read in here whatever you wrote
	// in 'saveProperties'

	QString mode = config->readEntry( "Mode", "ComparingFiles" );
	if ( mode == "ComparingFiles" )
	{
		m_mode  = Kompare::ComparingFiles;
		m_sourceURL  = config->readPathEntry( "SourceUrl", "" );
		m_destinationURL = config->readPathEntry( "DestinationFile", "" );

		m_viewPart->readProperties( config );

		m_viewPart->compareFiles( m_sourceURL, m_destinationURL );
	}
	else if ( mode == "ShowingDiff" )
	{
		m_mode = Kompare::ShowingDiff;
		m_diffURL = config->readPathEntry( "DiffUrl", "" );

		m_viewPart->readProperties( config );

		m_viewPart->openURL( m_diffURL );
	}
	else
	{ // just in case something weird has happened, dont restore the diff then
	  // Bruggie: or when some idiot like me changes the possible values for mode
	  // IOW, a nice candidate for a kconf_update thingy :)
		m_viewPart->readProperties( config );
	}
}

void KompareShell::slotFileOpen()
{
	KURL url = KFileDialog::getOpenURL( QString::null, "text/x-diff", this );
	if( !url.isEmpty() ) {
		KompareShell* shell = new KompareShell();
		kapp->ref();
		shell->show();
		shell->openDiff( url );
	}
}

void KompareShell::slotFileBlendURLAndDiff()
{
	KompareURLDialog* dialog = new KompareURLDialog( &m_sourceURL, &m_destinationURL, this );

	dialog->setCaption( i18n( "Blend File/Folder With diff Output" ) );
	dialog->setFirstGroupBoxTitle( i18n( "File/Folder" ) );
	dialog->setSecondGroupBoxTitle( i18n( "Diff Output" ) );

	KGuiItem blendGuiItem( i18n( "Blend" ), QString::null, i18n( "Blend this file or folder with the diff output" ), i18n( "If you have entered a file or folder name and a file that contains diff output in the fields in this dialog then this button will be enabled and pressing it will open kompare's main view where the output of the entered file or files from the folder are mixed with the diff output so you can then apply the difference(s) to a file or to the files. " ) );
	dialog->setButtonOK( blendGuiItem );

	dialog->setGroup( "Recent Blend Files" );

	dialog->setFirstURLRequesterMode( KFile::File|KFile::Directory|KFile::ExistingOnly );
	// diff output can not be a directory
	dialog->setSecondURLRequesterMode( KFile::File|KFile::ExistingOnly );
	if ( dialog->exec() == QDialog::Accepted )
	{
		m_sourceURL = dialog->getFirstURL();
		m_destinationURL = dialog->getSecondURL();
		KompareShell* shell = new KompareShell();
		kapp->ref();
		shell->show();
		shell->blend( m_sourceURL, m_destinationURL );
	}
	delete dialog;
}

void KompareShell::slotFileCompareFiles()
{
	KompareURLDialog* dialog = new KompareURLDialog( &m_sourceURL, &m_destinationURL, this );

	dialog->setCaption( i18n( "Compare Files or Folders" ) );
	dialog->setFirstGroupBoxTitle( i18n( "Source" ) );
	dialog->setSecondGroupBoxTitle( i18n( "Destination" ) );

	KGuiItem compareGuiItem( i18n( "Compare" ), QString::null, i18n( "Compare these files or folders" ), i18n( "If you have entered 2 filenames or 2 folders in the fields in this dialog then this button will be enabled and pressing it will start a comparison of the entered files or folders. " ) );
	dialog->setButtonOK( compareGuiItem );

	dialog->setGroup( "Recent Compare Files" );

	dialog->setFirstURLRequesterMode( KFile::File|KFile::Directory|KFile::ExistingOnly );
	dialog->setSecondURLRequesterMode( KFile::File|KFile::Directory|KFile::ExistingOnly );

	if ( dialog->exec() == QDialog::Accepted )
	{
		m_sourceURL = dialog->getFirstURL();
		m_destinationURL = dialog->getSecondURL();
		KompareShell* shell = new KompareShell();
		kapp->ref();
		shell->show();
		shell->compare( m_sourceURL, m_destinationURL );
	}
	delete dialog;
}

void KompareShell::slotFileClose()
{
	if ( m_viewPart->queryClose() )
	{
		delete this;
		kapp->deref();
	}
}

void KompareShell::slotShowTextView()
{
	if( !m_textViewWidget ) {

		KService::Ptr ptr = KServiceTypeProfile::preferredService( "text/plain", "KParts/ReadOnlyPart" );
		if ( ptr == 0 || !ptr->isValid() )
			return;

		m_textViewWidget = createDockWidget( i18n("Text View"), ptr->pixmap( KIcon::Small ) );
		m_textViewPart = KParts::ComponentFactory::createInstanceFromService<KParts::ReadOnlyPart>(
				 ptr, m_textViewWidget );
		if( m_textViewPart ) {
			m_textViewWidget->setWidget( m_textViewPart->widget() );
			m_textViewPart->openURL( m_viewPart->url() );
			connect( m_viewPart, SIGNAL( diffURLChanged() ), SLOT( slotDiffURLChanged() ) );
		}
	}

	m_textViewWidget->manualDock( m_mainViewDock, KDockWidget:: DockCenter );
}

void KompareShell::slotDiffURLChanged()
{
	if( m_textViewPart ) {
		m_textViewPart->openURL( m_viewPart->url() );
	}
}

void KompareShell::optionsConfigureKeys()
{
	KKeyDialog dlg( true, this );

	dlg.insert( actionCollection() );
	if ( m_viewPart )
	  dlg.insert( m_viewPart->actionCollection() );

	dlg.configure();
}

void KompareShell::optionsConfigureToolbars()
{
	saveMainWindowSettings( KGlobal::config(), autoSaveGroup() );
	// use the standard toolbar editor
	KEditToolbar dlg(factory());
	connect(&dlg,SIGNAL(newToolbarConfig()),this,SLOT(newToolbarConfig()));
	dlg.exec();
}

void KompareShell::newToolbarConfig()
{
	applyMainWindowSettings( KGlobal::config(), autoSaveGroup() );
}

#include "kompare_shell.moc"
