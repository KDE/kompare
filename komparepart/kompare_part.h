/***************************************************************************
                                kompare_part.h  -  description
                                -------------------
        begin                   : Sun Mar 4 2001
        copyright               : (C) 2001-2003 by Otto Bruggeman
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

#ifndef KOMPAREPART_H
#define KOMPAREPART_H

#include <kparts/factory.h>
#include <kparts/part.h>

#include "diffmodel.h"
#include "kompare.h"
#include "komparemodellist.h"

#include "kompareinterface.h"

class QWidget;

class KToggleAction;
class KURL;

class Diff2::Difference;
class DiffSettings;
class ViewSettings;
class KFileTreeView;
class KompareView;
class KompareNavigationTree;
class DifferencesAction;
class Diff2::KompareModelList;
class KompareProcess;
class KompareStatsDlg;
class KompareActions;
class MiscSettings;

/**
 * This is a "Part".  It does all the real work in a KPart
 * application.
 *
 * @short Main Part
 * @author John Firebaugh <jfirebaugh@kde.org>
 * @author Otto Bruggeman <bruggie@home.nl>
 * @version 0.2
 */
class KomparePart : public KParts::ReadWritePart,
                    public KompareInterface
{
	Q_OBJECT
public:
	/**
	* Default constructor
	*/
	KomparePart( QWidget *parentWidget, const char *widgetName,
	             QObject *parent, const char *name);

	/**
	* Destructor
	*/
	virtual ~KomparePart();

	// Sessionmanagement stuff, added to the kompare iface
	// because they are not in the Part class where they belong
	// Should be added when bic changes are allowed again (kde 4.0)
	virtual int readProperties( KConfig *config );
	virtual int saveProperties( KConfig *config );
	// this one is called when the shell_app is about to close.
	// we need it now to save the properties of the part  when apps dont (can't)
	// use the readProperties and saveProperties methods
	virtual bool queryClose();

	// bool isModified() const { return m_modelList->isModified(); };
	// Do we really want to expose this ???
	const Diff2::KompareModelList* model() const { return m_modelList; };
	/** Returns the url to the open diff file, or a url to a temporary
	    diff file if we are comparing files. */
	KURL diffURL(); // why ? url() from the ReadOnlyPart does the same...

public:
	// Reimplemented from the KompareInterface
	/**
	 * Open and parse the diff file at diffUrl.
	 */
	virtual bool openDiff( const KURL& diffUrl );

	/** Added on request of Harald Fernengel */
	virtual bool openDiff( const QString& diffOutput );

	/** Open and parse the diff output */
	virtual bool openDiff( const QStringList& diffOutput );

	/** Open and parse the diff3 file at diff3Url */
	virtual bool openDiff3( const KURL& diff3URL );

	/** Open and parse the file diff3Output with the output of diff3 */
	virtual bool openDiff3( const QString& diff3Output );

	/** Compare, with diff, source with destination */
	virtual void compare( const KURL& sourceFile, const KURL& destinationFile );

	/** Compare, with diff, source with destination */
	virtual void compareFiles( const KURL& sourceFile, const KURL& destinationFile );

	/** Compare, with diff, source with destination */
	virtual void compareDirs ( const KURL& sourceDir, const KURL& destinationDir );

	/** Compare, with diff3, originalFile with changedFile1 and changedFile2 */
	virtual void compare3Files( const KURL& originalFile, const KURL& changedFile1, const KURL& changedFile2 );

	/** This will show the file and the file with the diff applied */
	virtual void openFileAndDiff( const KURL& file, const KURL& diffFile );

	/** This will show the directory and the directory with the diff applied */
	virtual void openDirAndDiff ( const KURL& dir,  const KURL& diffFile );

	// This is the interpart interface, it is signal and slot based so no "real" nterface here
	// All you have to do is connect the parts from your application.
	// These just point to their namesake in the KompareModelList or get called from their
	// namesake in KompareModelList.
signals:
	void modelsChanged( const QPtrList<Diff2::DiffModel>* models );

	void setSelection( const Diff2::DiffModel* model, const Diff2::Difference* diff );
	void setSelection( const Diff2::Difference* diff );

	void selectionChanged( const Diff2::DiffModel* model, const Diff2::Difference* diff );
	void selectionChanged( const Diff2::Difference* diff );

	void applyDifference( bool apply );
	void applyAllDifferences( bool apply );
	void applyDifference( const Diff2::Difference*, bool apply );

	void configChanged();

	/*
	** This is emitted when a difference is clicked in the kompare view. You can connect to
	** it so you can use it to jump to this particular line in the editor in your app.
	*/
	void differenceClicked( int lineNumber );

	// Stuff that can probably be removed by putting it in the part where it belongs in my opinion
public slots:
	/** Save the currently selected destination in a multi-file diff,
	    or the single destination if a single file diff. */
	bool saveDestination();

	/** Save all destinations. */
	bool saveAll();

	/** Save the results of a comparison as a diff file. */
	void saveDiff();

	/** This slot is connected to the setModifed( bool ) signal from the KompareModelList */
	void slotSetModified( bool modified );

signals:
	void appliedChanged();
	void diffURLChanged();
	void setStatusBar( int modelIndex, int differenceIndex, int modelCount, int differenceCount, int appliedCount );
//	void setStatusBarText( const QString& text );

protected:
	/**
	 * This is the method that gets called when the file is opened,
	 * when using openURL( const KURL& ) or in our case also openDiff( const KURL& );
	 * return true when everything went ok, false if there were problems
	 */
	virtual bool openFile();
	virtual bool saveFile() { return true; };
	// This will read the m_file file and return the lines
	QStringList& readFile();

	// patchFile
	bool patchFile(KURL&);
	bool patchDir();

protected slots:
	void slotSetStatus( Kompare::Status status );
	void slotShowError( QString error );

	void slotSwap();
	void slotShowDiffstats();
	void optionsPreferences();
	void updateActions();
	void updateStatus();

private:
	void setupActions();
	bool exists( const QString& url );
	bool isDirectory( const KURL& url );
	const QString& fetchURL( const KURL& url );

private:
	// Uhm why were these static again ???
	// Ah yes, so multiple instances of kompare use the
	// same settings after one of them changes them
	static ViewSettings* m_viewSettings;
	static DiffSettings* m_diffSettings;
	static MiscSettings* m_miscSettings;

	Diff2::KompareModelList* m_modelList;

	KompareView*             m_diffView;

//	KompareNavigationTree*   m_navigationTree;

	KAction*                 m_save;
	KAction*                 m_saveAll;
	KAction*                 m_saveDiff;
	KAction*                 m_swap;
	KAction*                 m_diffStats;
	KompareActions*          m_kompareActions;

	KTempFile*               m_tempDiff;

	KURL                     m_sourceURL;
	KURL                     m_destinationURL;
	QString                  m_localSource;
	QString                  m_localDestination;
};

class KInstance;
class KAboutData;

class KomparePartFactory : public KParts::Factory
{
	Q_OBJECT
public:
	KomparePartFactory();
	virtual ~KomparePartFactory();
	virtual KParts::Part* createPartObject( QWidget *parentWidget, const char *widgetName,
	                                        QObject *parent, const char *name,
	                                        const char *classname, const QStringList &args );
	static KInstance* instance();

private:
	static KInstance* s_instance;
	static KAboutData* s_about;
};

#endif // KOMPAREPART_H
