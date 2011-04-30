/***************************************************************************
                                kompare_part.cpp
                                ----------------
        begin                   : Sun Mar 4 2001
        Copyright 2001-2005,2009 Otto Bruggeman <bruggie@gmail.com>
        Copyright 2001-2003 John Firebaugh <jfirebaugh@kde.org>
        Copyright 2004      Jeff Snyder    <jeff@caffeinated.me.uk>
        Copyright 2007-2009 Kevin Kofler   <kevin.kofler@chello.at>
****************************************************************************/

/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***************************************************************************/

#include "kompare_part.h"

#include <QtGui/QLayout>
#include <QtGui/QWidget>
#include <QPainter>
#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>

#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kcomponentdata.h>
#include <kdebug.h>
#include <kdeprintdialog.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandardaction.h>
#include <kstandarddirs.h>
#include <kstandardshortcut.h>
#include <ktemporaryfile.h>
#include <kparts/genericfactory.h>
#include <ktempdir.h>

#include <kio/netaccess.h>
#include <kglobal.h>

#include "diffmodel.h"
#include "komparelistview.h"
#include "kompareconnectwidget.h"
#include "diffsettings.h"
#include "viewsettings.h"
#include "kompareprefdlg.h"
#include "komparesaveoptionswidget.h"
#include "komparesplitter.h"

typedef KParts::GenericFactory<KomparePart> KomparePartFactory;
K_EXPORT_COMPONENT_FACTORY( libkomparepart, KomparePartFactory )

ViewSettings* KomparePart::m_viewSettings = 0L;
DiffSettings* KomparePart::m_diffSettings = 0L;

