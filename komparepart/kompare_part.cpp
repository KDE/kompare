/***************************************************************************
                                kdiff_part.cpp  -  description
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

#include "kdiff_part.h"

#include <kinstance.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <klistview.h>
#include <kdialogbase.h>

#include <qfile.h>

#include "kdiffview.h"
#include "kdiffnavigationtree.h"
#include "kdiff_actions.h"
#include "diffmodel.h"
#include "kdiffprocess.h"
#include "kdiffprefdlg.h"
#include "generalsettings.h"
#include "diffsettings.h"
#include "miscsettings.h"
#include "kdiffsaveoptionswidget.h"

GeneralSettings* KDiffPart::m_generalSettings = 0L;
DiffSettings*    KDiffPart::m_diffSettings = 0L;
MiscSettings*    KDiffPart::m_miscSettings = 0L;

KDiffPart::KDiffPart( QWidget *parentWidget, const char *widgetName,
                                  QObject *parent, const char *name )
	: KParts::ReadWritePart(parent, name),
	m_selectedModel( -1 ),
	m_selectedDifference( -1 ),
	m_navigationTree( 0 ),
	m_tempDiff( 0 )
{
	// we need an instance
	setInstance( KDiffPartFactory::instance() );
	
	if( !m_generalSettings ) {
		m_generalSettings = new GeneralSettings( 0 );
		m_diffSettings = new DiffSettings( 0 );
		m_miscSettings = new MiscSettings( 0 );
	}
	
	m_models = new KDiffModelList();
	connect( m_models, SIGNAL(status( KDiffModelList::Status )),
	         this, SLOT(slotSetStatus( KDiffModelList::Status )) );
	connect( m_models, SIGNAL(error( QString )),
	         this, SLOT(slotShowError( QString )) );
	connect( m_models, SIGNAL(modelsChanged()),
	         this, SLOT(slotModelsChanged()) );
	
	// this should be your custom internal widget
	m_diffView = new KDiffView( m_models, m_generalSettings, parentWidget, widgetName );
	connect( this, SIGNAL(selectionChanged(int,int)),
	         m_diffView, SLOT(slotSetSelection(int,int)) );
	connect( m_diffView, SIGNAL(selectionChanged(int,int)),
	         this, SLOT(slotSetSelection(int,int)) );

	// notify the part that this is our internal widget
	setWidget(m_diffView);
	
	setupActions();
	
	loadSettings( instance()->config() );
	
	// set our XML-UI resource file
	setXMLFile("kdiffpartui.rc");
	
	// we are read-write by default
	setReadWrite(true);
	
	// we are not modified since we haven't done anything yet
	setModified(false);
	
	connect( this, SIGNAL(selectionChanged(int,int)),
	         this, SLOT(slotSelectionChanged(int,int)) );
}

KDiffPart::~KDiffPart()
{
}

QWidget* KDiffPart::createNavigationWidget( QWidget* parent, const char* name )
{
	if( !m_navigationTree ) {
		m_navigationTree = new KDiffNavigationTree( m_models, parent, name );
		connect( this, SIGNAL(selectionChanged(int,int)),
		         m_navigationTree, SLOT(slotSetSelection(int,int)) );
		connect( m_navigationTree, SIGNAL(selectionChanged(int,int)),
		         this, SLOT(slotSetSelection(int,int)) );
	} else {
		// FIXME
	}

	return m_navigationTree;
}

void KDiffPart::setupActions()
{
	// create our actions

	m_save = KStdAction::save( this, SLOT(saveDestination()), actionCollection() );
	m_saveAll = new KAction( i18n("Save &All"), "save_all", 0,
	              this, SLOT(saveAll()),
	              actionCollection(), "file_save_all" );
	m_saveDiff = new KAction( i18n("Save .&diff"), 0,
	              this, SLOT(saveDiff()),
	              actionCollection(), "file_save_diff" );
	m_swap = new KAction( i18n( "Swap Source and Destination" ), 0,
	              this, SLOT(slotSwap()),
	              actionCollection(), "file_swap" );
	m_diffStats = new KAction( i18n( "Show Statistics" ), 0,
	              this, SLOT(slotShowDiffstats()),
	              actionCollection(), "file_diffstats" );
	m_applyDifference = new KAction( i18n("&Apply Difference"), "1rightarrow", Qt::Key_Space,
	              this, SLOT(slotApplyDifference()),
	              actionCollection(), "difference_apply" );
	m_applyAll = new KAction( i18n("App&ly All"), "2rightarrow", Qt::CTRL + Qt::Key_A,
	              this, SLOT(slotApplyAllDifferences()),
	              actionCollection(), "difference_applyall" );
	m_unapplyAll = new KAction( i18n("U&napply All"), "2leftarrow", Qt::CTRL + Qt::Key_U,
	              this, SLOT(slotUnapplyAllDifferences()),
	              actionCollection(), "difference_unapplyall" );
	m_previousFile = new KAction( i18n("P&revious File"), "2uparrow", Qt::CTRL + Qt::Key_PageUp,
	              this, SLOT(slotPreviousFile()),
	              actionCollection(), "difference_previousfile" );
	m_nextFile = new KAction( i18n("N&ext File"), "2downarrow", Qt::CTRL + Qt::Key_PageDown,
	              this, SLOT(slotNextFile()),
	              actionCollection(), "difference_nextfile" );
	m_previousDifference = new KAction( i18n("&Previous Difference"), "1uparrow", Qt::CTRL + Qt::Key_Up,
	              this, SLOT(slotPreviousDifference()),
	              actionCollection(), "difference_previous" );
	m_previousDifference->setEnabled( false );
	m_nextDifference = new KAction( i18n("&Next Difference"), "1downarrow", Qt::CTRL + Qt::Key_Down,
	              this, SLOT(slotNextDifference()),
	              actionCollection(), "difference_next" );
	m_nextDifference->setEnabled( false );
	m_differences = new KDifferencesAction( i18n("Differences"), actionCollection(), "difference_differences" );
	connect( m_differences, SIGNAL( menuAboutToShow() ), this, SLOT( slotDifferenceMenuAboutToShow() ) );
	connect( m_differences, SIGNAL( activated( int ) ), this, SLOT( slotGoDifferenceActivated( int ) ) );

	KStdAction::preferences(this, SLOT(optionsPreferences()), actionCollection());
}

void KDiffPart::updateActions()
{
	m_saveAll->setEnabled( m_models->isModified() );
	m_saveDiff->setEnabled( m_models->mode() == KDiffModelList::Compare );
	m_swap->setEnabled( m_models->mode() == KDiffModelList::Compare );
	m_diffStats->setEnabled( m_models->modelCount() > 0 );
	
	int model = getSelectedModelIndex();
	int diff = getSelectedDifferenceIndex();
	if( model >= 0 ) {
		if( diff >= 0 ) {
			m_applyDifference->setEnabled( true );
			const Difference* d = getSelectedDifference();
			if( d->applied() ) {
				m_applyDifference->setText( i18n( "Un&apply Difference" ) );
				m_applyDifference->setIcon( "1leftarrow" );
			} else {
				m_applyDifference->setText( i18n( "&Apply Difference" ) );
				m_applyDifference->setIcon( "1rightarrow" );
			}
		}
		m_save->setEnabled( getSelectedModel()->isModified() );
	} else {
		m_save->setEnabled( false );
	}
	m_previousFile->setEnabled( model > 0 );
	m_nextFile->setEnabled( model < m_models->modelCount() - 1 );
	m_previousDifference->setEnabled( diff > 0 || model > 0 );
	m_nextDifference->setEnabled( model >= 0 &&
	    ( diff < getSelectedModel()->differenceCount() - 1 || model < m_models->modelCount() - 1 ) );
}

bool KDiffPart::openURL( const KURL& url )
{
  return openDiff( url );
}

void KDiffPart::compare( const KURL& source, const KURL& destination )
{
	m_models->compare( source, destination );
}

bool KDiffPart::saveDestination()
{
	bool result = m_models->saveDestination( m_selectedModel );
	updateActions();
	updateStatus();
	return result;
}

bool KDiffPart::saveAll()
{
	bool result = m_models->saveAll();
	updateActions();
	updateStatus();
	return result;
}

bool KDiffPart::openDiff( const KURL& url )
{
	return m_models->openDiff( url );
}

void KDiffPart::saveDiff()
{
	KDialogBase* dlg = new KDialogBase( widget(), "save options",
	    true /* modal */, i18n("Diff Options"), KDialogBase::Ok|KDialogBase::Cancel );
	KDiffSaveOptionsWidget* w = new KDiffSaveOptionsWidget(
	    m_models->sourceTemp(), m_models->destinationTemp(), m_diffSettings, dlg );
	dlg->setMainWidget( w );
	dlg->setButtonOKText( i18n("Save") );
	
	if( dlg->exec() ) {
		w->saveOptions();
		KConfig* config = instance()->config();
		saveSettings( config );
		config->sync();
		KURL url = KFileDialog::getSaveURL( m_models->destinationBaseURL().url(),
		              "*.diff *.patch", widget(), i18n( "Save .diff" ) );
		m_models->saveDiff( url, w->directory(), m_diffSettings );
	}
	delete dlg;
}

