/***************************************************************************
                                kompare_shell.h  -  description
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

#ifndef KOMPARESHELL_H
#define KOMPARESHELL_H

/* there is no config.h used yet, so disabling
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
*/

#include <kapplication.h>
#include <kparts/dockmainwindow.h>

#include "kompare.h"

class KToggleAction;
class KSqueezedTextLabel;
 
class KomparePart;
class KompareNavTreePart;

/**
* This is the application "Shell".  It has a menubar, toolbar, and
* statusbar but relies on the "Part" to do all the real work.
*
* Adapted the shell a bit so it now handles seperate view and navigation parts
*
* @short Application Shell
* @author John Firebaugh <jfirebaugh@kde.org>
* @author Otto Bruggeman <bruggie@bruggie.dnsalias.org>
* @version 0.2
*/
class KompareShell : public KParts::DockMainWindow, Kompare
{
	Q_OBJECT
public:
	/**
	* Default Constructor
	*/
	KompareShell();

	/**
	* Default Destructor
	*/
	virtual ~KompareShell();

	/**
	* Use this method to load whatever file/URL you have
	*/
	void load(const KURL& url);

	/**
	 *
	 */
	void compare(const KURL& source,const KURL& destination );

public slots:
	void setCaption( const QString& caption );

protected:
	virtual bool queryClose();
	
	/**
	* This method is called when it is time for the app to save its
	* properties for session management purposes.
	*/
	void saveProperties(KConfig *);

	/**
	* This method is called when this app is restored.  The KConfig
	* object points to the session management config file that was saved
	* with @ref saveProperties
	*/
	void readProperties(KConfig *);

private slots:
	void slotSetStatusBarText( const QString& text );
	void slotFileCompareFiles();
	void slotFileOpen();
	void optionsShowToolbar();
	void optionsShowStatusbar();
	void slotShowTextView();
	void optionsConfigureKeys();
	void optionsConfigureToolbars();
	void updateStatusBar();
	void slotDiffURLChanged();

private:
	void setupAccel();
	void setupActions();
	void setupStatusBar();

private:
	KURL                  m_sourceURL;
	KURL                  m_destinationURL;
	KURL                  m_diffURL;
	
	KomparePart*          m_viewPart;
	KompareNavTreePart*   m_navTreePart;
	KParts::ReadOnlyPart* m_textViewPart;

	KDockWidget*          m_textViewWidget;
	KDockWidget*          m_mainViewDock;
	KDockWidget*          m_navTreeDock;

	KToggleAction*        m_toolbarAction;
	KToggleAction*        m_statusbarAction;
	KToggleAction*        m_showTextView;
	
	enum Kompare::Mode    m_mode;
	// This is the statusbarwidget for displaying the general stuff
	KSqueezedTextLabel*   m_generalLabel;
};

#endif // KOMPARE_H