KomparePart::KomparePart( QWidget *parentWidget, QObject *parent, const QStringList & /*args*/ ) :
	KParts::ReadWritePart(parent),
	m_tempDiff( 0 ),
	m_info()
{
	// we need an instance
	setComponentData( KomparePartFactory::componentData() );

	if( !m_viewSettings ) {
		m_viewSettings = new ViewSettings( 0 );
	}
	if( !m_diffSettings ) {
		m_diffSettings = new DiffSettings( 0 );
	}

	readProperties( KGlobal::config().data() );

	m_splitter = new KompareSplitter ( m_viewSettings, parentWidget );

	// !!! Do not change this ->parentWidget()! Due to the bizarre reparenting hacks in KompareSplitter, !!!
	// !!! the ->parentWidget() is not the one passed in the constructor, but the scroll view created    !!!
	// !!! within the KompareSplitter. If you use m_splitter here, the scroll bars will not be shown!    !!!
	// !!! Using the parentWidget parameter is also wrong, of course, you have to use m_splitter->...!   !!!
	setWidget( m_splitter->parentWidget() );

	// This creates the "Model creator" and connects the signals and slots
	m_modelList = new Diff2::KompareModelList( m_diffSettings, m_splitter, this, "komparemodellist" );

	connect( m_modelList, SIGNAL(status( Kompare::Status )),
	         this, SLOT(slotSetStatus( Kompare::Status )) );
	connect( m_modelList, SIGNAL(setStatusBarModelInfo( int, int, int, int, int )),
	         this, SIGNAL(setStatusBarModelInfo( int, int, int, int, int )) );
	connect( m_modelList, SIGNAL(error( QString )),
	         this, SLOT(slotShowError( QString )) );
	connect( m_modelList, SIGNAL(applyAllDifferences( bool )),
	         this, SLOT(updateActions()) );
	connect( m_modelList, SIGNAL(applyDifference( bool )),
	         this, SLOT(updateActions()) );
	connect( m_modelList, SIGNAL(applyAllDifferences( bool )),
	         this, SIGNAL(appliedChanged()) );
	connect( m_modelList, SIGNAL(applyDifference( bool )),
	         this, SIGNAL(appliedChanged()) );
	connect( m_modelList, SIGNAL(updateActions()), this, SLOT(updateActions()) );

	// This is the stuff to connect the "interface" of the kompare part to the model inside
	connect( m_modelList, SIGNAL(modelsChanged(const Diff2::DiffModelList*)),
	         this, SIGNAL(modelsChanged(const Diff2::DiffModelList*)) );

	connect( m_modelList, SIGNAL(setSelection(const Diff2::DiffModel*, const Diff2::Difference*)),
	         this, SIGNAL(setSelection(const Diff2::DiffModel*, const Diff2::Difference*)) );
	connect( this, SIGNAL(selectionChanged(const Diff2::DiffModel*, const Diff2::Difference*)),
	         m_modelList, SLOT(slotSelectionChanged(const Diff2::DiffModel*, const Diff2::Difference*)) );

	connect( m_modelList, SIGNAL(setSelection(const Diff2::Difference*)),
	         this, SIGNAL(setSelection(const Diff2::Difference*)) );
	connect( this, SIGNAL(selectionChanged(const Diff2::Difference*)),
	         m_modelList, SLOT(slotSelectionChanged(const Diff2::Difference*)) );

	connect( m_modelList, SIGNAL(applyDifference(bool)),
	         this, SIGNAL(applyDifference(bool)) );
	connect( m_modelList, SIGNAL(applyAllDifferences(bool)),
	         this, SIGNAL(applyAllDifferences(bool)) );
	connect( m_modelList, SIGNAL(applyDifference(const Diff2::Difference*, bool)),
	         this, SIGNAL(applyDifference(const Diff2::Difference*, bool)) );
	connect( m_modelList, SIGNAL(diffString(const QString&)),
	         this, SIGNAL(diffString(const QString&)) );

	connect( this, SIGNAL(kompareInfo(Kompare::Info*)), m_modelList, SLOT(slotKompareInfo(Kompare::Info*)) );

	// Here we connect the splitter to the modellist
	connect( m_modelList, SIGNAL(setSelection(const Diff2::DiffModel*, const Diff2::Difference*)),
	         m_splitter,  SLOT(slotSetSelection(const Diff2::DiffModel*, const Diff2::Difference*)) );
//	connect( m_splitter,  SIGNAL(selectionChanged(const Diff2::Difference*, const Diff2::Difference*)),
//	         m_modelList, SLOT(slotSelectionChanged(const Diff2::Difference*, const Diff2::Difference*)) );
	connect( m_modelList, SIGNAL(setSelection(const Diff2::Difference*)),
	         m_splitter,  SLOT(slotSetSelection(const Diff2::Difference*)) );
	connect( m_splitter,  SIGNAL(selectionChanged(const Diff2::Difference*)),
	         m_modelList, SLOT(slotSelectionChanged(const Diff2::Difference*)) );

	connect( m_modelList, SIGNAL(applyDifference(bool)),
	         m_splitter, SLOT(slotApplyDifference(bool)) );
	connect( m_modelList, SIGNAL(applyAllDifferences(bool)),
	         m_splitter, SLOT(slotApplyAllDifferences(bool)) );
	connect( m_modelList, SIGNAL(applyDifference(const Diff2::Difference*, bool)),
	         m_splitter, SLOT(slotApplyDifference(const Diff2::Difference*, bool)) );
	connect( this, SIGNAL(configChanged()), m_splitter, SIGNAL(configChanged()) );

	setupActions();

	// set our XML-UI resource file
	setXMLFile( "komparepartui.rc" );

	// we are read-write by default -> uhm what if we are opened by lets say konq in RO mode ?
	// Then we should not be doing this...
	setReadWrite( true );

	// we are not modified since we haven't done anything yet
	setModified( false );
}

KomparePart::~KomparePart()
{
	// This is the only place allowed to call cleanUpTemporaryFiles
	// because before there might still be a use for them (when swapping)
	cleanUpTemporaryFiles();
}