KURL KDiffPart::diffURL()
{
	if( m_models->diffURL().isEmpty() ) {
		saveDiff();
	}
	return m_models->diffURL();
}

void KDiffPart::slotSetStatus( KDiffModelList::Status status )
{
	updateActions();
	switch( status ) {
	case KDiffModelList::RunningDiff:
		emit setStatusBarText( i18n( "Running diff..." ) );
		break;
	case KDiffModelList::Parsing:
		emit setStatusBarText( i18n( "Parsing diff..." ) );
		break;
	case KDiffModelList::FinishedParsing:
		updateStatus();
		if( m_models->modelCount() > 0 && m_models->modelAt( 0 )->differenceCount() > 0 )
			slotSetSelection( 0, 0 );
		break;
	case KDiffModelList::FinishedWritingDiff:
		updateStatus();
		emit diffURLChanged();
		break;
	default:
		break;
	}
}

void KDiffPart::updateStatus()
{
	if( m_models->mode() == KDiffModelList::Compare ) {
		if( modelCount() > 1 ) {
			emit setStatusBarText( i18n( "Comparing files in %1 with files in %2" )
			   .arg( m_models->sourceBaseURL().prettyURL() )
			   .arg( m_models->destinationBaseURL().prettyURL() ) );
			emit setWindowCaption( m_models->sourceBaseURL().prettyURL()
			   + " : " + m_models->destinationBaseURL().prettyURL() );
		} else if ( modelCount() == 1 ) {
			emit setStatusBarText( i18n( "Comparing %1 with %2" )
			   .arg( m_models->sourceBaseURL().prettyURL( 1 )
			   + m_models->modelAt( 0 )->sourceFile() )
			   .arg( m_models->destinationBaseURL().prettyURL( 1 )
			   + m_models->modelAt( 0 )->destinationFile() ) );
			emit setWindowCaption(  m_models->modelAt( 0 )->sourceFile()
			   + " : " + m_models->modelAt( 0 )->destinationFile() );
		}
	} else {
		emit setStatusBarText( i18n( "Viewing %1" ).arg( url().prettyURL() ) );
		emit setWindowCaption( url().filename() );
	}
}

