/***************************************************************************
                                kompare_part.cpp  -  description
                                -------------------
        begin                   : Sun Mar 4 2001
        copyright               : (C) 2001-2003 by Otto Bruggeman
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

#include <qlayout.h>
#include <qwidget.h>

#include <kdebug.h>
#include <kfiletreeview.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstdaction.h>
#include <kinstance.h>
#include <ktempfile.h>
//#include <ktempdir.h>

#include <kio/netaccess.h>

#include "diffmodel.h"
#include "diffsettings.h"
#include "viewsettings.h"
#include "kompare_actions.h"
#include "kompare_part.h"
#include "komparelistview.h"
#include "kompareconnectwidget.h"
#include "kompareprefdlg.h"
#include "komparesaveoptionswidget.h"
#include "kompareview.h"
#include "miscsettings.h"


ViewSettings* KomparePart::m_viewSettings = 0L;
DiffSettings* KomparePart::m_diffSettings = 0L;

KomparePart::KomparePart( QWidget *parentWidget, const char *widgetName,
                      QObject *parent, const char *name ) :
	KParts::ReadWritePart(parent, name),
	m_tempDiff( 0 )
{
	// we need an instance
	setInstance( KomparePartFactory::instance() );

	if( !m_viewSettings ) {
		m_viewSettings = new ViewSettings( 0 );
	}
	if( !m_diffSettings ) {
		m_diffSettings = new DiffSettings( 0 );
	}

	// This creates the "Model creator" and connects the signals and slots
	m_modelList = new Diff2::KompareModelList( m_diffSettings, m_viewSettings, this, "komparemodellist" );
	connect( m_modelList, SIGNAL(status( Kompare::Status )),
	         this, SLOT(slotSetStatus( Kompare::Status )) );
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

	connect( m_modelList, SIGNAL( setModified( bool ) ),
	         this, SLOT( slotSetModified( bool ) ) );

	// This is the stuff to connect the "interface" of the kompare part to the model inside
	connect( m_modelList, SIGNAL(modelsChanged(const QPtrList<Diff2::DiffModel>*)),
	         this, SIGNAL(modelsChanged(const QPtrList<Diff2::DiffModel>*)) );

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

	// This creates the viewwidget and connects the signals and slots
	m_diffView = new KompareView( m_viewSettings, parentWidget, widgetName );

	connect( m_modelList, SIGNAL(setSelection(const Diff2::DiffModel*, const Diff2::Difference*)),
	         m_diffView, SLOT(slotSetSelection(const Diff2::DiffModel*, const Diff2::Difference*)) );
	connect( m_modelList, SIGNAL(setSelection(const Diff2::Difference*)),
	         m_diffView, SLOT(slotSetSelection(const Diff2::Difference*)) );
	connect( m_diffView, SIGNAL(selectionChanged(const Diff2::Difference*)),
	         m_modelList, SLOT(slotSelectionChanged(const Diff2::Difference*)) );

	// left view (source)
	connect( m_modelList, SIGNAL(applyDifference(bool)),
	         m_diffView->srcLV(), SLOT(slotApplyDifference(bool)) );
	connect( m_modelList, SIGNAL(applyAllDifferences(bool)),
	         m_diffView->srcLV(), SLOT(slotApplyAllDifferences(bool)) );
	connect( m_modelList, SIGNAL(applyDifference(const Diff2::Difference*, bool)),
	         m_diffView->srcLV(), SLOT(slotApplyDifference(const Diff2::Difference*, bool)) );
	connect( this, SIGNAL(configChanged()), m_diffView->srcLV(), SLOT(slotConfigChanged()) );
	// right view (destination)
	connect( m_modelList, SIGNAL(applyDifference(bool)),
	         m_diffView->destLV(), SLOT(slotApplyDifference(bool)) );
	connect( m_modelList, SIGNAL(applyAllDifferences(bool)),
	         m_diffView->destLV(), SLOT(slotApplyAllDifferences(bool)) );
	connect( m_modelList, SIGNAL(applyDifference(const Diff2::Difference*, bool)),
	         m_diffView->destLV(), SLOT(slotApplyDifference(const Diff2::Difference*, bool)) );
	connect( this, SIGNAL(configChanged()), m_diffView->destLV(), SLOT(slotConfigChanged()) );

	// kompareactions
	m_kompareActions = new KompareActions( this, "KompareActions" );
	connect( m_modelList, SIGNAL(setSelection(const Diff2::DiffModel*, const Diff2::Difference*)),
	         m_kompareActions, SLOT(slotSetSelection(const Diff2::DiffModel*, const Diff2::Difference*)) );
	connect( m_modelList, SIGNAL(setSelection(const Diff2::Difference*)),
	         m_kompareActions, SLOT(slotSetSelection(const Diff2::Difference*)) );
	connect( m_kompareActions, SIGNAL(selectionChanged(const Diff2::DiffModel*, const Diff2::Difference*)),
	         m_modelList, SLOT(slotSelectionChanged(const Diff2::DiffModel*, const Diff2::Difference*)) );
	connect( m_kompareActions, SIGNAL(selectionChanged(const Diff2::Difference*)),
	         m_modelList, SLOT(slotSelectionChanged(const Diff2::Difference*)) );
	connect( m_modelList, SIGNAL(modelsChanged(const QPtrList<Diff2::DiffModel>*)),
	         m_kompareActions, SLOT(slotModelsChanged(const QPtrList<Diff2::DiffModel>*)) );
	connect( m_kompareActions, SIGNAL(applyDifference(bool)),
	         m_modelList, SLOT(slotApplyDifference(bool)) );
	connect( m_kompareActions, SIGNAL(applyAllDifferences(bool)),
	         m_modelList, SLOT(slotApplyAllDifferences(bool)) );
	connect( m_kompareActions, SIGNAL(previousModel()),
	         m_modelList, SLOT(slotPreviousModel()) );
	connect( m_kompareActions, SIGNAL(nextModel()),
	         m_modelList, SLOT(slotNextModel()) );
	connect( m_kompareActions, SIGNAL(previousDifference()),
	         m_modelList, SLOT(slotPreviousDifference()) );
	connect( m_kompareActions, SIGNAL(nextDifference()),
	         m_modelList, SLOT(slotNextDifference()) );

	// notify the part that this is our internal widget
	setWidget( m_diffView );

	setupActions();

	readProperties( instance()->config() );

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
}

void KomparePart::setupActions()
{
	// create our actions

	m_save            = KStdAction::save( this, SLOT(saveDestination()), actionCollection() );
	m_saveAll         = new KAction( i18n("Save &All"), "save_all", 0,
	                                 this, SLOT(saveAll()),
	                                 actionCollection(), "file_save_all" );
	m_saveDiff        = new KAction( i18n("Save .&diff"), 0,
	                                 this, SLOT(saveDiff()),
	                                 actionCollection(), "file_save_diff" );
	m_swap            = new KAction( i18n( "Swap Source with Destination" ), 0,
	                                 this, SLOT(slotSwap()),
	                                 actionCollection(), "file_swap" );
	m_diffStats       = new KAction( i18n( "Show Statistics" ), 0,
	                                 this, SLOT(slotShowDiffstats()),
	                                 actionCollection(), "file_diffstats" );

	KStdAction::preferences(this, SLOT(optionsPreferences()), actionCollection());
}

void KomparePart::updateActions()
{
	m_saveAll->setEnabled  ( m_modelList->isModified() );
	m_saveDiff->setEnabled ( m_modelList->mode() == Kompare::ComparingFiles );
	m_swap->setEnabled     ( m_modelList->mode() == Kompare::ComparingFiles );
	m_diffStats->setEnabled( m_modelList->modelCount() > 0 );

	const Diff2::DiffModel* model = m_modelList->selectedModel();
	if ( model )
		m_save->setEnabled( model->isModified() );
	else
		m_save->setEnabled( false );
}

bool KomparePart::openDiff( const KURL& url )
{
	kdDebug(8103) << "Url = " << url.url() << endl;
	bool result = m_modelList->openDiff( url );
	updateActions();
	return result;
}

bool KomparePart::openDiff( const QString& diffOutput )
{
	bool value;

	if ( (value = m_modelList->parseDiffOutput( diffOutput )) == true )
	{
		updateActions();
		updateStatus();
	}
	return value;
}

bool KomparePart::openDiff3( const KURL& diff3Url )
{
	// FIXME: Implement this !!!
	kdDebug() << "Not implemented yet. Filename is: " << diff3Url.url() << endl;
	return false;
}

bool KomparePart::openDiff3( const QString& diff3Output )
{
	// FIXME: Implement this !!!
	kdDebug() << "Not implemented yet. diff3 output is: " << endl;
	kdDebug() << diff3Output << endl;
	return false;
}

void KomparePart::compareFiles( const KURL& source, const KURL& destination )
{
	m_modelList->compareFiles( source, destination );
	updateActions();
	updateStatus();
}

void KomparePart::compareDirs( const KURL& source, const KURL& destination )
{
	m_modelList->compareDirs( source, destination );
	updateActions();
	updateStatus();
}

void KomparePart::compare3Files( const KURL& /*originalFile*/, const KURL& /*changedFile1*/, const KURL& /*changedFile2*/ )
{
	updateActions();
	updateStatus();
}

