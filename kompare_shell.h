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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif 

#include <kapp.h>
#include <kparts/dockmainwindow.h>

class KToggleAction;

class KomparePart;

/**
* This is the application "Shell".  It has a menubar, toolbar, and
* statusbar but relies on the "Part" to do all the real work.
*
* @short Application Shell
* @author John Firebaugh <jfirebaugh@kde.org>
* @version 0.1
*/
class KompareShell : public KParts::DockMainWindow
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
	KURL             m_source;
	KURL             m_destination;
	KomparePart*     m_part;
	KDockWidget*     m_textViewWidget;
	KParts::ReadOnlyPart* m_textViewPart;
	
	KToggleAction*   m_toolbarAction;
	KToggleAction*   m_statusbarAction;
	KToggleAction*   m_showTextView;
};

#endif // KOMPARE_H
