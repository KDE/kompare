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
#include "kdiffstatsdlg.h"

GeneralSettings* KDiffPart::m_generalSettings = 0L;
DiffSettings*    KDiffPart::m_diffSettings = 0L;
MiscSettings*    KDiffPart::m_miscSettings = 0L;

KDiffPart::KDiffPart( QWidget *parentWidget, const char *widgetName,
                                  QObject *parent, const char *name )
	: KParts::ReadWritePart(parent, name),
	m_selectedModel( -1 ),
	m_selectedDifference( -1 ),
	m_navigationTree( 0 )
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

	m_save = KStdAction::save( this, SLOT(slotSaveDestination()), actionCollection() );
	m_saveDiff = new KAction( i18n("Save .&diff"), 0,
	              this, SLOT(save()),
	              actionCollection(), "file_save_diff" );
	m_diffStats = new KAction( i18n( "Show diff stats" ), 0,
	              this, SLOT(slotShowDiffstats()),
	              actionCollection(), "file_diffstats" );
	m_applyDifference = new KAction( i18n("&Apply Difference"), "1rightarrow", Qt::Key_Space,
	              this, SLOT(slotApplyDifference()),
	              actionCollection(), "difference_apply" );
	m_applyAll = new KAction( i18n("App&ly all"), "2rightarrow", Qt::CTRL + Qt::Key_A,
	              this, SLOT(slotApplyAllDifferences()),
	              actionCollection(), "difference_applyall" );
	m_unapplyAll = new KAction( i18n("U&napply all"), "2leftarrow", Qt::CTRL + Qt::Key_U,
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
		setModified( appliedCount() > 0 );
	} else {
		m_applyDifference->setEnabled( false );
		setModified( false );
	}
	m_previousFile->setEnabled( model > 0 );
	m_nextFile->setEnabled( model < m_models->modelCount() - 1 );
	m_previousDifference->setEnabled( diff > 0 || model > 0 );
	m_nextDifference->setEnabled( diff < getSelectedModel()->differenceCount() - 1 ||
	                              model < m_models->modelCount() - 1 );
}

void KDiffPart::setReadWrite(bool rw)
{
	// notify your internal widget of the read-write state
	ReadWritePart::setReadWrite(rw);
}

void KDiffPart::setModified(bool modified)
{
	// get a handle on our Save action and make sure it is valid
	// KAction *save = actionCollection()->action(KStdAction::stdName(KStdAction::Save));
	if (!m_save)
		return;

	// if so, we either enable or disable it based on the current
	// state
	if (modified)
		m_save->setEnabled(true);
	else
		m_save->setEnabled(false);

	// in any event, we want our parent to do it's thing
	ReadWritePart::setModified(modified);
}

void KDiffPart::compare( const KURL& source, const KURL& destination )
{
	m_models->compare( source, destination );
}

bool KDiffPart::openFile()
{
	QFile file(m_file);

	if( !file.open(IO_ReadOnly) )
		return false;
	
	m_models->readDiffFile( file );
	
	return true;
}

bool KDiffPart::save()
{
	// Don't write to url yet, we must wait until the diff is finished
	return saveFile();
}

bool KDiffPart::saveFile()
{
	// if we aren't read-write, return immediately
	if (isReadWrite() == false)
		return false;

	if ( m_file.isEmpty() ) {
		KURL url = KFileDialog::getSaveURL( QString::null, "*.diff", widget(), "FileSaveDialog" );
		if ( !url.isEmpty() ) {
			return saveAs( url );
		}
		return false;
	}

	m_models->writeDiffFile( m_file, m_diffSettings );

	return true;
}

void KDiffPart::slotSetStatus( KDiffModelList::Status status )
{
	switch( status ) {
	case KDiffModelList::RunningDiff:
		emit setStatusBarText( i18n( "Running diff..." ) );
		break;
	case KDiffModelList::Parsing:
		emit setStatusBarText( i18n( "Parsing diff..." ) );
		break;
	case KDiffModelList::FinishedParsing:
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
			m_saveDiff->setEnabled(true);
		} else {
			emit setStatusBarText( i18n( "Viewing %1" ).arg( url().prettyURL() ) );
		}
		slotSetSelection( 0, 0 );
		break;
	case KDiffModelList::FinishedWritingDiff:
		saveToURL();
		break;
	default:
		break;
	}
}

void KDiffPart::slotShowError( QString error )
{
	KMessageBox::error( widget(), error );
}

void KDiffPart::slotSaveDestination()
{
	m_models->saveDestination( m_selectedModel );
}

void KDiffPart::slotShowDiffstats( void )
{
	// Fetch all the args needed for kdiffstatsdlg
	// oldfile, newfile, diffformat, noofhunks, noofdiffs

	QString oldFile;
	QString newFile;
	QString diffFormat;
	int noOfHunks;
	int noOfDiffs;

	oldFile = "";
	newFile = "";
	diffFormat = "";
	noOfHunks = 0;
	noOfDiffs = 0;

kdDebug() << "Create dialog" << endl;
	KDiffStatsDlg* diffStatsDlg = new KDiffStatsDlg( oldFile, newFile, diffFormat, noOfHunks, noOfDiffs );

kdDebug() << "Execute dialog" << endl;
	diffStatsDlg->exec();

kdDebug() << "Delete dialog" << endl;
	delete diffStatsDlg;
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

	disconnect( getSelectedModel(), SIGNAL(appliedChanged( const Difference* )),
	            this, SLOT(slotAppliedChanged( const Difference* )) );
	
	m_selectedModel = model;
	m_selectedDifference = diff;

	connect( getSelectedModel(), SIGNAL(appliedChanged( const Difference* )),
	            this, SLOT(slotAppliedChanged( const Difference* )) );
	
	emit selectionChanged( model, diff );
}

void KDiffPart::slotSelectionChanged( int /* model */, int /* diff */ )
{
	updateActions();
}

void KDiffPart::slotAppliedChanged( const Difference* /* d */ )
{
	updateActions();
	emit appliedChanged();
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
