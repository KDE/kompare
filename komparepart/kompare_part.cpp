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

#include <qfile.h>
#include <qtextstream.h>

#include "kdiffview.h"
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
    : KParts::ReadWritePart(parent, name)
{
	// we need an instance
	setInstance( KDiffPartFactory::instance() );

	if( !m_generalSettings ) {
		m_generalSettings = new GeneralSettings( 0 );
		m_diffSettings = new DiffSettings( 0 );
		m_miscSettings = new MiscSettings( 0 );
	}

	// this should be your custom internal widget
	m_diffView = new KDiffView( m_generalSettings, parentWidget, widgetName );

	// notify the part that this is our internal widget
	setWidget(m_diffView);

	setupActions();

	connect( m_diffView, SIGNAL(itemsChanged()), this, SLOT(slotItemsChanged()) );
	connect( m_diffView, SIGNAL(selectionChanged()), this, SLOT(slotSelectionChanged()));
	slotItemsChanged();

	loadSettings( instance()->config() );
	toggleSynchronize();

	// set our XML-UI resource file
	setXMLFile("kdiffpartui.rc");

	// we are read-write by default
	setReadWrite(true);

	// we are not modified since we haven't done anything yet
	setModified(false);

	m_diffProcess = 0;
	m_leftURL = 0;
	m_rightURL = 0;
}

KDiffPart::~KDiffPart()
{
	delete m_diffProcess;
	delete m_leftURL;
	delete m_rightURL;
}

KDiffView* KDiffPart::diffView()
{
	return m_diffView;
}

void KDiffPart::setupActions()
{
	// create our actions

	m_saveDiff = KStdAction::save( this, SLOT(save()), actionCollection() );
	m_saveDiff->setText( i18n("&Save .diff") );
	m_diffStats = new KAction( i18n( "Show diff stats" ), "", 0,
	              this, SLOT(slotShowDiffstats()),
	              actionCollection(), "file_diffstats" );
	m_previousDifference = new KAction( i18n("&Previous Difference"), "previous", Qt::CTRL + Qt::Key_Up,
	              this, SLOT(slotPreviousDifference()),
	              actionCollection(), "difference_previous" );
	m_nextDifference = new KAction( i18n("&Next Difference"), "next", Qt::CTRL + Qt::Key_Down,
	              this, SLOT(slotNextDifference()),
	              actionCollection(), "difference_next" );
	m_differences = new KDifferencesAction( i18n("Differences"), actionCollection(), "difference_differences" );
	connect( m_differences, SIGNAL( menuAboutToShow() ), this, SLOT( slotDifferenceMenuAboutToShow() ) );
	connect( m_differences, SIGNAL( activated( int ) ), this, SLOT( slotGoDifferenceActivated( int ) ) );

	m_synchronizeScrollBars = new KToggleAction( i18n("S&ynchronize Scroll Bars"),
	              0, this, SLOT(toggleSynchronize()),
	              actionCollection(), "settings_synchronize_scroll_bars" );

	KStdAction::preferences(this, SLOT(optionsPreferences()), actionCollection());
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
	if (!m_saveDiff)
		return;

	// if so, we either enable or disable it based on the current
	// state
	if (modified)
		m_saveDiff->setEnabled(true);
	else
		m_saveDiff->setEnabled(false);

	// in any event, we want our parent to do it's thing
	ReadWritePart::setModified(modified);
}

