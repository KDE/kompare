/***************************************************************************
                                kompare_shell.h
                                ----------------
        begin                   : Sun Mar 4 2001
        Copyright 2001-2003 Otto Bruggeman <otto.bruggeman@home.nl>
        Copyright 2001-2003 John Firebaugh <jfirebaugh@kde.org>
        Copyright 2007      Kevin Kofler   <kevin.kofler@chello.at>
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

#include <kapplication.h>
#include <kparts/mainwindow.h>

#include "kompare.h"

class KompareInterface;
namespace KParts {
    class ReadOnlyPart;
    class ReadWritePart;
}
class KToggleAction;

class KSqueezedTextLabel;
class KomparePart;
class KompareNavTreePart;

namespace KTextEditor {
	class Document;
	class EditInterface;
	class View;
}

/**
 * This is the application "Shell".  It has a menubar, toolbar, and
 * statusbar but relies on the "Part" to do all the real work.
 *
 * Adapted the shell a bit so it now handles separate view and navigation parts
 *
 * @short Application Shell
 * @author John Firebaugh <jfirebaugh@kde.org>
 * @author Otto Bruggeman <bruggie@home.nl>
 * @version 3.2.90
 */
class KompareShell : public KParts::MainWindow
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
	void openDiff( const KUrl& url );

	/**
	 * Use this method to load the diff from stdin
	 */
	void openStdin();

	/**
	 * Use this method to compare 2 URLs (files or directories)
	 */
	void compare( const KUrl& source, const KUrl& destination );

	/**
	 * Use this method to blend diff into url1 (file or directory)
	 */
	void blend( const KUrl& url1, const KUrl& diff );

public slots:
	void slotUpdateStatusBar( int modelIndex, int differenceIndex, int modelCount, int differenceCount, int appliedCount );
	void setCaption( const QString& caption );

	KompareInterface* viewPart() const;

protected:
	virtual bool queryClose();

	/**
	 * This method is called when it is time for the app to save its
	 * properties for session management purposes.
	 */
	void saveProperties(KConfigGroup &);

	/**
	 * This method is called when this app is restored.  The KConfig
	 * object points to the session management config file that was saved
	 * with @ref saveProperties
	 */
	void readProperties(const KConfigGroup &);

private slots:
	void slotSetStatusBarText( const QString& text );
	void slotFileOpen();
	void slotFileCompareFiles();
	void slotFileBlendURLAndDiff();
	void slotShowTextView();
	void slotFileClose();
	void optionsConfigureKeys();
	void optionsConfigureToolbars();
	void slotSetDiffString( const QString& diffString );
	void newToolbarConfig();
	void slotVisibilityChanged( bool visible );

private:
	void setupAccel();
	void setupActions();
	void setupStatusBar();

private:
	KUrl                        m_sourceURL;
	KUrl                        m_destinationURL;
	KUrl                        m_diffURL;

	KParts::ReadWritePart*      m_viewPart;
	KParts::ReadOnlyPart*         m_navTreePart;
	KTextEditor::Document*      m_textViewPart;
	KTextEditor::View*          m_textView;
// 	KTextEditor::EditInterface* m_textEditIface;

	QDockWidget*                m_textViewWidget;
	QDockWidget*                m_navTreeDock;

	KToggleAction*              m_showTextView;

	enum Kompare::Mode          m_mode;
	// This is the statusbarwidget for displaying the general stuff
	KSqueezedTextLabel*         m_generalLabel;

	QString                     m_diffString;
};

#endif // KOMPARE_H
