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

#include "diffmodel.h"

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
* @author John Firebaugh <jfirebaugh@mac.com>
* @version 0.1
*/
class KDiffPart : public KParts::ReadWritePart
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
	* This is a virtual function inherited from KParts::ReadWritePart.
	* A shell will use this to inform this Part if it should act
	* read-only
	*/
	virtual void setReadWrite(bool rw);

	/**
	* Reimplemented to disable and enable Save action
	*/
	virtual void setModified(bool modified);

	/**
	 * Create the navigation widget. For example, this may be embedded in a dock
	 * widget by the shell.
	 */
	QWidget* createNavigationWidget( QWidget* parent = 0L, const char* name = 0L );

	void compare( const KURL& source, const KURL& destination, DiffSettings* settings = 0 );
	void setFormat( QCString format );

	void loadSettings(KConfig *config);
	void saveSettings(KConfig *config);

	const KURL& getSourceURL() const { return m_sourceURL; };
	const KURL& getDestinationURL() const { return m_destinationURL; };

	int modelCount() const
		{ return m_models.count(); };
	const DiffModel* modelAt( int i )
		{ return m_models.at( i ); };
	int getSelectedModelIndex() const
		{ return m_selectedModel; };
	int getSelectedDifferenceIndex() const
		{ return m_selectedDifference; };
	const DiffModel* getSelectedModel()
		{ return m_models.at( m_selectedModel ); };
	const Difference* getSelectedDifference()
		{ return m_models.at( m_selectedModel )->differenceAt( m_selectedDifference ); };

public slots:
	void slotSetSelection( int model, int diff );

signals:
	void selectionChanged( int model, int diff );

protected:
	/**
	* This must be implemented by each part
	*/
	virtual bool openFile();

	/**
	* This must be implemented by each read-write part
	*/
	virtual bool saveFile();

protected:
	void updateActions();

protected slots:
	void slotSelectionChanged( int model, int diff );
	void slotDifferenceMenuAboutToShow();
	void slotGoDifferenceActivated( int item );
	void slotDiffProcessFinished( bool success );
	void slotPreviousDifference();
	void slotNextDifference();
	void optionsPreferences();
	void slotShowDiffstats();

private:
	bool parseDiff( QStringList diff );
	void setupActions();
	void setupStatusbar();

	static GeneralSettings*    m_generalSettings;
	static DiffSettings*       m_diffSettings;
	static MiscSettings*       m_miscSettings;

	QList<DiffModel>       m_models;

	int                    m_selectedModel;
	int                    m_selectedDifference;

	DiffModel::DiffFormat  m_format;
	KDiffView*             m_diffView;
	KDiffNavigationTree*   m_navigationTree;
	KAction*               m_diffStats;
	KAction*               m_saveDiff;
	KAction*               m_previousDifference;
	KAction*               m_nextDifference;
	KDifferencesAction*    m_differences;
	KDiffProcess*          m_diffProcess;
	QStringList            m_diffOutput;
	KURL                   m_sourceURL;
	KURL                   m_destinationURL;
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