void KDiffPart::setFormat( QCString format )
{
	// This format should also be set in the m_diffSettings
	if ( format == "CONTEXT" )
	{
		kdDebug() << "Context format" << endl;
		m_format = DiffModel::Context;
		m_diffSettings->m_useContextDiff = true;
		m_diffSettings->m_useEdDiff = false;
		m_diffSettings->m_useNormalDiff = false;
		m_diffSettings->m_useRCSDiff = false;
		m_diffSettings->m_useUnifiedDiff = false;
	}
	else if ( format == "ED" )
	{
		kdDebug() << "Ed format" << endl;
		m_format = DiffModel::Ed;
		m_diffSettings->m_useContextDiff = false;
		m_diffSettings->m_useEdDiff = true;
		m_diffSettings->m_useNormalDiff = false;
		m_diffSettings->m_useRCSDiff = false;
		m_diffSettings->m_useUnifiedDiff = false;
	}
	else if ( format == "NORMAL" )
	{
		kdDebug() << "Normal format" << endl;
		m_format = DiffModel::Normal;
		m_diffSettings->m_useContextDiff = false;
		m_diffSettings->m_useEdDiff = false;
		m_diffSettings->m_useNormalDiff = true;
		m_diffSettings->m_useRCSDiff = false;
		m_diffSettings->m_useUnifiedDiff = false;
	}
	else if ( format == "RCS" )
	{
		kdDebug() << "RCS format" << endl;
		m_format = DiffModel::RCS;
		m_diffSettings->m_useContextDiff = false;
		m_diffSettings->m_useEdDiff = false;
		m_diffSettings->m_useNormalDiff = false;
		m_diffSettings->m_useRCSDiff = true;
		m_diffSettings->m_useUnifiedDiff = false;
	}
	else if ( format == "UNIFIED" )
	{
		kdDebug() << "Unified format" << endl;
		m_format = DiffModel::Unified;
		m_diffSettings->m_useContextDiff = false;
		m_diffSettings->m_useEdDiff = false;
		m_diffSettings->m_useNormalDiff = false;
		m_diffSettings->m_useRCSDiff = false;
		m_diffSettings->m_useUnifiedDiff = true;
	}
}

void KDiffPart::compare( const KURL& source, const KURL& destination, DiffSettings* settings )
{
	if( settings )
		m_diffProcess = new KDiffProcess( settings );
	else
		m_diffProcess = new KDiffProcess( m_diffSettings );
	m_leftURL = new KURL( source );
	m_rightURL = new KURL( destination );
	m_diffProcess->setLeftURL( m_leftURL );
	m_diffProcess->setRightURL( m_rightURL );
	connect( m_diffProcess, SIGNAL(diffHasFinished( bool )), this, SLOT(slotDiffProcessFinished( bool )) );
	kdDebug() << "starting diff process" << endl;
	m_diffProcess->start();
}

void KDiffPart::slotDiffProcessFinished( bool success )
{
	DiffModel::DiffFormat type;

	if( success ) {
		kdDebug() << "diff process finished" << endl;
		DiffModel* model = new DiffModel();

		if      ( m_diffSettings->m_useContextDiff ) type = DiffModel::Context;
		else if ( m_diffSettings->m_useEdDiff )      type = DiffModel::Ed;
		else if ( m_diffSettings->m_useNormalDiff )  type = DiffModel::Normal;
		else if ( m_diffSettings->m_useRCSDiff )     type = DiffModel::RCS;
		else if ( m_diffSettings->m_useUnifiedDiff ) type = DiffModel::Unified;
		else                                         type = DiffModel::Unknown;

		m_diffOutput = m_diffProcess->getDiffOutput();	
		if ( model->parseDiff( m_diffOutput, type ) ) // XXX Fixme, is it fixed now ? :)
		{
			// error, do something
			KMessageBox::error( widget(), i18n( "Error during parsing... gotta abort" ) );
			delete model;
			delete m_diffProcess;
			m_diffProcess = 0;
			return;
		}
		m_diffmodels.append( model );
		m_diffView->setModel( model, false );
		setModified( true );
	// Hmmm m_diffProcess->m_diffProcess->exitStatus ? Isn't that one
	// m_diffProcess too much ???
	} else if( m_diffProcess->m_diffProcess->exitStatus() == 0 ) {
		KMessageBox::information( widget(), i18n( "The files are identical." ) );
	} else {
		KMessageBox::error( widget(), m_diffProcess->getStderr() );
	}

	delete m_diffProcess;
	m_diffProcess = 0;
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

	KDiffStatsDlg* diffStatsDlg = new KDiffStatsDlg( oldFile, newFile, diffFormat, noOfHunks, noOfDiffs );

	diffStatsDlg->exec();

	delete diffStatsDlg;
}

bool KDiffPart::openFile()
{
	// m_file is always local so we can use QFile on it
	QFile file(m_file);

	if (file.open(IO_ReadOnly) == false)
		return 1;

	QTextStream stream(&file);
	QStringList list;
	while (!stream.eof())
		list.append(stream.readLine());

	DiffModel* model = new DiffModel();

	if ( model->parseDiff( list ) != 0 )
	{
		KMessageBox::error( widget(), i18n( "Could not parse diff file." ) );
		delete model;
		return false;
	}

	m_diffmodels.append( model );

	m_diffView->setModel( model, false ); // XXX TODO handle multi files

	return true;
}

