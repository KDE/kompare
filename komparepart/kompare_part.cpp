/***************************************************************************
                                kompare_part.cpp  -  description
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

#include <qlayout.h>
#include <qwidget.h>

#include <kdebug.h>
#include <kfiletreeview.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstdaction.h>
#include <kinstance.h>

#include "diffmodel.h"
#include "diffsettings.h"
#include "generalsettings.h"
#include "kompare_actions.h"
#include "kompare_part.h"
//#include "komparenavigationtree.h"
// needed for the connect so it knows KompareConnectWidget and KompareListView are QObjects..
#include "komparelistview.h"
#include "kompareconnectwidget.h"
#include "kompareprefdlg.h"
#include "komparesaveoptionswidget.h"
#include "kompareview.h"
#include "miscsettings.h"


GeneralSettings* KomparePart::m_generalSettings = 0L;
DiffSettings*    KomparePart::m_diffSettings    = 0L;
MiscSettings*    KomparePart::m_miscSettings    = 0L;

KomparePart::KomparePart( QWidget *parentWidget, const char *widgetName,
                      QObject *parent, const char *name ) :
	KParts::ReadWritePart(parent, name),
	m_navigationTree( 0 ),
	m_tempDiff( 0 )
{
	// we need an instance
	setInstance( KomparePartFactory::instance() );

	if( !m_generalSettings ) {
		m_generalSettings = new GeneralSettings( 0 );
		m_diffSettings    = new DiffSettings   ( 0 );
		m_miscSettings    = new MiscSettings   ( 0 );
	}

	m_models = new KompareModelList();
	connect( m_models, SIGNAL(status( Kompare::Status )),
	         this, SLOT(slotSetStatus( Kompare::Status )) );
	connect( m_models, SIGNAL(error( QString )),
	         this, SLOT(slotShowError( QString )) );
	connect( m_models, SIGNAL(applyAllDifferences( bool )),
	         this, SLOT(updateActions()) );
	connect( m_models, SIGNAL(applyDifference( bool )),
	         this, SLOT(updateActions()) );
	connect( m_models, SIGNAL(applyAllDifferences( bool )),
	         this, SIGNAL(appliedChanged()) );
	connect( m_models, SIGNAL(applyDifference( bool )),
	         this, SIGNAL(appliedChanged()) );
	connect( m_models, SIGNAL(setSelection(const DiffModel*, const Difference*)),
	         this, SIGNAL(selectionChanged(const DiffModel*, const Difference*)) );
//	connect( m_models, SIGNAL(modelsChanged( const QPtrList<DiffModel>* )),
//	         this, SLOT(slotModelsChanged( const QPtrList<DiffModel>* )) );

	// this should be your custom internal widget
	m_diffView = new KompareView( m_generalSettings, parentWidget, widgetName );
	
	connect( m_models, SIGNAL(setSelection(const DiffModel*, const Difference*)),
	         m_diffView, SLOT(slotSetSelection(const DiffModel*, const Difference*)) );
	connect( m_models, SIGNAL(setSelection(const Difference*)),
	         m_diffView, SLOT(slotSetSelection(const Difference*)) );
	connect( m_diffView, SIGNAL(selectionChanged(const Difference*)),
	         m_models, SLOT(slotSelectionChanged(const Difference*)) );
	
	// left view (source)
	connect( m_models, SIGNAL(applyDifference(bool)),
	         m_diffView->srcLV(), SLOT(slotApplyDifference(bool)) );
	connect( m_models, SIGNAL(applyAllDifferences(bool)),
	         m_diffView->srcLV(), SLOT(slotApplyAllDifferences(bool)) );
	connect( m_models, SIGNAL(applyDifference(const Difference*, bool)),
	         m_diffView->srcLV(), SLOT(slotApplyDifference(const Difference*, bool)) );
	// right view (destination)
	connect( m_models, SIGNAL(applyDifference(bool)),
	         m_diffView->destLV(), SLOT(slotApplyDifference(bool)) );
	connect( m_models, SIGNAL(applyAllDifferences(bool)),
	         m_diffView->destLV(), SLOT(slotApplyAllDifferences(bool)) );
	connect( m_models, SIGNAL(applyDifference(const Difference*, bool)),
	         m_diffView->destLV(), SLOT(slotApplyDifference(const Difference*, bool)) );

	// kompareactions
	m_kompareActions = new KompareActions( this, "KompareActions" );
	connect( m_models, SIGNAL(setSelection(const DiffModel*, const Difference*)),
	         m_kompareActions, SLOT(slotSetSelection(const DiffModel*, const Difference*)) );
	connect( m_models, SIGNAL(setSelection(const Difference*)),
	         m_kompareActions, SLOT(slotSetSelection(const Difference*)) );
	connect( m_kompareActions, SIGNAL(selectionChanged(const DiffModel*, const Difference*)),
	         m_models, SLOT(slotSelectionChanged(const DiffModel*, const Difference*)) );
	connect( m_kompareActions, SIGNAL(selectionChanged(const Difference*)),
	         m_models, SLOT(slotSelectionChanged(const Difference*)) );
	connect( m_models, SIGNAL(modelsChanged(const QPtrList<DiffModel>*)),
	         m_kompareActions, SLOT(slotModelsChanged(const QPtrList<DiffModel>*)) );
	connect( m_kompareActions, SIGNAL(applyDifference(bool)),
	         m_models, SLOT(slotApplyDifference(bool)) );
	connect( m_kompareActions, SIGNAL(applyAllDifferences(bool)),
	         m_models, SLOT(slotApplyAllDifferences(bool)) );
	connect( m_kompareActions, SIGNAL(previousModel()),
	         m_models, SLOT(slotPreviousModel()) );
	connect( m_kompareActions, SIGNAL(nextModel()),
	         m_models, SLOT(slotNextModel()) );
	connect( m_kompareActions, SIGNAL(previousDifference()),
	         m_models, SLOT(slotPreviousDifference()) );
	connect( m_kompareActions, SIGNAL(nextDifference()),
	         m_models, SLOT(slotNextDifference()) );

	// notify the part that this is our internal widget
	setWidget(m_diffView);

	setupActions();

	loadSettings( instance()->config() );

	// set our XML-UI resource file
	setXMLFile("komparepartui.rc");

	// we are read-write by default
	setReadWrite(true);

	// we are not modified since we haven't done anything yet
	setModified(false);

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
	m_saveAll->setEnabled  ( m_models->isModified() );
	m_saveDiff->setEnabled ( m_models->mode() == Kompare::ComparingFiles || m_models->mode() == Kompare::ComparingDirs );
	m_swap->setEnabled     ( m_models->mode() == Kompare::ComparingFiles || m_models->mode() == Kompare::ComparingDirs );
	m_diffStats->setEnabled( m_models->modelCount() > 0 );

	const DiffModel* model = m_models->selectedModel();
	if ( model )
		m_save->setEnabled( model->isModified() );
	else
		m_save->setEnabled( false );
}

bool KomparePart::openURL( const KURL& url )
{
	return openDiff( url );
}

void KomparePart::compare( const KURL& source, const KURL& destination )
{
	m_models->compare( source, destination );
}

bool KomparePart::saveDestination()
{
	const DiffModel* model = m_models->selectedModel();
	if ( model )
	{
		bool result = m_models->saveDestination( model );
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
	bool result = m_models->saveAll();
	updateActions();
	updateStatus();
	return result;
}

bool KomparePart::openDiff( const KURL& url )
{
	return m_models->openDiff( url );
}

void KomparePart::saveDiff()
{
	KDialogBase* dlg = new KDialogBase( widget(), "save options",
	                                    true /* modal */, i18n("Diff Options"),
	                                    KDialogBase::Ok|KDialogBase::Cancel );
	KompareSaveOptionsWidget* w = new KompareSaveOptionsWidget(
	                                             m_models->sourceTemp(),
	                                             m_models->destinationTemp(),
	                                             m_diffSettings, dlg );
	dlg->setMainWidget( w );
	dlg->setButtonOKText( i18n("Save") );

	if( dlg->exec() ) {
		w->saveOptions();
		KConfig* config = instance()->config();
		saveSettings( config );
		config->sync();
		KURL url = KFileDialog::getSaveURL( m_models->destinationBaseURL().url(),
		              i18n("*.diff *.dif *.patch|Patch files"), widget(), i18n( "Save .diff" ) );
		m_models->saveDiff( url, w->directory(), m_diffSettings );
	}
	delete dlg;
}