void KomparePart::setupActions()
{
	// create our actions

	m_saveAll = actionCollection()->addAction("file_save_all", this, SLOT(saveAll()));
	m_saveAll->setIcon(KIcon("document-save-all"));
	m_saveAll->setText(i18n("Save &All"));
	m_saveDiff = actionCollection()->addAction("file_save_diff", this, SLOT(saveDiff()));
	m_saveDiff->setText(i18n("Save &Diff..."));
	m_swap = actionCollection()->addAction("file_swap", this, SLOT(slotSwap()));
	m_swap->setText(i18n("Swap Source with Destination"));
	m_diffStats = actionCollection()->addAction("file_diffstats", this, SLOT(slotShowDiffstats()));
	m_diffStats->setText(i18n("Show Statistics"));
	m_diffRefresh = actionCollection()->addAction("file_refreshdiff", this, SLOT(slotRefreshDiff()));
	m_diffRefresh->setIcon(KIcon("view-refresh"));
	m_diffRefresh->setText(i18n("Refresh Diff"));
	m_diffRefresh->setShortcut(KStandardShortcut::reload());

	m_print        = actionCollection()->addAction(KStandardAction::Print, this, SLOT( slotFilePrint() ));
	m_printPreview = actionCollection()->addAction(KStandardAction::PrintPreview, this, SLOT( slotFilePrintPreview() ));
	KStandardAction::preferences(this, SLOT(optionsPreferences()), actionCollection());
}

void KomparePart::updateActions()
{
	m_saveAll->setEnabled     ( m_modelList->hasUnsavedChanges() );
	m_saveDiff->setEnabled    ( m_modelList->mode() == Kompare::ComparingFiles || m_modelList->mode() == Kompare::ComparingDirs );
	m_swap->setEnabled        ( m_modelList->mode() == Kompare::ComparingFiles || m_modelList->mode() == Kompare::ComparingDirs );
	m_diffRefresh->setEnabled ( m_modelList->mode() == Kompare::ComparingFiles || m_modelList->mode() == Kompare::ComparingDirs );
	m_diffStats->setEnabled   ( m_modelList->modelCount() > 0 );
	m_print->setEnabled       ( m_modelList->modelCount() > 0 ); // If modellist has models then we have something to print, it's that simple.
	m_printPreview->setEnabled( m_modelList );
}

void KomparePart::setEncoding( const QString& encoding )
{
	kDebug(8103) << "Encoding: " << encoding << endl;
	m_modelList->setEncoding( encoding );
}

bool KomparePart::openDiff( const KUrl& url )
{
	kDebug(8103) << "Url = " << url.url() << endl;

	m_info.mode = Kompare::ShowingDiff;
	m_info.source = url;
	bool result = false;
	fetchURL( url, true );

	emit kompareInfo( &m_info );

	if ( !m_info.localSource.isEmpty() )
	{
		kDebug(8103) << "Download succeeded " << endl;
		result = m_modelList->openDiff( m_info.localSource );
		updateActions();
		updateCaption();
		updateStatus();
	}
	else
	{
		kDebug(8103) << "Download failed !" << endl;
	}

	return result;
}

bool KomparePart::openDiff( const QString& diffOutput )
{
	bool value = false;

	m_info.mode = Kompare::ShowingDiff;

	emit kompareInfo( &m_info );

	if ( m_modelList->parseDiffOutput( diffOutput ) == 0 )
	{
		value = true;
		m_modelList->show();
		updateActions();
		updateCaption();
		updateStatus();
	}

	return value;
}

bool KomparePart::openDiff3( const KUrl& diff3Url )
{
	// FIXME: Implement this !!!
	kDebug(8103) << "Not implemented yet. Filename is: " << diff3Url.prettyUrl() << endl;
	return false;
}

bool KomparePart::openDiff3( const QString& diff3Output )
{
	// FIXME: Implement this !!!
	kDebug(8103) << "Not implemented yet. diff3 output is: " << endl;
	kDebug(8103) << diff3Output << endl;
	return false;
}

bool KomparePart::exists( const QString& url )
{
	QFileInfo fi( url );
	return fi.exists();
}