void KomparePart::openFileAndDiff( const KURL& file, const KURL& diffFile )
{
	QString tempDiffFile;
	QString tempFile;

	m_modelList->clear();

	if ( !KIO::NetAccess::download( file, tempFile ) )
		return;

	if ( !KIO::NetAccess::download( diffFile, tempDiffFile ) )
	{
		KIO::NetAccess::removeTempFile( tempFile );
		return;
	}

	m_file = tempDiffFile;

	kdDebug(8103) << "Parsing the file contents..." << endl;
	if ( m_modelList->parseDiffOutput( readFile() ) == 0 )
	{
		kdDebug(8103) << "Parsing complete..." << endl;
	}
	else
	{
		kdDebug(8103) << "Parsing gave us some problems, no model(s) and or no difference(s)" << endl;
		KMessageBox::error( 0L, i18n("No models or no differences, this file: %1, is not a diff file").arg( diffFile.prettyURL() ) );
		return;
	}

	// Do our thing :)
	if ( !m_modelList->blendOriginalIntoModelList( tempFile ) )
	{
		// Trouble blending the original into the model
		kdDebug(8103) << "Oops cant blend original file into modellist : " << tempFile << endl;
	}

	// Still need to show the models even if something went wrong,
	// kompare will then act as if openDiff was called
	m_modelList->show();

	// Clean up after ourselves
	KIO::NetAccess::removeTempFile( tempFile );
	KIO::NetAccess::removeTempFile( tempDiffFile );
}

