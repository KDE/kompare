/***************************************************************************
                          komparemodellist.h  -  description
                             -------------------
    begin                : Tue Jun 26 2001
    copyright            : (C) 2001 by John Firebaugh
    email                : jfirebaugh@kde.org
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

class KTempFile;

class DiffSettings;
class KompareProcess;

class KompareModelList : public QObject, Kompare
{
	Q_OBJECT

public:
	KompareModelList();
	~KompareModelList();

	bool compare( const KURL& source, const KURL& destination );
	bool saveDestination( const DiffModel* model );

	bool openDiff( const KURL& url );
	bool saveDiff( const KURL& url, QString directory, DiffSettings* settings );
	bool saveAll();

	void swap();

	enum Kompare::Mode          mode() const
	    { return m_mode; };
	enum Kompare::Format        format() const
	    { return m_format; };
	const QPtrList<DiffModel>&  models() const
	    { return m_models; };
	int                         modelCount() const
	    { return m_models.count(); };
	DiffModel*                  modelAt( int i ) const
	    { return const_cast<KompareModelList*>(this)->m_models.at( i ); };
	int                         findModel( DiffModel* model ) const
	    { return const_cast<KompareModelList*>(this)->m_models.find( model ); };

	QString                     sourceTemp() const      { return m_sourceTemp; };
	QString                     destinationTemp() const { return m_destinationTemp; };

	KURL                        diffURL() const         { return m_diffURL; };
	KURL                        sourceBaseURL() const;
	KURL                        destinationBaseURL() const;

	bool                        isModified() const;

	int selectedModelIndex()      { return m_selectedModel ? m_selectedModel->index() : -1; };
	int selectedDifferenceIndex() { return m_selectedModel ? m_selectedModel->findDifference( m_selectedDifference ) : -1; };

	const DiffModel* selectedModel() const       { return m_selectedModel; };
	const Difference* selectedDifference() const { return m_selectedDifference; };
signals:
	void status( Kompare::Status status );
	void error( QString error );
	void modelsChanged( const QPtrList<DiffModel>* models );
	void setSelection( const DiffModel* model, const Difference* diff );
	void setSelection( const Difference* diff );
	void applyDifference( bool apply );
	void applyAllDifferences( bool apply );

public slots:
	void slotSelectionChanged( const DiffModel* model, const Difference* diff );
	void slotSelectionChanged( const Difference* diff );

	void slotApplyDifference( bool apply );
	void slotApplyAllDifferences( bool apply );
	void slotPreviousModel();
	void slotNextModel();
	void slotPreviousDifference();
	void slotNextDifference();


protected slots:
	void slotDiffProcessFinished( bool success );
	void slotWriteDiffOutput( bool success );

private:
	int parseDiffOutput    ( const QStringList& lines );
	int determineDiffFormat( const QStringList& lines );
	int determineType      ( const QStringList& lines, enum Kompare::Format format );
	QPtrList<DiffModel> splitFiles( const QStringList& lines, bool split );
	void clear();

private:
	KompareProcess*      m_diffProcess;
	QPtrList<DiffModel>  m_models;

	KURL                 m_sourceURL;
	KURL                 m_destinationURL;
	QString              m_sourceTemp;
	QString              m_destinationTemp;

	KURL                 m_diffURL;
	KTempFile*           m_diffTemp;

	enum Kompare::Format m_format;
	enum Kompare::Mode   m_mode; // reading from a DiffFile or Comparing in Kompare
	enum Kompare::Type   m_type; // single, multi and diff or cvsdiff

	DiffModel*     m_selectedModel;
	Difference*    m_selectedDifference;
	
	QPtrListIterator<DiffModel>*  m_modelIt;
	QPtrListIterator<Difference>* m_diffIt;
};

#endif