KURL KomparePart::diffURL()
{
	if( m_models->diffURL().isEmpty() ) {
		saveDiff();
	}
	return m_models->diffURL();
}

void KomparePart::slotSetStatus( enum Kompare::Status status )
{
	updateActions();
	switch( status ) {
	case Kompare::RunningDiff:
		emit setStatusBarText( i18n( "Running diff..." ) );
		break;
	case Kompare::Parsing:
		emit setStatusBarText( i18n( "Parsing diff..." ) );
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
	if( m_models->mode() == Kompare::ComparingFiles || m_models->mode() == Kompare::ComparingDirs )
	{
		if( modelCount() > 1 )
		{
			emit setStatusBarText( i18n( "Comparing files in %1 with files in %2" )
			   .arg( m_models->sourceBaseURL().prettyURL() )
			   .arg( m_models->destinationBaseURL().prettyURL() ) );
			emit setWindowCaption( m_models->sourceBaseURL().prettyURL()
			   + " : " + m_models->destinationBaseURL().prettyURL() );
		}
		else if ( modelCount() == 1 )
		{
			emit setStatusBarText( i18n( "Comparing %1 with %2" )
			   .arg( m_models->sourceBaseURL().prettyURL( 1 )
			   + m_models->modelAt( 0 )->srcFile() )
			   .arg( m_models->destinationBaseURL().prettyURL( 1 )
			   + m_models->modelAt( 0 )->destFile() ) );
			emit setWindowCaption(  m_models->modelAt( 0 )->srcFile()
			   + " : " + m_models->modelAt( 0 )->destFile() );
		}
	}
	else
	{
		emit setStatusBarText( i18n( "Viewing %1" ).arg( m_models->diffURL().prettyURL() ) );
		emit setWindowCaption( m_models->diffURL().filename() );
	}
}

void KomparePart::slotShowError( QString error )
{
	KMessageBox::error( widget(), error );
}

/*
void KomparePart::slotModelsChanged( const QPtrList<DiffModel>* models )
{
	m_selectedModel = 0;
	m_selectedDifference = 0;
}
*/

void KomparePart::slotSwap()
{
	m_models->swap();
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

	oldFile = m_models->selectedModel() ? m_models->selectedModel()->srcFile()  : QString::null;
	newFile = m_models->selectedModel() ? m_models->selectedModel()->destFile() : QString::null;
	if ( m_models->selectedModel() )
	{
		switch( m_models->format() ) {
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
		case Kompare::Unknown :
		default:
			diffFormat = i18n( "Unknown" );
			break;
		}
	}
	else
	{
		diffFormat = "";
	}

	filesInDiff = modelCount();

	noOfHunks = m_models->selectedModel() ? m_models->selectedModel()->hunkCount() : 0;
	noOfDiffs = m_models->selectedModel() ? m_models->selectedModel()->differenceCount() : 0;

	if ( modelCount() == 0 ) { // no diff loaded yet
		KMessageBox::information( 0L, i18n(
		    "No diff file, or no 2 files have been diffed. "
		    "Therefore no stats are available."),
		    i18n("Diff Statistics"), QString::null, false );
	}
	else if ( modelCount() == 1 ) { // 1 file in diff, or 2 files compared
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
	} else { // more than 1 file in diff, or 1 or more directories compared (not yet possible afaik))
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

bool KomparePart::askSaveChanges()
{
	if( !isModified() ) return true;

	int query = KMessageBox::warningYesNoCancel( widget(),
	    i18n("You have made changes to the destination.\n"
	         "Would you like to save them?" ),
	         i18n( "Save Changes?" ),
	         i18n( "Save" ),
	         i18n( "Discard" ) );

	if( query == KMessageBox::Cancel ) return false;
	if( query == KMessageBox::Yes ) return m_models->saveAll();
	return true;
}

int KomparePart::selectedModelIndex()
{
	return m_models->selectedModelIndex();
}

int KomparePart::selectedDifferenceIndex()
{
	return m_models->selectedDifferenceIndex();
}

int KomparePart::differenceCount()
{
	const DiffModel* model = m_models->selectedModel();
	if ( model )
		return model->differenceCount();
	else
		return -1;
}

int KomparePart::appliedCount()
{
	const DiffModel* model = m_models->selectedModel();
	if ( model )
		return model->appliedCount();
	else
		return -1;
}

void KomparePart::loadSettings(KConfig *config)
{
	config->setGroup( "General" );
	m_generalSettings->loadSettings( config );
	config->setGroup( "DiffSettings" );
	m_diffSettings->loadSettings   ( config );
	config->setGroup( "MiscSettings" );
	m_miscSettings->loadSettings   ( config );
}

void KomparePart::saveSettings(KConfig *config)
{
	config->setGroup( "General" );
	m_generalSettings->saveSettings( config );
	config->setGroup( "DiffSettings" );
	m_diffSettings->saveSettings   ( config );
	config->setGroup( "MiscSettings" );
	m_miscSettings->saveSettings   ( config );
}

/*
void KomparePart::slotSetSelection( const DiffModel* model, const Difference* diff )
{
	if( model == m_selectedModel && diff == m_selectedDifference )
		return;

	if( m_selectedModel ) {
		disconnect( m_selectedModel, SIGNAL(appliedChanged( const Difference* )),
		            this, SLOT(slotAppliedChanged( const Difference* )) );
	}

	m_selectedModel = model;
	m_selectedDifference = diff;

	if( m_selectedModel ) {
		connect( m_selectedModel, SIGNAL(appliedChanged( const Difference* )),
		         this, SLOT(slotAppliedChanged( const Difference* )) );
	}

	emit selectionChanged( model, diff );
}
*/

/*
void KomparePart::slotSelectionChanged( int model, int diff )
{
	updateActions();
}
*/

/*
void KomparePart::slotAppliedChanged( const Difference* diff )
{
	updateActions();
	updateStatus();
}
*/

void KomparePart::optionsPreferences()
{
	// show preferences
	KomparePrefDlg* pref = new KomparePrefDlg( m_generalSettings, m_diffSettings, m_miscSettings );

	if ( pref->exec() ) {
		KConfig* config = instance()->config();
		saveSettings( config );
		config->sync();
	}
};

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
		s_about = new KAboutData("komparepart", I18N_NOOP("KomparePart"), "2.0");
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
};

#include "kompare_part.moc"