void KomparePart::openDirAndDiff ( const KURL& dir,  const KURL& diffFile )
{
	QString tempDiffFile;
	QString tempDir;

	m_modelList->clear();

	if ( !KIO::NetAccess::download( dir, tempDir ) )
		return;

	if ( !KIO::NetAccess::download( diffFile, tempDiffFile ) )
	{
		KIO::NetAccess::removeTempFile( tempDir );
		return;
	}

	m_file = tempDiffFile;

	kdDebug(8103) << "Parsing the file contents..." << endl;
	if ( m_modelList->parseDiffOutput( readFile() ) == 0 )
	{
		kdDebug(8103) << "Parsing complete..." << endl;
	}
	else
	{
		kdDebug(8103) << "Parsing gave us some problems, no model(s) and or no difference(s)" << endl;
		KMessageBox::error( 0L, i18n("No models or no differences, this file: %1, is not a diff file").arg( diffFile.prettyURL() ) );
		return;
	}

	// Do our thing :)
	if ( !m_modelList->blendOriginalIntoModelList( tempDir ) )
	{
		// Trouble blending the original into the model
		kdDebug(8103) << "Oops cant blend original dir into modellist : " << tempDir << endl;
	}

	// Still need to show the models even if something went wrong,
	// kompare will then act as if openDiff was called
	m_modelList->show();

	// Clean up after ourselves
	KIO::NetAccess::removeTempFile( tempDir );
	KIO::NetAccess::removeTempFile( tempDiffFile );
}

