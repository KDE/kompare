/***************************************************************************
                          komparemodellist.h  -  description
                             -------------------
    begin                : Tue Jun 26 2001
    copyright            : (C) 2001-2003 by John Firebaugh
                           and Otto Bruggeman
    email                : jfirebaugh@kde.org
                           otto.bruggeman@home.nl
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KOMPAREMODELLIST_H
#define KOMPAREMODELLIST_H

#include <qptrlist.h>
#include <qobject.h>

#include <kurl.h>

#include "diffmodel.h"
#include "kompare.h"

class QFile;

class KDirWatch;
class KTempFile;

class DiffSettings;
class ViewSettings;
class KompareProcess;

namespace Diff2
{

class KompareModelList : public QObject
{
	Q_OBJECT
public:
	KompareModelList( DiffSettings* diffSettings, ViewSettings* viewSettings, QObject* parent = 0, const char* name = 0 );
	~KompareModelList();

public:
	/* Comparing methods */
	bool compare( const QString& source, const QString& destination );

	// Swap source with destination and show differences
	void swap();

	bool compareFiles( const QString& source, const QString& destination );
	bool compareDirs( const QString& source, const QString& destination );

public:
	bool openDiff( const QString& url );

public:
	bool openFileAndDiff( const QString& file, const QString& diff );
	bool openDirAndDiff( const QString& dir, const QString& diff );

public:
	bool saveDiff( const QString& url, QString directory, DiffSettings* diffSettings, ViewSettings* viewSettings );
	bool saveAll();

	bool saveDestination( const DiffModel* model );

	// This parses the difflines and creates new models
	int parseDiffOutput( const QStringList& lines );

	// Call this to emit the signals to the rest of the "world" to show the diff
	void show();

	// This will blend the original URL (dir or file) into the diffmodel,
	// this is like patching but with a twist
	bool blendOriginalIntoModelList( const QString& localURL );

	enum Kompare::Mode          mode()   const { return m_mode; };
	enum Kompare::Format        format() const { return m_format; };
	const QPtrList<DiffModel>*  models() const { return m_models; };

	int                         modelCount() const;
	int                         differenceCount() const;
	int                         appliedCount() const;

	DiffModel*                  modelAt( int i ) const { return const_cast<KompareModelList*>(this)->m_models->at( i ); };
	int                         findModel( DiffModel* model ) const { return const_cast<KompareModelList*>(this)->m_models->find( model ); };

	bool                        isModified() const;

	int currentModel() const      { return const_cast<KompareModelList*>(this)->m_models->find( m_selectedModel ); };
	int currentDifference() const { return m_selectedModel ? m_selectedModel->findDifference( m_selectedDifference ) : -1; };

	const DiffModel* selectedModel() const       { return m_selectedModel; };
	const Difference* selectedDifference() const { return m_selectedDifference; };

	// clear the models (needs to be public if the part is reused)
	void clear();

protected:
	bool blendFile( DiffModel* model, const QStringList& lines );

signals:
	void status( Kompare::Status status );
	void error( QString error );
	void modelsChanged( const QPtrList<Diff2::DiffModel>* models );
	void setSelection( const Diff2::DiffModel* model, const Diff2::Difference* diff );
	void setSelection( const Diff2::Difference* diff );
	void applyDifference( bool apply );
	void applyAllDifferences( bool apply );
	void applyDifference( const Diff2::Difference* diff, bool apply );

	// Emits true when m_noOfModified > 0, false when m_noOfModified == 0
	void setModified( bool modified );

public slots:
	void slotSelectionChanged( const Diff2::DiffModel* model, const Diff2::Difference* diff );
	void slotSelectionChanged( const Diff2::Difference* diff );

	void slotApplyDifference( bool apply );
	void slotApplyAllDifferences( bool apply );
	void slotPreviousModel();
	void slotNextModel();
	void slotPreviousDifference();
	void slotNextDifference();

	// This slot is called by the diffmodels whenever their status changes to modified or unmodified
	void slotSetModified( bool modified );

protected slots:
	void slotDiffProcessFinished( bool success );
	void slotWriteDiffOutput( bool success );

private slots:
	void slotDirectoryChanged( const QString& );
	void slotFileChanged( const QString& );

private: // Helper methods
	bool isDirectory( const QString& url );
	bool isDiff( const QString& mimetype );
	QStringList& readFile( const QString& fileName );

private:
	KompareProcess*      m_diffProcess;

	DiffSettings*        m_diffSettings;
	ViewSettings*        m_viewSettings;

	QPtrList<DiffModel>* m_models;

	QString              m_source;
	QString              m_destination;

	enum Kompare::Format m_format;
	enum Kompare::Mode   m_mode; // reading from a DiffFile or Comparing in Kompare
	enum Kompare::Type   m_type; // single or multi

	DiffModel*           m_selectedModel;
	Difference*          m_selectedDifference;

	QPtrListIterator<DiffModel>*  m_modelIt;
	QPtrListIterator<Difference>* m_diffIt;

	KDirWatch*           m_dirWatch;
	KDirWatch*           m_fileWatch;

	int                  m_noOfModified;
};

} // End of namespace Diff2

#endif
