/***************************************************************************
                                kdiff_part.h  -  description
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

#ifndef KDIFFPART_H
#define KDIFFPART_H

#include <kparts/part.h>
#include <kparts/factory.h>

#include "kdiff.h"
#include "diffmodel.h"

#include <kdiffmodellist.h>

class QWidget;

class KToggleAction;
class KURL;

class KDiffView;
class KDiffNavigationTree;
class KDifferencesAction;
class KDiffProcess;
class KDiffStatsDlg;
class GeneralSettings;
class DiffSettings;
class MiscSettings;

/**
* This is a "Part".  It that does all the real work in a KPart
* application.
*
* @short Main Part
* @author John Firebaugh <jfirebaugh@kde.org>
* @version 0.1
*/
class KDiffPart : public KParts::ReadWritePart, KDiff
{
	Q_OBJECT
public:
	/**
	* Default constructor
	*/
	KDiffPart( QWidget *parentWidget, const char *widgetName,
	           QObject *parent, const char *name);

	/**
	* Destructor
	*/
	virtual ~KDiffPart();

	/**
	 * Create the navigation widget. For example, this may be embedded in a dock
	 * widget by the shell.
	 */
	QWidget* createNavigationWidget( QWidget* parent = 0L, const char* name = 0L );

	bool askSaveChanges();
	
	void loadSettings(KConfig *config);
	void saveSettings(KConfig *config);

	int modelCount() const
		{ return m_models->modelCount(); };
	int getSelectedModelIndex() const
		{ return m_selectedModel; };
	int getSelectedDifferenceIndex() const
		{ return m_selectedDifference; };
	DiffModel* getSelectedModel()
		{ return m_models->modelAt( m_selectedModel ); };
	Difference* getSelectedDifference()
		{ return m_models->modelAt( m_selectedModel )->differenceAt( m_selectedDifference ); };
	int appliedCount() const { return m_models->modelAt( m_selectedModel )->appliedCount(); };
	
	bool isModified() const { return m_models->isModified(); };
	
	/** Returns the url to the open diff file, or a url to a temporary
	    diff file if we are comparing files. */
	KURL diffURL();

public slots:

  /** Overridden from KPart::ReadOnlyPart. Just calls openDiff( url ). */
	bool openURL( const KURL &url );

	/** Compare source with destination */
	void compare( const KURL& source, const KURL& destination );

	/** Save the currently selected destination in a multi-file diff,
	    or the single destination if a single file diff. */
	bool saveDestination();

	/** Save all destinations. */
	bool saveAll();

	/** Open and parse the diff file at url. */
	bool openDiff( const KURL& url );

	/** Save the results of a comparison as a diff file. */
	void saveDiff();

	void slotSetSelection( int model, int diff );

signals:
	void selectionChanged( int model, int diff );
	void appliedChanged();
	void diffURLChanged();

protected:

	/* We handle the urls ourself */
	bool openFile() { return true; };
	bool saveFile() { return true; };

protected slots:
	void slotSetStatus( KDiffModelList::Status status );
	void slotShowError( QString error );
	void slotModelsChanged();
	
	void slotSelectionChanged( int model, int diff );
	void slotAppliedChanged( const Difference* d );
	void slotDifferenceMenuAboutToShow();
	void slotGoDifferenceActivated( int item );
	
	void slotSwap();
	void slotShowDiffstats();
	void slotApplyDifference();
	void slotApplyAllDifferences();
	void slotUnapplyAllDifferences();
	void slotPreviousFile();
	void slotNextFile();
	void slotPreviousDifference();
	void slotNextDifference();
	void optionsPreferences();

private:
	void setupActions();
	void setupStatusbar();
	void updateActions();
	void updateStatus();

	static GeneralSettings*    m_generalSettings;
	static DiffSettings*       m_diffSettings;
	static MiscSettings*       m_miscSettings;

	KDiffModelList*        m_models;
	int                    m_selectedModel;
	int                    m_selectedDifference;

	KDiffView*             m_diffView;
	KDiffNavigationTree*   m_navigationTree;
	KAction*               m_save;
	KAction*               m_saveAll;
	KAction*               m_saveDiff;
	KAction*               m_swap;
	KAction*               m_diffStats;
	KAction*               m_applyDifference;
	KAction*               m_applyAll;
	KAction*               m_unapplyAll;
	KAction*               m_previousFile;
	KAction*               m_nextFile;
	KAction*               m_previousDifference;
	KAction*               m_nextDifference;
	KDifferencesAction*    m_differences;
	
	KTempFile*             m_tempDiff;
};

class KInstance;
class KAboutData;

class KDiffPartFactory : public KParts::Factory
{
	Q_OBJECT
public:
	KDiffPartFactory();
	virtual ~KDiffPartFactory();
	virtual KParts::Part* createPartObject( QWidget *parentWidget, const char *widgetName,
											QObject *parent, const char *name,
											const char *classname, const QStringList &args );
	static KInstance* instance();

private:
	static KInstance* s_instance;
	static KAboutData* s_about;
};

#endif // KDIFFPART_H