QStringList& KomparePart::readFile()
{
	QStringList*  lines = new QStringList();
	QFile file( m_file );
	file.open(  IO_ReadOnly );
	QTextStream stream( &file );

	kdDebug() << "Reading from m_file = " << m_file << endl;
	while ( !stream.eof() )
	{
		lines->append( stream.readLine() );
	}

	file.close();

	return *lines;
}

bool KomparePart::openFile()
{
	kdDebug() << "Please don't use this method ! Use openDiff( url ) instead." << endl;
	return false;
}

bool KomparePart::saveDestination()
{
	const Diff2::DiffModel* model = m_modelList->selectedModel();
	if ( model )
	{
		bool result = m_modelList->saveDestination( model );
		updateActions();
		updateStatus();
		return result;
	}
	else
	{
		return false;
	}
}

bool KomparePart::saveAll()
{
	bool result = m_modelList->saveAll();
	updateActions();
	updateStatus();
	return result;
}

void KomparePart::saveDiff()
{
	KDialogBase* dlg = new KDialogBase( widget(), "save options",
	                                    true /* modal */, i18n("Diff Options"),
	                                    KDialogBase::Ok|KDialogBase::Cancel );
	KompareSaveOptionsWidget* w = new KompareSaveOptionsWidget(
	                                             m_modelList->sourceTemp(),
	                                             m_modelList->destinationTemp(),
	                                             m_diffSettings, dlg );
	dlg->setMainWidget( w );
	dlg->setButtonOKText( i18n("Save") );

	if( dlg->exec() ) {
		w->saveOptions();
		KConfig* config = instance()->config();
		saveProperties( config );
		config->sync();
		KURL url = KFileDialog::getSaveURL( m_modelList->destinationBaseURL().url(),
		              i18n("*.diff *.dif *.patch|Patch files"), widget(), i18n( "Save .diff" ) );
		m_modelList->saveDiff( url, w->directory(), m_diffSettings, m_viewSettings );
	}
	delete dlg;
}

KURL KomparePart::diffURL()
{
	// This should just call url from the ReadOnlyPart, or leave it out completely
	if( m_modelList->diffURL().isEmpty() ) {
		saveDiff();
	}
	return m_modelList->diffURL();
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

void KomparePart::updateStatus()
{
	if( m_modelList->mode() == Kompare::ComparingFiles )
	{
		if( m_modelList->modelCount() > 1 )
		{
			emit setStatusBarText( i18n( "Comparing files in %1 with files in %2" )
			   .arg( m_modelList->sourceBaseURL().prettyURL() )
			   .arg( m_modelList->destinationBaseURL().prettyURL() ) );
			emit setWindowCaption( m_modelList->sourceBaseURL().prettyURL()
			   + " : " + m_modelList->destinationBaseURL().prettyURL() );
		}
		else if ( m_modelList->modelCount() == 1 )
		{
			emit setStatusBarText( i18n( "Comparing %1 with %2" )
			   .arg( m_modelList->sourceBaseURL().prettyURL( 1 )
			   + m_modelList->modelAt( 0 )->sourceFile() )
			   .arg( m_modelList->destinationBaseURL().prettyURL( 1 )
			   + m_modelList->modelAt( 0 )->destinationFile() ) );
			emit setWindowCaption(  m_modelList->modelAt( 0 )->sourceFile()
			   + " : " + m_modelList->modelAt( 0 )->destinationFile() );
		}
	}
	else
	{
		emit setStatusBarText( i18n( "Viewing %1" ).arg( m_modelList->diffURL().prettyURL() ) );
		emit setWindowCaption( m_modelList->diffURL().filename() );
	}
}

void KomparePart::slotShowError( QString error )
{
	KMessageBox::error( widget(), error );
}

void KomparePart::slotSwap()
{
	m_modelList->swap();
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

	oldFile = m_modelList->selectedModel() ? m_modelList->selectedModel()->sourceFile()  : QString::null;
	newFile = m_modelList->selectedModel() ? m_modelList->selectedModel()->destinationFile() : QString::null;
	if ( m_modelList->selectedModel() )
	{
		switch( m_modelList->format() ) {
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
		    i18n("Diff Statistics"), QString::null, false );
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
		    "Number of differences: %5")
		    .arg(oldFile).arg(newFile).arg(diffFormat)
		    .arg(noOfHunks).arg(noOfDiffs),
		    i18n("Diff Statistics"), QString::null, false );
	} else { // more than 1 file in diff, or 2 directories compared
		KMessageBox::information( 0L, i18n(
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
		    .arg(filesInDiff).arg(diffFormat).arg(oldFile)
		    .arg(newFile).arg(noOfHunks).arg(noOfDiffs),
		    i18n("Diff Statistics"), QString::null, false );
	}
}