void KDiffPart::slotShowError( QString error )
{
	KMessageBox::error( widget(), error );
}

void KDiffPart::slotModelsChanged()
{
	if( m_selectedModel > modelCount() - 1 )
		slotSetSelection( -1, -1 );
}

void KDiffPart::slotSwap()
{
	m_models->swap();
}

void KDiffPart::slotShowDiffstats( void )
{
	// Fetch all the args needed for kdiffstatsmessagebox
	// oldfile, newfile, diffformat, noofhunks, noofdiffs

	QString oldFile;
	QString newFile;
	QString diffFormat;
	int filesInDiff;
	int noOfHunks;
	int noOfDiffs;

	oldFile = getSelectedModel() ? getSelectedModel()->sourceFile() : QString::null;
	newFile = getSelectedModel() ? getSelectedModel()->destinationFile() : QString::null;
	if ( getSelectedModel() )
	{
		switch( getSelectedModel()->getFormat() ) {
		case Unified :
			diffFormat = i18n( "Unified" );
			break;
		case Context :
			diffFormat = i18n( "Context" );
			break;
		case RCS :
			diffFormat = i18n( "RCS" );
			break;
		case Ed :
			diffFormat = i18n( "Ed" );
			break;
		case Normal :
			diffFormat = i18n( "Normal" );
			break;
		case Unknown :
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

	noOfHunks = getSelectedModel() ? getSelectedModel()->hunkCount() : 0;
	noOfDiffs = getSelectedModel() ? getSelectedModel()->differenceCount() : 0;

	if ( modelCount() == 0 ) { // no diff loaded yet
		KMessageBox::information( 0L, i18n(
		    "Sorry, no diff file or no 2 files have been diffed, "
		    "so there are no stats available."),
		    i18n("Diff statistics"), QString::null, false );
	}
	else if ( modelCount() == 1 ) { // 1 file in diff, or 2 files compared
		KMessageBox::information( 0L, i18n(
		    "Statistics:\n"
		    "\n"
		    "Old file : %1\n"
		    "New file : %2\n"
		    "\n"
		    "Format : %3\n"
		    "Number of hunks : %4\n"
		    "Number of differences : %5")
		    .arg(oldFile).arg(newFile).arg(diffFormat)
		    .arg(noOfHunks).arg(noOfDiffs),
		    i18n("Diff statistics"), QString::null, false );
	} else { // more than 1 file in diff, or 1 or more directories compared (not yet possible afaik))
		KMessageBox::information( 0L, i18n(
		    "Statistics:\n"
		    "\n"
		    "Number of files in diff file: %1\n"
		    "Format : %2\n"
		    "\n"
		    "Current old file : %3\n"
		    "Current new file : %4\n"
		    "\n"
		    "Number of hunks : %5\n"
		    "Number of differences : %6")
		    .arg(filesInDiff).arg(diffFormat).arg(oldFile)
		    .arg(newFile).arg(noOfHunks).arg(noOfDiffs),
		    i18n("Diff statistics"), QString::null, false );
	}
}

bool KDiffPart::askSaveChanges()
{
	if( !isModified() ) return true;
	
	int query = KMessageBox::warningYesNoCancel( widget(),
	    i18n("You have made changes to the destination.\n"
	         "Would you like to save them?" ), i18n( "Save Changes?" ),
	         i18n( "Save" ), i18n( "Discard" ) );
	
	if( query == KMessageBox::Cancel ) return false;
	if( query == KMessageBox::Yes ) return m_models->saveAll();
	return true;
}

void KDiffPart::loadSettings(KConfig *config)
{
	config->setGroup( "General" );
	m_generalSettings->loadSettings( config );
	config->setGroup( "DiffSettings" );
	m_diffSettings->loadSettings( config );
	config->setGroup( "MiscSettings" );
	m_miscSettings->loadSettings( config );
}

void KDiffPart::saveSettings(KConfig *config)
{
	config->setGroup( "General" );
	m_generalSettings->saveSettings( config );
	config->setGroup( "DiffSettings" );
	m_diffSettings->saveSettings( config );
	config->setGroup( "MiscSettings" );
	m_miscSettings->saveSettings( config );
}

void KDiffPart::slotSetSelection( int model, int diff )
{
	if( model == m_selectedModel && diff == m_selectedDifference )
		return;

	if( m_selectedModel >= 0 ) {
		disconnect( getSelectedModel(), SIGNAL(appliedChanged( const Difference* )),
		            this, SLOT(slotAppliedChanged( const Difference* )) );
	}
	
	m_selectedModel = model;
	m_selectedDifference = diff;

	if( m_selectedModel >= 0 ) {
		connect( getSelectedModel(), SIGNAL(appliedChanged( const Difference* )),
		            this, SLOT(slotAppliedChanged( const Difference* )) );
	}
	
	emit selectionChanged( model, diff );
}

void KDiffPart::slotSelectionChanged( int /* model */, int /* diff */ )
{
	updateActions();
}

void KDiffPart::slotAppliedChanged( const Difference* /* d */ )
{
	updateActions();
	updateStatus();
}

void KDiffPart::slotDifferenceMenuAboutToShow()
{
	m_differences->fillDifferenceMenu( getSelectedModel(), getSelectedDifferenceIndex() );
}

void KDiffPart::slotGoDifferenceActivated( int item )
{
	slotSetSelection( getSelectedModelIndex(), item );
}

void KDiffPart::slotApplyDifference()
{
	getSelectedModel()->toggleApplied( getSelectedDifferenceIndex() );
	if( m_nextDifference->isEnabled() )
		slotNextDifference();
}

void KDiffPart::slotApplyAllDifferences()
{
	DiffModel* model = getSelectedModel();
	QListIterator<Difference> it = QListIterator<Difference>(model->getDifferences());
	int i = 0;
	while ( it.current() ) {
		if( !(*it)->applied() )
			model->toggleApplied( i );
		i++; ++it;
	}
}

void KDiffPart::slotUnapplyAllDifferences()
{
	DiffModel* model = getSelectedModel();
	QListIterator<Difference> it = QListIterator<Difference>(model->getDifferences());
	int i = 0;
	while ( it.current() ) {
		if( (*it)->applied() )
			model->toggleApplied( i );
		i++; ++it;
	}
}

void KDiffPart::slotPreviousFile()
{
	int modelIndex = getSelectedModelIndex();
	slotSetSelection( modelIndex - 1, 0 );
}

void KDiffPart::slotNextFile()
{
	int modelIndex = getSelectedModelIndex();
	slotSetSelection( modelIndex + 1, 0 );
}

void KDiffPart::slotPreviousDifference()
{
	int modelIndex = getSelectedModelIndex();
	int diffIndex = getSelectedDifferenceIndex();
	if( diffIndex > 0 )
		slotSetSelection( modelIndex, diffIndex - 1 );
	else
		slotSetSelection( modelIndex - 1, m_models->modelAt( modelIndex - 1 )->differenceCount() - 1 );
}

void KDiffPart::slotNextDifference()
{
	int modelIndex = getSelectedModelIndex();
	int diffIndex = getSelectedDifferenceIndex();
	if( diffIndex < getSelectedModel()->differenceCount() - 1 )
		slotSetSelection( getSelectedModelIndex(), getSelectedDifferenceIndex() + 1 );
	else
		slotSetSelection( modelIndex + 1, 0 );
}

void KDiffPart::optionsPreferences()
{
	// show preferences
	KDiffPrefDlg* pref = new KDiffPrefDlg( m_generalSettings, m_diffSettings, m_miscSettings );
	
	if ( pref->exec() ) {
		KConfig* config = instance()->config();
		saveSettings( config );
		config->sync();
	}
};

// It's usually safe to leave the factory code alone.. with the
// notable exception of the KAboutData data
#include <kaboutdata.h>
#include <klocale.h>

KInstance*  KDiffPartFactory::s_instance = 0L;
KAboutData* KDiffPartFactory::s_about = 0L;

KDiffPartFactory::KDiffPartFactory()
    : KParts::Factory()
{
}

KDiffPartFactory::~KDiffPartFactory()
{
	delete s_instance;
	delete s_about;

	s_instance = 0L;
}

KParts::Part* KDiffPartFactory::createPartObject( QWidget *parentWidget, const char *widgetName,
                                                        QObject *parent, const char *name,
                                                        const char *classname, const QStringList & /*args*/ )
{
	// Create an instance of our Part
	KDiffPart* obj = new KDiffPart( parentWidget, widgetName, parent, name );

	// See if we are to be read-write or not
	if (QCString(classname) == "KParts::ReadOnlyPart")
		obj->setReadWrite(false);

	return obj;
}

KInstance* KDiffPartFactory::instance()
{
	if( !s_instance )
	{
		s_about = new KAboutData("kdiffpart", I18N_NOOP("KDiffPart"), "2.0");
		s_about->addAuthor("John Firebaugh", "Author", "jfirebaugh@kde.org");
		s_about->addAuthor("Otto Bruggeman", "Author", "otto.bruggeman@home.nl" );
		s_instance = new KInstance(s_about);
	}
	return s_instance;
}

extern "C"
{
	void* init_libkdiffpart()
	{
		return new KDiffPartFactory;
	}
};

#include "kdiff_part.moc"
