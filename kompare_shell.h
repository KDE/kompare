#ifndef KDIFFSHELL_H
#define KDIFFSHELL_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif 

#include <kapp.h>
#include <kparts/mainwindow.h>

class KToggleAction;

class KDiffPart;

/**
* This is the application "Shell".  It has a menubar, toolbar, and
* statusbar but relies on the "Part" to do all the real work.
*
* @short Application Shell
* @author John Firebaugh <jfirebaugh@mac.com>
* @version 0.1
*/
class KDiffShell : public KParts::MainWindow
{
	Q_OBJECT
public:
	/**
	* Default Constructor
	*/
	KDiffShell();

	/**
	* Default Destructor
	*/
	virtual ~KDiffShell();

	/**
	* Use this method to load whatever file/URL you have
	*/
	void load(const KURL& url);

	/**
	 *
	 */
	void compare(const KURL& source,const KURL& destination );

protected:
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
	void slotFileCompareFiles();
	void slotFileOpen();
	void optionsShowToolbar();
	void optionsShowStatusbar();
	void optionsConfigureKeys();
	void optionsConfigureToolbars();
	void updateStatusBar();

private:
	void setupAccel();
	void setupActions();
	void setupStatusBar();

private:
	KURL             m_source;
	KURL             m_destination;
	KDiffPart*       m_part;

	KToggleAction*   m_toolbarAction;
	KToggleAction*   m_statusbarAction;
};

#endif // KDIFF_H