bool KomparePart::queryClose()
{
	if( !isModified() ) return true;

	int query = KMessageBox::warningYesNoCancel( widget(),
	    i18n("You have made changes to the destination.\n"
	         "Would you like to save them?" ),
	         i18n( "Save Changes?" ),
	         i18n( "Save" ),
	         i18n( "Discard" ) );

	if( query == KMessageBox::Cancel ) return false;
	if( query == KMessageBox::Yes )    return m_modelList->saveAll();
	return true;
}

int KomparePart::readProperties( KConfig *config )
{
	config->setGroup( "View" );
	m_viewSettings->loadSettings( config );
	config->setGroup( "DiffSettings" );
	m_diffSettings->loadSettings   ( config );
	emit configChanged();
	return 0;
}

int KomparePart::saveProperties( KConfig *config )
{
	config->setGroup( "View" );
	m_viewSettings->saveSettings( config );
	config->setGroup( "DiffSettings" );
	m_diffSettings->saveSettings   ( config );
	return 0;
}

void KomparePart::optionsPreferences()
{
	// show preferences
	KomparePrefDlg* pref = new KomparePrefDlg( m_viewSettings, m_diffSettings );

	if ( pref->exec() ) {
		KConfig* config = instance()->config();
		saveProperties( config );
		config->sync();
		//FIXME: maybe this signal should also be emitted when
		// Apply is pressed. I'll figure it out this week.
		emit configChanged();
	}
}

void KomparePart::slotSetModified( bool modified )
{
	setModified( modified );
	updateActions();
}

// It's usually safe to leave the factory code alone.. with the
// notable exception of the KAboutData data
#include <kaboutdata.h>

KInstance*  KomparePartFactory::s_instance = 0L;
KAboutData* KomparePartFactory::s_about = 0L;

KomparePartFactory::KomparePartFactory()
    : KParts::Factory()
{
}

KomparePartFactory::~KomparePartFactory()
{
	delete s_instance;
	delete s_about;

	s_instance = 0L;
}

KParts::Part* KomparePartFactory::createPartObject( QWidget *parentWidget, const char *widgetName,
                                                  QObject *parent, const char *name,
                                                  const char *classname, const QStringList & /*args*/ )
{
	// Create an instance of our Part
	KomparePart* obj = new KomparePart( parentWidget, widgetName, parent, name );

	// See if we are to be read-write or not
	if (QCString(classname) == "KParts::ReadOnlyPart")
		obj->setReadWrite(false);

	KGlobal::locale()->insertCatalogue("kompare");

	return obj;
}

KInstance* KomparePartFactory::instance()
{
	if( !s_instance )
	{
		s_about = new KAboutData("komparepart", I18N_NOOP("KomparePart"), "2.1");
		s_about->addAuthor("John Firebaugh", "Author", "jfirebaugh@kde.org");
		s_about->addAuthor("Otto Bruggeman", "Author", "otto.bruggeman@home.nl" );
		s_instance = new KInstance(s_about);
	}
	return s_instance;
}

extern "C"
{
	void* init_libkomparepart()
	{
		return new KomparePartFactory;
	}
}

#include "kompare_part.moc"