bool KDiffPart::saveFile()
{
	// if we aren't read-write, return immediately
	if (isReadWrite() == false)
		return false;

	if ( m_file.isEmpty() )
	{
		m_file = KFileDialog::getSaveFileName( "", "*.diff", 0, "FileSaveDialog" );
		if ( m_file.isEmpty() )
		{
			return false;
		}
	}

	// m_file is always local, so we use QFile
	QFile file(m_file);
	if (file.open(IO_WriteOnly) == false)
		return false;

	// use QTextStream to dump the text to the file
	QTextStream stream(&file);

	for ( QStringList::ConstIterator it = m_diffOutput.begin(); it != m_diffOutput.end(); ++it )
	{
		// Is this enough ?
		stream << (*it) << "\n";
	}

	file.close();

	return true;
}

void KDiffPart::loadSettings(KConfig *config)
{
	config->setGroup( "General" );
	m_synchronizeScrollBars->setChecked(config->readBoolEntry("Synchronize Scroll Bars",true));
//	m_diffView->setFont(config->readFontEntry("Font"));
//	m_diffView->setTabWidth(config->readUnsignedNumEntry("Tab Width",8));

	m_generalSettings->loadSettings( config );
	config->setGroup( "DiffSettings" );
	m_diffSettings->loadSettings( config );
	config->setGroup( "MiscSettings" );
	m_miscSettings->loadSettings( config );
}

void KDiffPart::saveSettings(KConfig *config)
{
	config->setGroup( "General" );
	config->writeEntry("Synchronize Scroll Bars", m_synchronizeScrollBars->isChecked());
//	config->writeEntry("Font", m_diffView->getFont());
//	config->writeEntry("Tab Width", m_diffView->getTabWidth());

	m_generalSettings->saveSettings( config );
	config->setGroup( "DiffSettings" );
	m_diffSettings->saveSettings( config );
	config->setGroup( "MiscSettings" );
	m_miscSettings->saveSettings( config );
}

void KDiffPart::slotItemsChanged() {
	updateActions();
}

void KDiffPart::slotSelectionChanged() {
	updateActions();
}

void KDiffPart::slotDifferenceMenuAboutToShow()
{
	m_differences->fillDifferenceMenu( m_diffView->getModel(),
	    m_diffView->getSelectedItem() );
}

void KDiffPart::slotGoDifferenceActivated( int item )
{
	m_diffView->setSelectedItem( item );
}

void KDiffPart::updateActions()
{
	int selected = m_diffView->getSelectedItem();
	int count = m_diffView->getModel()->getDifferences().count();
	m_previousDifference->setEnabled( selected > 0 );
	m_nextDifference->setEnabled( selected < count - 1 );
}

void KDiffPart::slotPreviousDifference()
{
	int newitem;
	int markeditem = m_diffView->getSelectedItem();
	if (markeditem == 0)
		return; // internal error (button not disabled)
	else
		newitem = markeditem-1;
	m_diffView->setSelectedItem(newitem);
}

void KDiffPart::slotNextDifference()
{
	int newitem;
	int markeditem = m_diffView->getSelectedItem();
	int count = m_diffView->getModel()->getDifferences().count();
	if (markeditem == count - 1)
		return; // internal error (button not disabled)
	else
		newitem = markeditem+1;
	m_diffView->setSelectedItem(newitem);
}

void KDiffPart::optionsPreferences()
{
	// show preferences
	KDiffPrefDlg* pref = new KDiffPrefDlg( m_generalSettings, m_diffSettings, m_miscSettings );
	
	if ( pref->exec() )
	{
		KConfig* config = instance()->config();
		saveSettings( config );
		config->sync();
	}
	kdDebug() << m_diffSettings->m_linesOfContext << endl;

};

void KDiffPart::toggleSynchronize()
{
//	m_diffView->setSynchronizeScrollBars( m_synchronizeScrollBars->isChecked() );
}

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
		s_about = new KAboutData("kdiffpart", I18N_NOOP("KDiffPart"), "0.1");
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
