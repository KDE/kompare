#ifndef KDIFFPART_H
#define KDIFFPART_H

#include <kparts/part.h>
#include <kparts/factory.h>

#include "generalsettings.h"
#include "diffsettings.h"
#include "miscsettings.h"

class QWidget;

class KToggleAction;
class KURL;

class KDiffView;
class KDifferencesAction;
class KDiffProcess;

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

	void compare( const KURL& source, const KURL& destination, DiffSettings* settings = 0 );

	KDiffView* diffView();
	void loadSettings(KConfig *config);
	void saveSettings(KConfig *config);

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
	void setBusy(bool busy);
	void updateActions();

protected slots:
	void slotDifferenceMenuAboutToShow();
	void slotGoDifferenceActivated( int item );
	void slotDiffProcessFinished( bool success );
	void toggleSynchronize();
	void slotItemsChanged();
	void slotSelectionChanged();
	void slotPreviousDifference();
	void slotNextDifference();
	void optionsPreferences();

private:
	void setupActions();
	void setupStatusbar();

	GeneralSettings*    m_generalSettings;
	DiffSettings*       m_diffSettings;
	MiscSettings*       m_miscSettings;

	KDiffView*          m_diffView;
	KAction*            m_previousDifference;
	KAction*            m_nextDifference;
	KDifferencesAction* m_differences;
	KToggleAction*      m_synchronizeScrollBars;
	KDiffProcess*       m_diffProcess;
	KURL*               m_leftURL;
	KURL*               m_rightURL;
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
