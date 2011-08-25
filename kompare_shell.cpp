/***************************************************************************
                                kompare_shell.cpp
                                -----------------
        begin                   : Sun Mar 4 2001
        Copyright 2001-2004,2009 Otto Bruggeman <bruggie@gmail.com>
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
#include "kompare_shell.h"

#include <QtCore/QTextStream>
#include <QtGui/QDockWidget>

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <kdebug.h>
#include <kedittoolbar.h>
#include <kencodingfiledialog.h>
#include <kiconloader.h>
#include <kshortcutsdialog.h>
#include <klibloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kparts/componentfactory.h>
#include <ksqueezedtextlabel.h>
#include <kstatusbar.h>
#include <kstandardaction.h>
#include <kmimetypetrader.h>
#include <kservicetypetrader.h>
#include <ktoggleaction.h>
// #include <kuserprofile.h>
#include <kactioncollection.h>

#include "kompareinterface.h"
#include "kompareurldialog.h"

#define ID_N_OF_N_DIFFERENCES      1
#define ID_N_OF_N_FILES            2
#define ID_GENERAL                 3

KompareShell::KompareShell()
	: KParts::MainWindow( ),
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

	m_viewPart = KMimeTypeTrader::createInstanceFromQuery<KParts::ReadWritePart>("text/x-patch", "Kompare/ViewPart", this);

	if ( m_viewPart )
	{
		setCentralWidget( m_viewPart->widget() );
		// and integrate the part's GUI with the shell's
		createGUI(m_viewPart);
	}
	else
	{
		// if we couldn't load our Part, we exit since the Shell by
		// itself can't do anything useful
		KMessageBox::error(this, i18n( "Could not load our KompareViewPart." ) );
		exit(2);
	}

	m_navTreeDock = new QDockWidget( i18n( "Navigation" ), this );
	m_navTreeDock->setObjectName( "Navigation" );

	// This part is implemented in KompareNavTreePart
	m_navTreePart = KServiceTypeTrader::createInstanceFromQuery<KParts::ReadOnlyPart>
		("KParts/ReadOnlyPart", "'Kompare/NavigationPart' in ServiceTypes", m_navTreeDock);

	if ( m_navTreePart )
	{
		m_navTreeDock->setWidget( m_navTreePart->widget() );
		addDockWidget( Qt::TopDockWidgetArea, m_navTreeDock );
// 			m_navTreeDock->manualDock( m_mainViewDock, KDockWidget::DockTop, 20 );
	}
	else
	{
		// if we couldn't load our Part, we exit since the Shell by
		// itself can't do anything useful
		KMessageBox::error(this, i18n( "Could not load our KompareNavigationPart." ) );
		exit(4);
	}

	// Hook up the inter part communication
	connect( m_viewPart, SIGNAL( modelsChanged(const Diff2::DiffModelList*) ),
	         m_navTreePart, SLOT( slotModelsChanged( const Diff2::DiffModelList*) ) );

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

	connect( m_viewPart, SIGNAL(diffString(const QString&)),
	         this, SLOT(slotSetDiffString(const QString&)) );

	// Read basic main-view settings, and set to autosave
	setAutoSaveSettings( "General Options" );
}

KompareShell::~KompareShell()
{
}

bool KompareShell::queryClose()
{
	bool rv = m_viewPart->queryClose();
	if ( rv )
		KGlobal::deref();
	return rv;
}

void KompareShell::openDiff(const KUrl& url)
{
	kDebug(8102) << "Url = " << url.prettyUrl() << endl;
	m_diffURL = url;
	viewPart()->openDiff( url );
}

void KompareShell::openStdin()
{
	kDebug(8102) << "Using stdin to read the diff" << endl;
	QFile file;
	file.open( QIODevice::ReadOnly, stdin );
	QTextStream stream( &file );

	QString diff = stream.read();

	file.close();

	viewPart()->openDiff( diff );

}

void KompareShell::compare(const KUrl& source,const KUrl& destination )
{
	m_sourceURL = source;
	m_destinationURL = destination;

	viewPart()->compare( source, destination );
}

void KompareShell::blend( const KUrl& url1, const KUrl& diff )
{
	m_sourceURL = url1;
	m_destinationURL = diff;

	viewPart()->openDirAndDiff( url1, diff );
}

void KompareShell::setupActions()
{
	KAction *a;
	a = actionCollection()->addAction(KStandardAction::Open, this, SLOT(slotFileOpen()));
	a->setText( i18n( "&Open Diff..." ) );
	a = actionCollection()->addAction("file_compare_files", this, SLOT(slotFileCompareFiles()));
	a->setIcon(KIcon("document-open"));
	a->setText(i18n("&Compare Files..."));
	a->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_C));
	a = actionCollection()->addAction("file_blend_url", this, SLOT(slotFileBlendURLAndDiff()));
	a->setText(i18n("&Blend URL with Diff..."));
	a->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_B));
	actionCollection()->addAction(KStandardAction::Quit, this, SLOT( slotFileClose() ));

	createStandardStatusBarAction();
	setStandardToolBarMenuEnabled(true);
	m_showTextView = new KToggleAction(i18n("Show T&ext View"), this);
// needs a KGuiItem, also the doc says explicitly not to do this
//	m_showTextView->setCheckedState(i18n("Hide T&ext View"));
	actionCollection()->addAction("options_show_text_view", m_showTextView);
	connect(m_showTextView, SIGNAL(triggered(bool)), SLOT(slotShowTextView()));

	KStandardAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
	KStandardAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());
}

void KompareShell::setupStatusBar()
{
	// Made these entries permanent so they will appear on the right side
	statusBar()->insertPermanentItem( i18n(" 0 of 0 differences "), ID_N_OF_N_DIFFERENCES, 0);
	statusBar()->insertPermanentItem( i18n(" 0 of 0 files "), ID_N_OF_N_FILES, 0);

	m_generalLabel = new KSqueezedTextLabel( "", 0 );
	statusBar()->addWidget( m_generalLabel, 1, false );
	m_generalLabel->setAlignment( Qt::AlignLeft );
}

void KompareShell::slotUpdateStatusBar( int modelIndex, int differenceIndex, int modelCount, int differenceCount, int appliedCount )
{
	kDebug(8102) << "KompareShell::updateStatusBar()" << endl;

	QString fileStr;
	QString diffStr;

	if ( modelIndex >= 0 )
		fileStr = i18np( " %2 of %1 file ", " %2 of %1 files ", modelCount, modelIndex + 1 );
	else
		fileStr = i18np( " %1 file ", " %1 files ", modelCount );

	if ( differenceIndex >= 0 )
		diffStr = i18np( " %2 of %1 difference, %3 applied ", " %2 of %1 differences, %3 applied ", differenceCount ,
		            differenceIndex + 1, appliedCount );
	else
		diffStr = i18np( " %1 difference ", " %1 differences ", differenceCount );

	statusBar()->changeItem( fileStr, ID_N_OF_N_FILES );
	statusBar()->changeItem( diffStr, ID_N_OF_N_DIFFERENCES );
}

void KompareShell::slotSetStatusBarText( const QString& text )
{
	m_generalLabel->setText( text );
}

void KompareShell::setCaption( const QString& caption )
{
//	kDebug(8102) << kBacktrace();
	KParts::MainWindow::setCaption( caption, m_viewPart->isModified() );
}

void KompareShell::saveProperties(KConfigGroup &config)
{
	// The 'config' object points to the session managed
	// config file.  Anything you write here will be available
	// later when this app is restored
	if ( m_mode == Kompare::ComparingFiles )
	{
		config.writeEntry( "Mode", "ComparingFiles" );
		config.writePathEntry( "SourceUrl", m_sourceURL.url() );
		config.writePathEntry( "DestinationUrl", m_destinationURL.url() );
	}
	else if ( m_mode == Kompare::ShowingDiff )
	{
		config.writeEntry( "Mode", "ShowingDiff" );
		config.writePathEntry( "DiffUrl", m_diffURL.url() );
	}

	viewPart()->saveProperties( config.config() );
}

void KompareShell::readProperties(const KConfigGroup &config)
{
	// The 'config' object points to the session managed
	// config file. This function is automatically called whenever
	// the app is being restored. Read in here whatever you wrote
	// in 'saveProperties'

	QString mode = config.readEntry( "Mode", "ComparingFiles" );
	if ( mode == "ComparingFiles" )
	{
		m_mode  = Kompare::ComparingFiles;
		m_sourceURL  = config.readPathEntry( "SourceUrl", "" );
		m_destinationURL = config.readPathEntry( "DestinationFile", "" );

		viewPart()->readProperties( const_cast<KConfig *>(config.config()) );

		viewPart()->compareFiles( m_sourceURL, m_destinationURL );
	}
	else if ( mode == "ShowingDiff" )
	{
		m_mode = Kompare::ShowingDiff;
		m_diffURL = config.readPathEntry( "DiffUrl", "" );

		viewPart()->readProperties( const_cast<KConfig *>(config.config()) );

		m_viewPart->openUrl( m_diffURL );
	}
	else
	{ // just in case something weird has happened, don't restore the diff then
	  // Bruggie: or when some idiot like me changes the possible values for mode
	  // IOW, a nice candidate for a kconf_update thingy :)
		viewPart()->readProperties( const_cast<KConfig *>(config.config()) );
	}
}

void KompareShell::slotFileOpen()
{
	// FIXME: use different filedialog which gets encoding
	KUrl url = KFileDialog::getOpenUrl( KUrl(), "text/x-patch", this );
	if( !url.isEmpty() ) {
		KompareShell* shell = new KompareShell();
		KGlobal::ref();
		shell->show();
		shell->openDiff( url );
	}
}

void KompareShell::slotFileBlendURLAndDiff()
{
	KompareURLDialog dialog( this );

	dialog.setCaption( i18n( "Blend File/Folder with diff Output" ) );
	dialog.setFirstGroupBoxTitle( i18n( "File/Folder" ) );
	dialog.setSecondGroupBoxTitle( i18n( "Diff Output" ) );

	KGuiItem blendGuiItem( i18n( "Blend" ), QString(), i18n( "Blend this file or folder with the diff output" ), i18n( "If you have entered a file or folder name and a file that contains diff output in the fields in this dialog then this button will be enabled and pressing it will open kompare's main view where the output of the entered file or files from the folder are mixed with the diff output so you can then apply the difference(s) to a file or to the files. " ) );
	dialog.setButtonGuiItem( KDialog::Ok, blendGuiItem );

	dialog.setGroup( "Recent Blend Files" );

	dialog.setFirstURLRequesterMode( KFile::File|KFile::Directory|KFile::ExistingOnly );
	// diff output can not be a directory
	dialog.setSecondURLRequesterMode( KFile::File|KFile::ExistingOnly );
	if ( dialog.exec() == QDialog::Accepted )
	{
		m_sourceURL = dialog.getFirstURL();
		m_destinationURL = dialog.getSecondURL();
		// Leak???
		KompareShell* shell = new KompareShell();
		KGlobal::ref();
		shell->show();
		shell->viewPart()->setEncoding( dialog.encoding() );
		shell->blend( m_sourceURL, m_destinationURL );
	}
}

void KompareShell::slotFileCompareFiles()
{
	KompareURLDialog dialog( this );

	dialog.setCaption( i18n( "Compare Files or Folders" ) );
	dialog.setFirstGroupBoxTitle( i18n( "Source" ) );
	dialog.setSecondGroupBoxTitle( i18n( "Destination" ) );

	KGuiItem compareGuiItem( i18n( "Compare" ), QString(), i18n( "Compare these files or folders" ), i18n( "If you have entered 2 filenames or 2 folders in the fields in this dialog then this button will be enabled and pressing it will start a comparison of the entered files or folders. " ) );
	dialog.setButtonGuiItem( KDialog::Ok, compareGuiItem );

	dialog.setGroup( "Recent Compare Files" );

	dialog.setFirstURLRequesterMode( KFile::File|KFile::Directory|KFile::ExistingOnly );
	dialog.setSecondURLRequesterMode( KFile::File|KFile::Directory|KFile::ExistingOnly );

	if ( dialog.exec() == QDialog::Accepted )
	{
		m_sourceURL = dialog.getFirstURL();
		m_destinationURL = dialog.getSecondURL();
		KompareShell* shell = new KompareShell();
		KGlobal::ref();
		shell->show();
		kDebug(8102) << "The encoding is: " << dialog.encoding() << endl;
		shell->viewPart()->setEncoding( dialog.encoding() );
		shell->compare( m_sourceURL, m_destinationURL );
	}
}

void KompareShell::slotFileClose()
{
	if ( m_viewPart->queryClose() )
	{
		KGlobal::deref();
	}
}

void KompareShell::slotShowTextView()
{
	if ( !m_textViewWidget )
	{
		QString error;

		// FIXME: proper error checking
		m_textViewWidget = new QDockWidget( i18n( "Text View" ), this );
		m_textViewWidget->setObjectName( "Text View" );
// 		m_textViewWidget = createDockWidget( i18n("Text View"), SmallIcon( "text-x-generic") );
		m_textViewPart = KServiceTypeTrader::createInstanceFromQuery<KTextEditor::Document>(
		                 QString::fromLatin1("KTextEditor/Document"),
		                 this, this, QString(), QVariantList(), &error );
		if ( m_textViewPart )
		{
			m_textView = qobject_cast<KTextEditor::View*>( m_textViewPart->createView( this ) );
			m_textViewWidget->setWidget( static_cast<QWidget*>(m_textView) );
	 		m_textViewPart->setHighlightingMode( "Diff" );
	 		m_textViewPart->setText( m_diffString );
		}
		m_textViewWidget->show();
		connect( m_textViewWidget, SIGNAL(visibilityChanged(bool)), SLOT(slotVisibilityChanged(bool)) );
	}
	else if ( m_textViewWidget->isVisible() )
		m_textViewWidget->hide();
	else
		m_textViewWidget->show();

	addDockWidget( Qt::BottomDockWidgetArea, m_textViewWidget );
// 	m_textViewWidget->manualDock( m_mainViewDock, KDockWidget:: DockCenter );
}

void KompareShell::slotVisibilityChanged( bool visible )
{
	m_showTextView->setChecked( visible );
}

void KompareShell::slotSetDiffString( const QString& diffString )
{
 	if ( m_textViewPart )
 		m_textViewPart->setText( diffString );
	m_diffString = diffString;
}

void KompareShell::optionsConfigureKeys()
{
	KShortcutsDialog dlg( KShortcutsEditor::AllActions, KShortcutsEditor::LetterShortcutsAllowed, this );

	dlg.addCollection( actionCollection() );
	if ( m_viewPart )
		dlg.addCollection( m_viewPart->actionCollection() );

	dlg.configure();
}

void KompareShell::optionsConfigureToolbars()
{
	KConfigGroup group(KGlobal::config(), autoSaveGroup());
	saveMainWindowSettings(group);
	// use the standard toolbar editor
	KEditToolBar dlg(factory());
	connect(&dlg,SIGNAL(newToolbarConfig()),this,SLOT(newToolbarConfig()));
	dlg.exec();
}

void KompareShell::newToolbarConfig()
{
	KConfigGroup group(KGlobal::config(), autoSaveGroup());
	applyMainWindowSettings(group);
}

KompareInterface* KompareShell::viewPart() const
{
	return qobject_cast<KompareInterface *>(m_viewPart);
}

#include "kompare_shell.moc"