bool KomparePart::fetchURL( const KUrl& url, bool addToSource )
{
	// Default value if there is an error is "", we rely on it!
	QString tempFileName( "" );
	// Only in case of error do we set result to false, don't forget!!
	bool result = true;
	KTempDir* tmpDir = 0;

	if ( !url.isLocalFile() )
	{
		KIO::UDSEntry node;
		KIO::NetAccess::stat( url, node, widget() );
		if ( !node.isDir() )
		{
			if ( ! KIO::NetAccess::download( url, tempFileName, widget() ) )
			{
				slotShowError( i18n( "<qt>The URL <b>%1</b> cannot be downloaded.</qt>", url.prettyUrl() ) );
				tempFileName = ""; // Not sure if download has already touched this tempFileName when there is an error
				result = false;
			}
		}
		else
		{
			tmpDir = new KTempDir(KStandardDirs::locateLocal("tmp", "kompare"));
			tmpDir->setAutoRemove( true ); // Yes this is the default but just to make sure
			if ( ! KIO::NetAccess::dircopy( url, KUrl( tmpDir->name() ), widget() ) )
			{
				slotShowError( i18n( "<qt>The URL <b>%1</b> cannot be downloaded.</qt>", url.prettyUrl() ) );
				delete tmpDir;
				tmpDir = 0;
				result = false;
			}
			else
			{
				tempFileName = tmpDir->name();
				kDebug(8101) << "tempFileName = " << tempFileName << endl;
				// If a directory is copied into KTempDir then the directory in 
				// here is what I need to add to tempFileName
				QDir dir( tempFileName );
				QStringList entries = dir.entryList( QDir::Dirs | QDir::NoDotAndDotDot );
				if ( entries.size() == 1 ) // More than 1 entry in here means big problems!!!
				{
					if ( !tempFileName.endsWith( '/' ) )
						tempFileName += '/';
					tempFileName += entries.at( 0 );
					tempFileName += '/';
				}
				else
				{
					kDebug(8101) << "Yikes, nothing downloaded?" << endl;
					delete tmpDir;
					tmpDir = 0;
					tempFileName = "";
					result = false;
				}
			}
		}
	}
	else
	{
		// is Local already, check if exists
		if ( exists( url.toLocalFile() ) )
			tempFileName = url.toLocalFile();
		else
		{
			slotShowError( i18n( "<qt>The URL <b>%1</b> does not exist on your system.</qt>", url.prettyUrl() ) );
			result = false;
		}
	}

	if ( addToSource )
	{
		m_info.localSource = tempFileName;
		m_info.sourceKTempDir = tmpDir;
	}
	else
	{
		m_info.localDestination = tempFileName;
		m_info.destinationKTempDir = tmpDir;
	}

	return result;
}

void KomparePart::cleanUpTemporaryFiles()
{
	kDebug(8101) << "Cleaning temporary files." << endl;
	if ( !m_info.localSource.isEmpty() )
	{
		if ( m_info.sourceKTempDir == 0 )
			KIO::NetAccess::removeTempFile( m_info.localSource );
		else
		{
			delete m_info.sourceKTempDir;
			m_info.sourceKTempDir = 0;
		}
		m_info.localSource = "";
	}
	if ( !m_info.localDestination.isEmpty() )
	{
		if ( m_info.destinationKTempDir == 0  )
			KIO::NetAccess::removeTempFile( m_info.localDestination );
		else
		{
			delete m_info.destinationKTempDir;
			m_info.destinationKTempDir = 0;
		}
		m_info.localDestination = "";
	}
}

void KomparePart::compare( const KUrl& source, const KUrl& destination )
{
	// FIXME: This is silly, i can use NetAccess::stat to figure out what it is and not
	// wait until i am in the modellist to determine the mode we're supposed to be in.
	// That should make the code more readable
	// I should store the KTempDir(s)/File(s) in the Info struct as well and delete it at the right time
	m_info.source = source;
	m_info.destination = destination;

	// FIXME: (Not urgent) But turn this into an enum, for now i cant find a nice name for the enum that has Source and Destination as values
	// For now we do not do error checking, user has already been notified and if the localString is empty then we dont diff
	fetchURL( source, true );
	fetchURL( destination, false );

	emit kompareInfo( &m_info );

	compareAndUpdateAll();
}

