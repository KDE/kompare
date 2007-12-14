/***************************************************************************
                                kompare_part.h  -  description
                                -------------------
        begin                   : Sun Mar 4 2001
        copyright               : (C) 2001-2004 Otto Bruggeman
                                  (C) 2001-2003 John Firebaugh
                                  (C) 2004      Jeff Snyder
                                  (C) 2007      Kevin Kofler
        email                   : otto.bruggeman@home.nl
                                  jfirebaugh@kde.org
                                  jeff@caffeinated.me.uk
                                  kevin.kofler@chello.at
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

#include "kompare.h"

#include "kompareinterface.h"

class QWidget;

class KTemporaryFile;
class KToggleAction;
class KUrl;
class KAboutData;
class KAction;

namespace Diff2 {
class Difference;
class DiffModel;
class DiffModelList;
class KompareModelList;
}
class DiffSettings;
class ViewSettings;
class KompareSplitter;
class KompareProcess;

/**
 * This is a "Part".  It does all the real work in a KPart
 * application.
 *
 * @short Main Part
 * @author John Firebaugh <jfirebaugh@kde.org>
 * @author Otto Bruggeman <bruggie@home.nl>
 * @version 0.3
 */
class KomparePart : public KParts::ReadWritePart,
                    public KompareInterface
{
	Q_OBJECT
public:
	/**
	* Default constructor
	*/
	KomparePart( QWidget *parentWidget, QObject *parent, const QStringList & /*args*/);

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
	// we need it now to save the properties of the part when apps dont (can't)
	// use the readProperties and saveProperties methods
	virtual bool queryClose();

	// bool isModified() const { return m_modelList->isModified(); }
	// Do we really want to expose this ???
	const Diff2::KompareModelList* model() const { return m_modelList; };

	static KAboutData *createAboutData();

public:
	// Reimplemented from the KompareInterface
	/**
	 * Open and parse the diff file at diffUrl.
	 */
	virtual bool openDiff( const KUrl& diffUrl );

	/** Added on request of Harald Fernengel */
	virtual bool openDiff( const QString& diffOutput );

	/** Open and parse the diff3 file at diff3Url */
	virtual bool openDiff3( const KUrl& diff3URL );

	/** Open and parse the file diff3Output with the output of diff3 */
	virtual bool openDiff3( const QString& diff3Output );

	/** Compare, with diff, source with destination */
	virtual void compare( const KUrl& sourceFile, const KUrl& destinationFile );

	/** Compare, with diff, source with destination */
	virtual void compareFiles( const KUrl& sourceFile, const KUrl& destinationFile );

	/** Compare, with diff, source with destination */
	virtual void compareDirs ( const KUrl& sourceDir, const KUrl& destinationDir );

	/** Compare, with diff3, originalFile with changedFile1 and changedFile2 */
	virtual void compare3Files( const KUrl& originalFile, const KUrl& changedFile1, const KUrl& changedFile2 );

	/** This will show the file and the file with the diff applied */
	virtual void openFileAndDiff( const KUrl& file, const KUrl& diffFile );

	/** This will show the directory and the directory with the diff applied */
	virtual void openDirAndDiff ( const KUrl& dir,  const KUrl& diffFile );

	/** Reimplementing this because this one knows more about the real part then the interface */
	virtual void setEncoding( const QString& encoding );

	// This is the interpart interface, it is signal and slot based so no "real" interface here
	// All you have to do is connect the parts from your application.
	// These just point to their counterpart in the KompareModelList or get called from their
	// counterpart in KompareModelList.
signals:
	void modelsChanged( const Diff2::DiffModelList* models );

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
	/** Save all destinations. */
	bool saveAll();

	/** Save the results of a comparison as a diff file. */
	void saveDiff();

	/** This slot is connected to the setModifed( bool ) signal from the KompareModelList */
	void slotSetModified( bool modified );

signals:
	void appliedChanged();
	void diffURLChanged();
	void kompareInfo( Kompare::Info* info );
	void setStatusBarModelInfo( int modelIndex, int differenceIndex, int modelCount, int differenceCount, int appliedCount );
//	void setStatusBarText( const QString& text );
	void diffString(const QString&);

protected:
	/**
	 * This is the method that gets called when the file is opened,
	 * when using openURL( const KUrl& ) or in our case also openDiff( const KUrl& );
	 * return true when everything went ok, false if there were problems
	 */
	virtual bool openFile();
	virtual bool saveFile() { return true; };

	// patchFile
	bool patchFile(KUrl&);
	bool patchDir();

protected slots:
	void slotSetStatus( Kompare::Status status );
	void slotShowError( QString error );

	void slotSwap();
	void slotShowDiffstats();
	void optionsPreferences();
	void updateActions();
	void updateCaption();
	void updateStatus();

private:
	void cleanUpTemporaryFiles();
	void setupActions();
	bool exists( const QString& url );
	bool isDirectory( const KUrl& url );
	const QString fetchURL( const KUrl& url );

private:
	// Uhm why were these static again ???
	// Ah yes, so multiple instances of kompare use the
	// same settings after one of them changes them
	static ViewSettings* m_viewSettings;
	static DiffSettings* m_diffSettings;

	Diff2::KompareModelList* m_modelList;

	KompareSplitter*         m_splitter;

	KAction*                 m_saveAll;
	KAction*                 m_saveDiff;
	KAction*                 m_swap;
	KAction*                 m_diffStats;

	KTemporaryFile*               m_tempDiff;

	struct Kompare::Info     m_info;
};

#endif // KOMPAREPART_H