void KomparePart::compareFileString( const KUrl & sourceFile, const QString & destination)
{
	//Set the modeto specify that the source is a file, and the destination is a string
	m_info.mode = Kompare::ComparingFileString;
	
	m_info.source = sourceFile;
	m_info.localDestination = destination;
	
	fetchURL(sourceFile, true);
	
	emit kompareInfo( &m_info );
	
	compareAndUpdateAll();
}

void KomparePart::compareStringFile( const QString & source, const KUrl & destinationFile)
{
	//Set the modeto specify that the source is a file, and the destination is a string
	m_info.mode = Kompare::ComparingStringFile;
	
	m_info.localSource = source;
	m_info.destination = destinationFile;
	
	fetchURL(destinationFile, false);
	
	emit kompareInfo( &m_info );
	
	compareAndUpdateAll();
}

void KomparePart::compareFiles( const KUrl& sourceFile, const KUrl& destinationFile )
{
	m_info.mode = Kompare::ComparingFiles;

	m_info.source = sourceFile;
	m_info.destination = destinationFile;

	// FIXME: (Not urgent) But turn this into an enum, for now i cant find a nice name for the enum that has Source and Destination as values
	// For now we do not do error checking, user has already been notified and if the localString is empty then we dont diff
	fetchURL( sourceFile, true );
	fetchURL( destinationFile, false );

	emit kompareInfo( &m_info );

	compareAndUpdateAll();
}

void KomparePart::compareDirs( const KUrl& sourceDirectory, const KUrl& destinationDirectory )
{
	m_info.mode = Kompare::ComparingDirs;

	m_info.source = sourceDirectory;
	m_info.destination = destinationDirectory;

	fetchURL( sourceDirectory, true );
	fetchURL( destinationDirectory, false );

	emit kompareInfo( &m_info );

	compareAndUpdateAll();
}

void KomparePart::compare3Files( const KUrl& /*originalFile*/, const KUrl& /*changedFile1*/, const KUrl& /*changedFile2*/ )
{
	// FIXME: actually implement this some day :)
	updateActions();
	updateCaption();
	updateStatus();
}

void KomparePart::openFileAndDiff( const KUrl& file, const KUrl& diffFile )
{
	m_info.source = file;
	m_info.destination = diffFile;

	fetchURL( file, true );
	fetchURL( diffFile, false );
	m_info.mode = Kompare::BlendingFile;

	emit kompareInfo( &m_info );

	compareAndUpdateAll();
}

void KomparePart::openDirAndDiff ( const KUrl& dir,  const KUrl& diffFile )
{
	m_info.source = dir;
	m_info.destination = diffFile;

	fetchURL( dir, true );
	fetchURL( diffFile, false );
	m_info.mode = Kompare::BlendingDir;

	emit kompareInfo( &m_info );

	if ( !m_info.localSource.isEmpty() && !m_info.localDestination.isEmpty() )
	{
		m_modelList->openDirAndDiff();
		//Must this be in here? couldn't we use compareAndUpdateAll as well?
		updateActions();
		updateCaption();
		updateStatus();
	}
}

bool KomparePart::openFile()
{
	// This is called from openURL
	// This is a little inefficient but i will do it anyway
	openDiff( url() );
	return true;
}

bool KomparePart::saveAll()
{
	bool result = m_modelList->saveAll();
	updateActions();
	updateCaption();
	updateStatus();
	return result;
}

void KomparePart::saveDiff()
{
	KDialog dlg( widget() );
	dlg.setName( "save_options" );
	dlg.setModal( true );
	dlg.setWindowTitle( i18n("Diff Options") );
	dlg.setButtons( KDialog::Ok|KDialog::Cancel );
	KompareSaveOptionsWidget* w = new KompareSaveOptionsWidget(
	                                             m_info.localSource,
	                                             m_info.localDestination,
	                                             m_diffSettings, &dlg );
	dlg.setMainWidget( w );
	dlg.setButtonGuiItem( KDialog::Ok, KStandardGuiItem::save() );

	if( dlg.exec() ) {
		w->saveOptions();
		KSharedConfig::Ptr config = componentData().config();
		saveProperties( config.data() );
		config->sync();

		while ( 1 )
		{
			KUrl url = KFileDialog::getSaveUrl( m_info.destination.url(),
			              i18n("*.diff *.dif *.patch|Patch Files"), widget(), i18n( "Save .diff" ) );
			if ( KIO::NetAccess::exists( url, KIO::NetAccess::DestinationSide, widget() ) )
			{
				int result = KMessageBox::warningYesNoCancel( widget(), i18n("The file exists or is write-protected; do you want to overwrite it?"), i18n("File Exists"), KGuiItem(i18n("Overwrite")), KGuiItem(i18n("Do Not Overwrite")) );
				if ( result == KMessageBox::Cancel )
				{
					break;
				}
				else if ( result == KMessageBox::No )
				{
					continue;
				}
				else
				{
					kDebug(8103) << "URL = " << url.prettyUrl() << endl;
					kDebug(8103) << "Directory = " << w->directory() << endl;
					kDebug(8103) << "DiffSettings = " << m_diffSettings << endl;

					m_modelList->saveDiff( url.url(), w->directory(), m_diffSettings );
					break;
				}
			}
			else
			{
				kDebug(8103) << "URL = " << url.prettyUrl() << endl;
				kDebug(8103) << "Directory = " << w->directory() << endl;
				kDebug(8103) << "DiffSettings = " << m_diffSettings << endl;

				m_modelList->saveDiff( url.url(), w->directory(), m_diffSettings );
				break;
			}
		}
	}
}

void KomparePart::slotFilePrint()
{
	QPrinter printer;
	printer.setOrientation( QPrinter::Landscape );
	QPrintDialog* dlg = KdePrint::createPrintDialog( &printer, m_splitter );

	if ( dlg->exec() == QDialog::Accepted )
	{
		// do some printing in qprinter
		slotPaintRequested( &printer );
	}

	delete dlg;
}

void KomparePart::slotFilePrintPreview()
{
	QPrinter printer;
	printer.setOrientation( QPrinter::Landscape );
	QPrintPreviewDialog dlg( &printer );

	connect( &dlg, SIGNAL(paintRequested(QPrinter*)), this, SLOT(slotPaintRequested(QPrinter*)) );

	dlg.exec();
}

void KomparePart::slotPaintRequested( QPrinter* printer )
{
	kDebug(8103) << "Now paint something..." << endl;
	QPainter p;
	p.begin( printer );

	QSize widgetWidth = m_splitter->parentWidget()->size();
	kDebug(8103) << "printer.width()     = " << printer->width() << endl;
	kDebug(8103) << "widgetWidth.width() = " << widgetWidth.width() << endl;
	qreal factor = ((qreal)printer->width())/((qreal)widgetWidth.width());

	kDebug(8103) << "factor              = " << factor << endl;

	p.scale( factor, factor );
	m_splitter->parentWidget()->render( &p );

	p.end();
	kDebug(8103) << "Done painting something..." << endl;
}

KAboutData* KomparePart::createAboutData()
{
    KAboutData *about = new KAboutData("kompare", 0, ki18n("KomparePart"), "4.0");
    about->addAuthor(ki18n("John Firebaugh"), ki18n("Author"), "jfirebaugh@kde.org");
    about->addAuthor(ki18n("Otto Bruggeman"), ki18n("Author"), "bruggie@gmail.com" );
    about->addAuthor(ki18n("Kevin Kofler"), ki18n("Author"), "kevin.kofler@chello.at" );
    return about;
}

void KomparePart::slotSetStatus( enum Kompare::Status status )
{
	updateActions();

	switch( status ) {
	case Kompare::RunningDiff:
		emit setStatusBarText( i18n( "Running diff..." ) );
		break;
	case Kompare::Parsing:
		emit setStatusBarText( i18n( "Parsing diff output..." ) );
		break;
	case Kompare::FinishedParsing:
		updateStatus();
		break;
	case Kompare::FinishedWritingDiff:
		updateStatus();
		emit diffURLChanged();
		break;
	default:
		break;
	}
}

void KomparePart::updateCaption()
{
	QString source = m_info.source.prettyUrl();
	QString destination = m_info.destination.prettyUrl();

	QString text;

	switch ( m_info.mode )
	{
	case Kompare::ComparingFiles :
	case Kompare::ComparingDirs :
	case Kompare::BlendingFile :
	case Kompare::BlendingDir :
		text = source + " -- " + destination; // no need to translate this " -- "
		break;
	case Kompare::ShowingDiff :
		text = source;
		break;
	default:
		break;
	}

	emit setWindowCaption( text );
}

void KomparePart::updateStatus()
{
	QString source = m_info.source.prettyUrl();
	QString destination = m_info.destination.prettyUrl();

	QString text;

	switch ( m_info.mode )
	{
	case Kompare::ComparingFiles :
		text = i18n( "Comparing file %1 with file %2" ,
		     source ,
		     destination );
		break;
	case Kompare::ComparingDirs :
		text = i18n( "Comparing files in %1 with files in %2" ,
		     source ,
		     destination );
		break;
	case Kompare::ShowingDiff :
		text = i18n( "Viewing diff output from %1", source );
		break;
	case Kompare::BlendingFile :
		text = i18n( "Blending diff output from %1 into file %2" ,
		      source ,
		      destination );
		break;
	case Kompare::BlendingDir :
		text = i18n( "Blending diff output from %1 into folder %2" ,
		      m_info.source.prettyUrl() ,
		      m_info.destination.prettyUrl() );
		break;
	default:
		break;
	}

	emit setStatusBarText( text );
}

void KomparePart::compareAndUpdateAll()
{
	if ( !m_info.localSource.isEmpty() && !m_info.localDestination.isEmpty() )
	{
		switch(m_info.mode)
		{
		default:
		case Kompare::UnknownMode:
			m_modelList->compare();
			break;
		
		case Kompare::ComparingStringFile:
		case Kompare::ComparingFileString:
		case Kompare::ComparingFiles:
		case Kompare::ComparingDirs:
			m_modelList->compare(m_info.mode);
			break;
			
		case Kompare::BlendingFile:
			m_modelList->openFileAndDiff();
			break;
		}
		updateCaption();
		updateStatus();
	}
	updateActions();
}

void KomparePart::slotShowError( QString error )
{
	KMessageBox::error( widget(), error );
}

void KomparePart::slotSwap()
{
	if ( m_modelList->hasUnsavedChanges() )
	{
		int query = KMessageBox::warningYesNoCancel
		            (
		                widget(),
		                i18n( "You have made changes to the destination file(s).\n"
		                      "Would you like to save them?" ),
		                i18n(  "Save Changes?" ),
		                KStandardGuiItem::save(),
		                KStandardGuiItem::discard()
		            );

		if ( query == KMessageBox::Yes )
			m_modelList->saveAll();

		if ( query == KMessageBox::Cancel )
			return; // Abort prematurely so no swapping
	}

	// Swap the info in the Kompare::Info struct
	m_info.swapSourceWithDestination();

	// Update window caption and statusbar text
	updateCaption();
	updateStatus();

	m_modelList->swap();
}

void KomparePart::slotRefreshDiff()
{
	if ( m_modelList->hasUnsavedChanges() )
	{
		int query = KMessageBox::warningYesNoCancel
		            (
		                widget(),
		                i18n( "You have made changes to the destination file(s).\n"
		                      "Would you like to save them?" ),
		                i18n(  "Save Changes?" ),
		                KStandardGuiItem::save(),
		                KStandardGuiItem::discard()
		            );

		if ( query == KMessageBox::Cancel )
			return; // Abort prematurely so no refreshing

		if ( query == KMessageBox::Yes )
			m_modelList->saveAll();
	}

	// For this to work properly you have to refetch the files from their (remote) locations
	cleanUpTemporaryFiles();
	fetchURL( m_info.source, true );
	fetchURL( m_info.destination, false );
	m_modelList->refresh();
}

void KomparePart::slotShowDiffstats( void )
{
	// Fetch all the args needed for komparestatsmessagebox
	// oldfile, newfile, diffformat, noofhunks, noofdiffs

	QString oldFile;
	QString newFile;
	QString diffFormat;
	int filesInDiff;
	int noOfHunks;
	int noOfDiffs;

	oldFile = m_modelList->selectedModel() ? m_modelList->selectedModel()->sourceFile()  : QString( "" );
	newFile = m_modelList->selectedModel() ? m_modelList->selectedModel()->destinationFile() : QString( "" );

	if ( m_modelList->selectedModel() )
	{
		switch( m_info.format ) {
		case Kompare::Unified :
			diffFormat = i18n( "Unified" );
			break;
		case Kompare::Context :
			diffFormat = i18n( "Context" );
			break;
		case Kompare::RCS :
			diffFormat = i18n( "RCS" );
			break;
		case Kompare::Ed :
			diffFormat = i18n( "Ed" );
			break;
		case Kompare::Normal :
			diffFormat = i18n( "Normal" );
			break;
		case Kompare::UnknownFormat :
		default:
			diffFormat = i18n( "Unknown" );
			break;
		}
	}
	else
	{
		diffFormat = "";
	}

	filesInDiff = m_modelList->modelCount();

	noOfHunks = m_modelList->selectedModel() ? m_modelList->selectedModel()->hunkCount() : 0;
	noOfDiffs = m_modelList->selectedModel() ? m_modelList->selectedModel()->differenceCount() : 0;

	if ( m_modelList->modelCount() == 0 ) { // no diff loaded yet
		KMessageBox::information( 0L, i18n(
		    "No diff file, or no 2 files have been diffed. "
		    "Therefore no stats are available."),
		    i18n("Diff Statistics"), QString(), 0 );
	}
	else if ( m_modelList->modelCount() == 1 ) { // 1 file in diff, or 2 files compared
		KMessageBox::information( 0L, i18n(
		    "Statistics:\n"
		    "\n"
		    "Old file: %1\n"
		    "New file: %2\n"
		    "\n"
		    "Format: %3\n"
		    "Number of hunks: %4\n"
		    "Number of differences: %5",
		     oldFile, newFile, diffFormat,
		     noOfHunks, noOfDiffs),
		    i18n("Diff Statistics"), QString(), 0 );
	} else { // more than 1 file in diff, or 2 directories compared
		KMessageBox::information( 0L, ki18n(
		    "Statistics:\n"
		    "\n"
		    "Number of files in diff file: %1\n"
		    "Format: %2\n"
		    "\n"
		    "Current old file: %3\n"
		    "Current new file: %4\n"
		    "\n"
		    "Number of hunks: %5\n"
		    "Number of differences: %6")
		    .subs(filesInDiff).subs(diffFormat).subs(oldFile)
		    .subs(newFile).subs(noOfHunks).subs(noOfDiffs)
                    .toString(),
		    i18n("Diff Statistics"), QString(), 0 );
	}
}

bool KomparePart::queryClose()
{
	if ( !m_modelList->hasUnsavedChanges() ) return true;

	int query = KMessageBox::warningYesNoCancel
	            (
	                widget(),
	                i18n("You have made changes to the destination file(s).\n"
	                     "Would you like to save them?" ),
	                i18n( "Save Changes?" ),
	                KStandardGuiItem::save(),
	                KStandardGuiItem::discard()
	            );

	if( query == KMessageBox::Cancel )
		return false;

	if( query == KMessageBox::Yes )
		return m_modelList->saveAll();

	return true;
}

int KomparePart::readProperties( KConfig *config )
{
	m_viewSettings->loadSettings( config );
	m_diffSettings->loadSettings( config );
	emit configChanged();
	return 0;
}

int KomparePart::saveProperties( KConfig *config )
{
	m_viewSettings->saveSettings( config );
	m_diffSettings->saveSettings( config );
	return 0;
}

void KomparePart::optionsPreferences()
{
	// show preferences
	KomparePrefDlg pref( m_viewSettings, m_diffSettings );

	connect( &pref, SIGNAL(configChanged()), this, SIGNAL(configChanged()) );

	if ( pref.exec() )
		emit configChanged();
}

#include "kompare_part.moc"

K_EXPORT_PLUGIN( KomparePartFactory )
